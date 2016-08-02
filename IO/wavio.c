#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
// #include <sys/time.h>
#include <time.h>

#ifndef MU
#define MU 255
#endif

typedef struct {
	char		RIFF[4];
	uint32_t	riff_chunk_size;
	char		WAVE[4];

	char		fmt[4];
	uint32_t	fmt_chunk_size;
	uint16_t	fmt_code; // 1: Uncompressed PCM 7: mu-law
	uint16_t	n_channel;
	uint32_t	sample_rate;
	uint32_t	byte_rate;
	uint16_t	block_size;
	uint16_t	bits_per_sample;
} WAVHeader;

WAVHeader* header;

void wavread(const char *file_name, uint8_t **samples, uint32_t *data_chunk_size)
{
	int file_descriptor;
	if (!file_name)
		errx(1, "Invalid file name");
	if (!samples)
		errx(1, "Invalid sample pointer");
	if (!data_chunk_size)
		errx(1, "Invalid data_chunk_size pointer");
	if ((file_descriptor = open(file_name, O_RDONLY)) < 1)
		errx(1, "Error opening file");
	if (!header)
		header = (WAVHeader*) malloc(sizeof(WAVHeader));
	if (read(file_descriptor, header, sizeof(WAVHeader)) < sizeof(WAVHeader))
		errx(1, "WAV header corrupted");
	if (strncmp(header->RIFF, "RIFF", 4) || strncmp(header->WAVE, "WAVE", 4))
		errx(1, "File is not a legal WAV file");
	if (strncmp(header->fmt, "fmt ", 4))
		errx(1, "Unsupported WAV header");
	if (*samples)
		free(samples);

	uint32_t size_to_the_end = header->riff_chunk_size - 8 - sizeof(WAVHeader);
	off_t data_chunk_offset = 0;
	
	char data_chunk_id[4];
	for(;;)
	{
		if(lseek(file_descriptor, data_chunk_offset, SEEK_SET) < 0)
			errx(1, "Error occurred finding data chunk");
		if(read(file_descriptor, data_chunk_id, 4) < 4)
			errx(1, "Error occurred reading data chunk id");
		if(strncmp(data_chunk_id, "data", 4))	
		{
			data_chunk_offset += 1;
			continue;
		}else{
			if(read(file_descriptor, data_chunk_size, 4) < 4)
				errx(1, "Error occurred reading data chunk size");
			break;
		}
	}
	*samples = (uint8_t*) malloc(*data_chunk_size);
	if (!*samples)
		errx(1, "Error allocating memory to store samples");
	if (read(file_descriptor, *samples, *data_chunk_size) < *data_chunk_size)
		errx(1, "Error reading samples");
	close(file_descriptor);
}

void create_mu_header(uint32_t sample_number)
{
	uint32_t output_file_size = sizeof(WAVHeader) + 8 + sample_number;
	header->riff_chunk_size = output_file_size - 8;
	header->fmt_code = 7;
	
	header->bits_per_sample = 8;
	header->block_size = (header->bits_per_sample >> 3) * header->n_channel;
	header->byte_rate = header->sample_rate * header->block_size;
	header->fmt_chunk_size = 16;
}

void wavwrite(const char* file_name, uint8_t* samples, uint32_t data_chunk_size)
{
	int fd;
	char data_chunk_id[4] = {'d', 'a', 't', 'a'};
	if (!file_name)
		errx(1, "Unspecified filename");
	if (!samples)
		errx(1, "Unspecified sample pointer");
	if((fd = open(file_name, O_RDWR|O_CREAT, 0666)) < 1)
		errx(1, "Error creating output file");
	if(write(fd, header, sizeof(WAVHeader)) < sizeof(WAVHeader))
		errx(1, "Error writing header");
	if(write(fd, data_chunk_id, 4) < 4)
		errx(1, "Error data chunk id");
	if(write(fd, &data_chunk_size, 4) < 4)
		errx(1, "Error data chunk size");
	if(write(fd, samples, data_chunk_size) < data_chunk_size)
		errx(1, "Error writing samples");
	close(fd);
}

// unsigned char flin2mu(int16_t input_frame, uint8_t mu, char print_debug)
inline unsigned char flin2mu(int16_t input_frame)
{
	uint8_t sign_bit = ((uint16_t)input_frame) >> 15;
	uint16_t magnitude = (sign_bit)? -input_frame: input_frame;
	if(magnitude > 32767)
		magnitude = 32767;
	// if(print_debug)
	// 	printf("Magnitude: %u\n", magnitude);
	double x = magnitude / 32767.0;
	double result = (log(1.0 + MU * x)/log(1.0 + MU));
	// if(print_debug)
	// 	printf("Result: %g\n", result);
	uint8_t return_value = rint(result * 127.0);
	// if(print_debug)
	// 	printf("Return value: %u\n", return_value);
	return_value = return_value | (sign_bit << 7);

	return return_value;
}

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';

	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
	long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
	result->tv_sec = diff / 1000000;
	result->tv_usec = diff % 1000000;

	return (diff<0);
}

