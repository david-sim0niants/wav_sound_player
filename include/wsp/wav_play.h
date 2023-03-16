#ifndef WSP__WAV_PLAY_H__
#define WSP__WAV_PLAY_H__

#include <stddef.h>


typedef struct wsp_device_handle wsp_device_handle_t;

/* Initialize PCM device */
int wsp_play_init_pcm_device(const char *name, wsp_device_handle_t **handle);
/* Terminate PCM device */
void wsp_play_term_pcm_device(wsp_device_handle_t *handle);


/* Params to start PCM device with. */
struct wsp_play_pcm_params {
	short nr_channels; /* number of channels in the waveform */
	short bits_per_sample; /* each sample size in bits */
	unsigned int sample_rate; /* samples per second */
};

/* Start playing PCM. */
int wsp_play_start_pcm(wsp_device_handle_t *handle,
			struct wsp_play_pcm_params *params);
/* Stop playing PCM. */
int wsp_play_stop_pcm(wsp_device_handle_t *handle);

/* Run playing PCM. */
int wsp_play_run_pcm(wsp_device_handle_t *handle,
			const char *pcm_data, size_t pcm_size);


#endif
