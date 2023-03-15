#include <alsa/asoundlib.h>
#include <stddef.h>
#include <stdint.h>

#include <wsp/wav_read.h>

snd_pcm_format_t wsp_bpS_to_snd_pcm_format(unsigned int bits_per_sample);


#define PCM_DEVICE "default"

int main(int argc, char *argv[])
{
	int ret;
	struct wsp_wav_header wav_header;

	const char *wav_fp;
	int wav_fd;

	unsigned int tmp, period_time;

	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;

	char *buffer_data;
	size_t buffer_size;
	unsigned int loops;

	if (argc < 2) {
		printf("Usage: %s <wav_filepath>\n", argv[0]);
		return -1;
	}
	wav_fp = argv[1];

	ret = wsp_wav_read_header(wav_fp, &wav_header);
	if (ret != 0) {
		errno = ret;
		perror("Failed reading WAV header");
		return 1;
	}

	ret = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("Failed opening %s pcm device. %s",
		       PCM_DEVICE, snd_strerror(ret));
		return EXIT_FAILURE;
	}


	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	ret = snd_pcm_hw_params_set_access(pcm_handle, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		printf("Failed setting the interleaved access mode. %s\n",
			snd_strerror(ret));
		return EXIT_FAILURE;
	}

	ret = snd_pcm_hw_params_set_format(pcm_handle, params,
					   SND_PCM_FORMAT_S16_LE);
	if (ret < 0) {
		printf("Failed setting the PCM format. %s\n",
		       	snd_strerror(ret));
		return EXIT_FAILURE;
	}

	ret = snd_pcm_hw_params_set_channels(pcm_handle, params,
						wav_header.nr_channels);
	if (ret < 0) {
		printf("Failed setting the PCM channels. %s\n",
			snd_strerror(ret));
		return EXIT_FAILURE;
	}

	ret = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &wav_header.sample_rate, 0);
	if (ret < 0) {
		printf("Failed setting PCM rate. %s\n", snd_strerror(ret));
		return EXIT_FAILURE;
	}

	ret = snd_pcm_hw_params(pcm_handle, params);
	if (ret < 0) {
		printf("Failed to set hardware parameters. %s\n",
			snd_strerror(ret));
	}

	printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

	printf("PCM state: %s\n",snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	snd_pcm_hw_params_get_channels(params, &tmp);
	printf("channels: %i \n", tmp);

	if (tmp == 1)
		printf("(mono)\n");
	else if (tmp == 2)
		printf("(stereo)\n");

	snd_pcm_hw_params_get_rate(params, &tmp, NULL);
	printf("rate: %d bps\n", tmp);


	snd_pcm_hw_params_get_period_size(params, &frames, NULL);

	snd_pcm_hw_params_get_period_time(params, &period_time, NULL);

	buffer_size = frames * wav_header.nr_channels * 2;
	buffer_data = (char *)malloc(buffer_size);

	printf("buffer_size: %lu\n", buffer_size);

	wav_fd = open(wav_fp, 0, O_RDONLY);

	int header_bytes_left = 44;
	char header[44];
	while (header_bytes_left > 0) {
		ret = read(wav_fd, header, header_bytes_left);
		header_bytes_left -= ret;
	}

	int32_t data_bytes_left = *(int32_t *)(header + 40);
	lseek(wav_fd, data_bytes_left, SEEK_CUR);

	printf("Period time: %d\n", period_time);

	while ((ret = read(wav_fd, buffer_data, buffer_size))) {

		// for (size_t i = 0, j = 0; i < samples_per_period; ++i, ++j) {
		// 	printf("%06hd ", samples_data[i]);
		// 	if (j == (j >> 3) << 3) {
		// 		printf("\n");
		// 		fsync(1);
		// 		j = 0;
		// 	}
		// }

		ret = snd_pcm_writei(pcm_handle, buffer_data, buffer_size);
		if (ret == -EPIPE) {
			printf("XRUN.\n");
			snd_pcm_prepare(pcm_handle);
		} else if (ret < 0) {
			printf("Failed setting PCM device. %s\n",
				snd_strerror(ret));
		}
		sleep(1);
	}

	close(wav_fd);

	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
	free(buffer_data);

	return 0;
}


snd_pcm_format_t wsp_bpS_to_snd_pcm_format(unsigned int bits_per_sample)
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
