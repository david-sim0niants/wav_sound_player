#include <alsa/asoundlib.h>
#include <stddef.h>
#include <stdint.h>

#include <wsp/wav_read.h>
#include <wsp/wav_play.h>


#define PCM_DEVICE "default"

int main(int argc, char *argv[])
{
	int ret;

	const char *wav_fp;
	FILE *wav_file;

	struct wsp_wav_header wav_header;
	char *wav_data_section;

	wsp_device_handle_t *dev_handle;
	struct wsp_play_pcm_params params;

	char *pcm_data;
	size_t pcm_size;


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


	ret = wsp_play_init_pcm_device(PCM_DEVICE, &dev_handle);
	if (ret < 0) {
		fprintf(stderr,
			"Failed initializing PCM device. %s\n", strerror(ret));
		goto LEAVE_TERM_dev_handle;
	}

	params.nr_channels = wav_header.nr_channels,
	params.bits_per_sample = wav_header.bits_per_sample,
	params.sample_rate = wav_header.sample_rate;

	ret = wsp_play_start_pcm(dev_handle, &params);
	if (ret < 0) {
		fprintf(stderr, "Failed to start playing PCM device. %s\n",
			strerror(ret));
		goto LEAVE_STOP_dev_handle;
	}


	pcm_size = wav_header.sample_rate * wav_header.bits_per_sample / 8;
	// pcm_size = 4096;
	pcm_data = malloc(pcm_size);
	if (!pcm_data) {
		fprintf(stderr, "Failed allocating memory for PCM buffer\n");
		goto LEAVE_STOP_dev_handle;
	}

	while ((ret = fread(pcm_data, pcm_size, 1, wav_file))) {

		// for (size_t i = 0, j = 0; i < samples_per_period; ++i, ++j) {
		// 	printf("%06hd ", samples_data[i]);
		// 	if (j == (j >> 3) << 3) {
		// 		printf("\n");
		// 		fsync(1);
		// 		j = 0;
		// 	}
		// }

		ret = wsp_play_run_pcm(dev_handle, pcm_data, pcm_size);
		if (ret < 0) {
			fprintf(stderr, "Failed running PCM\n");
			goto LEAVE_pcm_data;
		}
	}

	ret = 0;

LEAVE_pcm_data:
	free(pcm_data);

LEAVE_STOP_dev_handle:
	wsp_play_stop_pcm(dev_handle);

LEAVE_TERM_dev_handle:
	wsp_play_term_pcm_device(dev_handle);

LEAVE_wav_data_section:
	free(wav_data_section);

LEAVE_wav_file:
	fclose(wav_file);

LEAVE:
	return ret;
}
