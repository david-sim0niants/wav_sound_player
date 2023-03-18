#include <wsp/wav_play.h>
#include <alsa/asoundlib.h>


struct wsp_device_handle {
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *pcm_params;
	struct wsp_play_pcm_params params;
};


#define wsp_to_pcm_device_handle(handle) (handle->pcm_handle)


int wsp_play_init_pcm_device(const char *name, wsp_device_handle_t **p_handle)
{
	int ret;
	snd_pcm_t *pcm_handle;

	ret = snd_pcm_open(&pcm_handle, name, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("Failed opening %s pcm device. %s",
		       name, snd_strerror(ret));
		return ret;
	}

	*p_handle = malloc(sizeof(struct wsp_device_handle));
	(*p_handle)->pcm_handle = pcm_handle;
	(*p_handle)->pcm_params = NULL;
	memset(&(*p_handle)->params, 0, sizeof((*p_handle)->params));

	return 0;
}

void wsp_play_term_pcm_device(wsp_device_handle_t *handle)
{
	snd_pcm_close(wsp_to_pcm_device_handle(handle));
}


static snd_pcm_format_t bpS_to_pcm_format(unsigned int bits_per_sample)
{
	switch (bits_per_sample) {
		case 8:
			return SND_PCM_FORMAT_S8;
		case 16:
			return SND_PCM_FORMAT_S16_LE;
		case 24:
			return SND_PCM_FORMAT_S24_LE;
		case 20:
			return SND_PCM_FORMAT_S20_LE;
		case 32:
			return SND_PCM_FORMAT_S32_LE;
		default:
			return SND_PCM_FORMAT_UNKNOWN;
	}
}

int __wsp_play_get_pcm_params(wsp_device_handle_t *handle)
{
	int ret;
	snd_pcm_t *pcm_handle = wsp_to_pcm_device_handle(handle);
	snd_pcm_hw_params_t *pcm_params = handle->pcm_params;

	unsigned int tmp_uint;

	/* Get number of channels. */
	ret = snd_pcm_hw_params_get_channels(pcm_params, &tmp_uint);
	if (ret < 0) return ret;
	handle->params.nr_channels = (short)tmp_uint;

	/* Get sample rate. */
	ret = snd_pcm_hw_params_get_rate(pcm_params, &tmp_uint, NULL);
	if (ret < 0) return ret;
	handle->params.sample_rate = (unsigned int)tmp_uint;

	snd_pcm_uframes_t tmp_uframes;

	/* Get period size. */
	ret = snd_pcm_hw_params_get_period_size(pcm_params, &tmp_uframes, NULL);
	if (ret < 0) return ret;
	handle->params.period_size = (unsigned int)tmp_uframes;

	return 0;
}

int wsp_play_start_pcm(wsp_device_handle_t *handle,
			struct wsp_play_pcm_params *params)
{
	int ret;
	snd_pcm_t *pcm_handle = wsp_to_pcm_device_handle(handle);
	snd_pcm_hw_params_t *pcm_params;

	/* Allocating PCM params. */
	snd_pcm_hw_params_malloc(&pcm_params);
	/* Initializing PCM params. */
	snd_pcm_hw_params_any(pcm_handle, pcm_params);

	/* Channels are interleaved. */
	ret = snd_pcm_hw_params_set_access(pcm_handle, pcm_params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) return ret;

	/* Setting bits per sample as a snd_pcm_format_t type. */
	ret = snd_pcm_hw_params_set_format(
		pcm_handle, pcm_params,
		bpS_to_pcm_format(params->bits_per_sample));
	if (ret < 0) return ret;

	/* Setting number of channels. */
	ret = snd_pcm_hw_params_set_channels(
		pcm_handle, pcm_params, params->nr_channels);
	if (ret < 0) return ret;

	/* Setting sample rate. */
	ret = snd_pcm_hw_params_set_rate_near(
		pcm_handle, pcm_params, &params->sample_rate, 0);
	if (ret < 0) return ret;

	/* Setting period size to %nr_frames frames. */
	snd_pcm_uframes_t nr_frames = 0x100;
	ret = snd_pcm_hw_params_set_period_size_near(
		pcm_handle, pcm_params, &nr_frames, 0);

	/* Setting the pcm params. */
	ret = snd_pcm_hw_params(pcm_handle, pcm_params);
	if (ret < 0) return ret;

	handle->pcm_handle = pcm_handle;
	handle->pcm_params = pcm_params;
	handle->params.bits_per_sample = params->bits_per_sample;
	return __wsp_play_get_pcm_params(handle);
}

int wsp_play_stop_pcm(wsp_device_handle_t *handle)
{
	int ret;
	ret = snd_pcm_drain(wsp_to_pcm_device_handle(handle));
	snd_pcm_hw_params_free(handle->pcm_params);
	return ret;
}

long wsp_play_run_pcm(wsp_device_handle_t *handle,
			const void *pcm_buffer_data, unsigned int pcm_size)
{
	int ret;

	if (handle->params.period_size < pcm_size)
		return -EINVAL;

	snd_pcm_t *pcm_handle = wsp_to_pcm_device_handle(handle);

	snd_pcm_sframes_t nr_bytes;
	nr_bytes = snd_pcm_writei(pcm_handle, pcm_buffer_data, pcm_size);

	if (nr_bytes == -EPIPE) {
		/* Underrun or overrun occurred. */
		snd_pcm_prepare(pcm_handle);
		return 0;
	}

	return nr_bytes;
}

void wsp_play_get_pcm_params(wsp_device_handle_t *handle,
				struct wsp_play_pcm_params *params)
{
	memcpy(params, &handle->params, sizeof(*params));
}
