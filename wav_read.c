#include "wsp/wav_read.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>


#define WAV_HEADER_SIZE 44

int wsp_wav_read_header(FILE *wav_file, struct wsp_wav_header *header)
{
	int ret;
	char wav_file_header[WAV_HEADER_SIZE];
	size_t blocks_read;

	ret = fseek(wav_file, 0, SEEK_SET);
	if (ret)
		return ret;

	blocks_read = fread(wav_file_header, WAV_HEADER_SIZE, 1, wav_file);

	if (blocks_read == 0)
		return ENODATA;

	header->file_size 	  = *(unsigned int *) 	(wav_file_header + 4);
	header->nr_channels 	  = *(short *) 		(wav_file_header + 22);
	header->sample_rate 	  = *(unsigned int *)  	(wav_file_header + 24);
	header->bit_rate 	  = *(unsigned int *) 	(wav_file_header + 28);
	header->block_align 	  = *(short *) 		(wav_file_header + 32);
	header->bits_per_sample   = *(short *) 		(wav_file_header + 34);
	header->data_section_size = *(unsigned int *)  	(wav_file_header + 40);

	return 0;
}

int wsp_wav_read_data_section(FILE *wav_file, char *data, size_t size)
{
	int ret;

	ret = fseek(wav_file, WAV_HEADER_SIZE, SEEK_SET);
	if (ret)
		return ret;

	ret = fread(data, size, 1, wav_file);
	if (ret == 0)
		return ENODATA;

	return 0;
}
