#ifndef WSP__WAV_PLAY_H__
#define WSP__WAV_PLAY_H__


#include "wav_read.h"


typedef struct _wsp_device_handle wsp_device_handle_t;

int wsp_play_init_pcm_device(const char *name, wsp_device_handle_t **handle);
void wsp_play_term_pcm_device(wsp_device_handle_t *handle);


struct wsp_play_pcm_params {
};

int wsp_play_set_pcm_params(struct wsp_wav_header *wav_header);


#endif