void timeval_print(struct timeval *tv)
{
	char buffer[30];
	time_t curtime;

	curtime = tv->tv_sec;
	strftime(buffer, 30, "%Y-%m-%d  %T", localtime(&curtime));
	printf("%s.%06ld\n", buffer, tv->tv_usec);
}

// In the WAV File, 8-bit samples are stored as unsigned bytes, ranging from 0 to 255. 
// The 16-bit samples are stored as signed integers in 2's-complement.
int main(int argc, char const *argv[])
{
	struct timeval tvBegin, tvEnd, tvDiff;
	uint8_t *samples = NULL;
	uint32_t data_chunk_size;
	if(argc < 2){
		printf("usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	wavread(argv[1], &samples, &data_chunk_size);
	printf("No. of channels:   %d\n",      header->n_channel);
	printf("RIFF chunk size:   %d\n",      header->riff_chunk_size);
	printf("FMT chunk size:    %d\n",      header->fmt_chunk_size);
	printf("Format code:       %d\n",      header->fmt_code);
	printf("Sample rate:       %d\n",      header->sample_rate);
	printf("Byte rate:         %d Bps\n",  header->byte_rate);
	printf("Bit rate:          %d kbps\n", header->byte_rate * 8 / 1000);
	printf("Bits per sample:   %d\n",      header->bits_per_sample);
	printf("Block size:        %d\n\n",    header->block_size);

	register uint32_t i;
	uint32_t sample_number = data_chunk_size / (header->bits_per_sample >> 3);

	if(header->bits_per_sample == 8){
		printf("Sample size is unsigned 8 bits\n");
		printf("Now in debug printing mode. Print first 50 samples.\n");
		uint8_t sample;
		register uint8_t* sample_pointer = (uint8_t*) samples;
		if(header->fmt_code == 7){
			printf("Processing u-Law compressed file\n");
			for(i = sample_number; i > 0; i--){
				sample = ~(*sample_pointer);
				sample_pointer++;
				printf("Sample [%d] = %s%d\n", sample_number - i, sample >> 7? "-":"+", sample & 0x7f);
				printf("\tBinary: %s\n", byte_to_binary(sample));
				if(sample_number - i > 50){
					exit(0);
				}
			}
		}else if(header->fmt_code == 1){
			printf("Processing normal PCM file\n");
			for(i = sample_number; i > 0; i--){
				sample = *sample_pointer;
				sample_pointer++;
				printf("Sample [%d] = %s%d\n", sample_number - i, sample >> 7? "-":"+", sample & 0x7f);
				printf("\tBinary: %s\n", byte_to_binary(sample));
				if(sample_number - i > 50){
					exit(0);
				}
			}
			printf("Sample [%d] = %d\n", sample_number - i, sample);
		}else{
			errx(1, "Unsupported compression format");
		}
	}else if(header->bits_per_sample == 16){
		// Priority
		printf("Sample size is signed 16 bits\n");
		int16_t sample;
		register int16_t* sample_pointer = (int16_t*) samples;
		printf("Sample number: %d\n", sample_number);
		uint8_t output_samples[sample_number];
		register uint8_t* output_sample_pointer = output_samples;
		gettimeofday(&tvBegin, NULL);
		printf("Compression started at: \n");
		timeval_print(&tvBegin);
		for(i = sample_number; i > 0; i--){
			sample = *sample_pointer;

			// if(sample_number - i < 50){
			// 	printf("Sample [%d] = %d\n", sample_number - i, sample);
			// }
			// *output_sample_pointer = ~flin2mu(sample, MU, sample_number - i < 50);
			*output_sample_pointer = ~flin2mu(sample);
			sample_pointer++;
			output_sample_pointer++;
		}
		gettimeofday(&tvEnd, NULL);
		printf("Compression finished at: \n");
		timeval_print(&tvEnd);
		printf("Time elapsed: \n");
		timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
		printf("%ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);
		create_mu_header(sample_number);
		wavwrite(argc == 3? argv[2]: "output.wav", output_samples, sample_number);
	}else if(header->bits_per_sample == 32){
		errx(1, "The current version can not handle 32 bit floating point WAV file");
	}
	return 0;
}
