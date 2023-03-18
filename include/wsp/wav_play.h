#ifndef WSP__WAV_PLAY_H__
#define WSP__WAV_PLAY_H__

#include <stddef.h>


/*Some abstract opaque pointer to handle audio device and some related params.*/
typedef struct wsp_device_handle wsp_device_handle_t;

/* Initialize PCM device */
int wsp_play_init_pcm_device(const char *name, wsp_device_handle_t **p_handle);
/* Terminate PCM device */
void wsp_play_term_pcm_device(wsp_device_handle_t *handle);


/* Params to start PCM device with. */
struct wsp_play_pcm_params {
	short nr_channels; /* number of channels in the waveform */
	short bits_per_sample; /* each sample size in bits */
	unsigned int sample_rate; /* samples per second */
	unsigned int period_size; /* size of PCM period to play once per iteration */
};

/* Start playing PCM. */
int wsp_play_start_pcm(wsp_device_handle_t *handle,
			struct wsp_play_pcm_params *params);
/* Stop playing PCM. */
int wsp_play_stop_pcm(wsp_device_handle_t *handle);

/* Run playing PCM. */
long wsp_play_run_pcm(wsp_device_handle_t *handle,
			const void *pcm_buffer_data, unsigned int pcm_size);

/* Get PCM params. */
void wsp_play_get_pcm_params(wsp_device_handle_t *handle,
				struct wsp_play_pcm_params *params);

/* Get size of the buffer that stores samples of interleaved channels
 * in one period. */
static inline
size_t wsp_play_get_pcm_period_buffer_size(struct wsp_play_pcm_params *params)
{
	return params->period_size * params->nr_channels
		* params->bits_per_sample / 8;
}

#endif
