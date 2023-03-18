#include <alsa/asoundlib.h>
#include <stddef.h>
#include <stdint.h>

#include <wsp/wav_read.h>
#include <wsp/wav_play.h>


void print_wav_header_contents(struct wsp_wav_header *header);

#define PCM_DEVICE "default"
#define PCM_PERIOD_SIZE 1024

int main(int argc, char *argv[])
{
	int ret;

	const char *wav_fp;
	FILE *wav_file;

	struct wsp_wav_header wav_header;
	char *wav_data_section;

	wsp_device_handle_t *handle;
	struct wsp_play_pcm_params params;

	char *pcm_buffer_data;
	size_t pcm_buffer_size;


	if (argc < 2) {
		printf("Usage: %s <wav_filepath>\n", argv[0]);
		return -1;
	}
	wav_fp = argv[1];
	wav_file = fopen(wav_fp, "r");
	if (wav_file == NULL) {
		ret = -1;
		fprintf(stderr, "Failed opening WAV file at %s\n", wav_fp);
		goto LEAVE;
	}

	ret = wsp_wav_read_header(wav_file, &wav_header);
	if (ret != 0) {
		fprintf(stderr,
			"Failed reading WAV header. %s\n", strerror(ret));
		goto LEAVE_wav_file;
	}

	print_wav_header_contents(&wav_header);

	wav_data_section = malloc(wav_header.data_section_size);
	if (!wav_data_section)
		goto LEAVE_wav_data_section;

	ret = wsp_wav_read_data_section(wav_file, wav_data_section,
					wav_header.data_section_size);
	if (ret != 0) {
		fprintf(stderr,
			"Failed reading data section. %s\n", strerror(ret));
		goto LEAVE_wav_data_section;
	}


	ret = wsp_play_init_pcm_device(PCM_DEVICE, &handle);
	if (ret < 0) {
		fprintf(stderr,
			"Failed initializing PCM device. %s\n", strerror(ret));
		goto LEAVE_TERM_dev_handle;
	}

	params.nr_channels = wav_header.nr_channels,
	params.bits_per_sample =  wav_header.bits_per_sample,
	params.sample_rate = wav_header.sample_rate;
	params.period_size = PCM_PERIOD_SIZE;

	ret = wsp_play_start_pcm(handle, &params);
	if (ret < 0) {
		fprintf(stderr, "Failed to start playing PCM device. %s\n",
			strerror(ret));
		goto LEAVE_STOP_dev_handle;
	}

	wsp_play_get_pcm_params(handle, &params);

	pcm_buffer_size = wsp_play_get_pcm_period_buffer_size(&params);
	pcm_buffer_data = malloc(pcm_buffer_size);
	if (!pcm_buffer_data) {
		fprintf(stderr, "Failed allocating memory for PCM buffer\n");
		goto LEAVE_STOP_dev_handle;
	}

	while ((ret = fread(pcm_buffer_data, pcm_buffer_size, 1, wav_file))) {
		long pcm_bytes_run;
		pcm_bytes_run = wsp_play_run_pcm(handle,
					pcm_buffer_data, params.period_size);

		if (pcm_bytes_run < 0) {
			ret = -(int)pcm_bytes_run;
			fprintf(stderr, "Failed running PCM: %s\n",
				strerror(ret));
			goto LEAVE_pcm_data;
		}
	}

	if (ret == 0 && ferror(wav_file)) {
		fprintf(stderr, "Error reading file. %s", wav_fp);
		ret = EXIT_FAILURE;
		goto LEAVE_pcm_data;
	}

	ret = 0;

LEAVE_pcm_data:
	free(pcm_buffer_data);

LEAVE_STOP_dev_handle:
	wsp_play_stop_pcm(handle);

LEAVE_TERM_dev_handle:
	wsp_play_term_pcm_device(handle);

LEAVE_wav_data_section:
	free(wav_data_section);

LEAVE_wav_file:
	fclose(wav_file);

LEAVE:
	return ret;
}


void print_wav_header_contents(struct wsp_wav_header *header)
{
	printf( "Channels:          %hd\n"
		"Sample rate:       %i \n"
		"Bit rate:          %i \n"
		"Block align:       %hd\n"
		"Bits per sample:   %hd\n"
		"Data section size: %i \n",

		header->nr_channels, header->sample_rate, header->bit_rate,
		header->block_align, header->bits_per_sample,
		header->data_section_size);
}

