#ifndef WSP__WAV_READ_H__
#define WSP__WAV_READ_H__


/* WAV file header's general contents */
struct wsp_wav_header {
	unsigned int file_size; /* size of the WAV file */

	short nr_channels; /* number of channels in the waveform */
	unsigned int sample_rate; /* samples per second */
	unsigned int bit_rate; /*bits per second =bits_per_sample*sample_rate */
	short block_align; /*size of each interleaved samples of all channels*/
	short bits_per_sample; /* each sample size in bits */

	unsigned int data_section_size; /* size of the data section */
};

/* Read WAV file header. */
int wsp_wav_read_header(const char *fp, struct wsp_wav_header *header);


#endif
