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
