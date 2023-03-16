#include <wsp/wav_play.h>
#include <alsa/asoundlib.h>


#define pcm_to_wsp_device_handle(handle) ((wsp_device_handle_t *)handle)
#define wsp_to_pcm_device_handle(handle) ((snd_pcm_t *)handle)


int wsp_play_init_pcm_device(const char *name, wsp_device_handle_t **handle)
{
	int ret;
	snd_pcm_t *pcm_handle;

	ret = snd_pcm_open(&pcm_handle, name, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("Failed opening %s pcm device. %s",
		       name, snd_strerror(ret));
		return ret;
	}

	*handle = pcm_to_wsp_device_handle(pcm_handle);
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


int wsp_play_start_pcm(wsp_device_handle_t *handle,
			struct wsp_play_pcm_params *params)
{
	int ret;
	snd_pcm_t *pcm_handle = wsp_to_pcm_device_handle(handle);
	snd_pcm_hw_params_t *pcm_params;

	/* Allocating PCM params. */
	snd_pcm_hw_params_alloca(&pcm_params);
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

	/* Setting the pcm params. */
	ret = snd_pcm_hw_params(pcm_handle, pcm_params);

	return ret;
}

int wsp_play_stop_pcm(wsp_device_handle_t *handle)
{
	return snd_pcm_drain(wsp_to_pcm_device_handle(handle));
}

int wsp_play_run_pcm(wsp_device_handle_t *handle,
			const char *pcm_data, size_t pcm_size)
{
	snd_pcm_t *pcm_handle = wsp_to_pcm_device_handle(handle);
	int ret;

	ret = snd_pcm_writei(pcm_handle, pcm_data, pcm_size);
	if (ret == -EPIPE) {
		snd_pcm_prepare(pcm_handle);
		return 0;
	}
	return ret;
}
