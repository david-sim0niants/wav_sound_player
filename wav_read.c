#include "wsp/wav_read.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>


#define WAV_HEADER_SIZE 44

int wsp_wav_read_header(const char *fp, struct wsp_wav_header *header)
{
	FILE *wav_file;
	char wav_file_header[WAV_HEADER_SIZE];

	wav_file = fopen(fp, "r");
	if (wav_file == NULL) {
		return ENOENT;
	}

	ssize_t bytes_read = fread(wav_file_header,WAV_HEADER_SIZE,1, wav_file);

	if (bytes_read < WAV_HEADER_SIZE)
		return ENODATA;

	header->file_size 	  = *(unsigned int *) 	(wav_file_header + 4);
	header->nr_channels 	  = *(short *) 		(wav_file_header + 22);
	header->sample_rate 	  = *(int *)  		(wav_file_header + 24);
	header->bit_rate 	  = *(int *)  		(wav_file_header + 28);
	header->block_align 	  = *(short *) 		(wav_file_header + 32);
	header->bits_per_sample   = *(short *) 		(wav_file_header + 34);
	header->data_section_size = *(unsigned int *)  	(wav_file_header + 40);

	return 0;
}
