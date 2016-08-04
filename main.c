#include "stdio.h"
#include "stdint.h"

/* To use the functions defined in Functions.c I need to #include Functions.h */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "wav.h"


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



int main(int argc, char **argv)
{
    struct timeval tvBegin;
    struct timeval tvEnd;
    struct timeval tvDiff;
    uint8_t *samples = NULL;
    uint32_t data_chunk_size;

    if (argc < 2)
    {   
        puts ("Usage : main <filename>\n") ;
        exit (1) ;
    };
    unsigned int x;
    char* filename = argv[1];
    printf("%s\n", filename);
    wavread(filename,  &samples, &data_chunk_size);
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
    uint32_t sample_number = data_chunk_size / (header->bits_per_sample >> 3) - 46;
    printf("sample_number:        %d\n\n",    sample_number);

    if(header->bits_per_sample == 16){
        // Priority
        printf("Sample size is signed 16 bits\n");
        int16_t sample;
        int16_t* sample_pointer = (int16_t*) samples;
        printf("Sample number: %d\n", sample_number);
        // uint8_t output_samples[sample_number];
        uint8_t *output_samples = (uint8_t*) malloc(sample_number * sizeof(uint8_t));
        if( !output_samples){
            errx(1,"Error Allocating memory");
        }
        uint8_t* output_sample_pointer = output_samples;
        gettimeofday(&tvBegin, NULL);
        printf("Compression started at: \n");
        timeval_print(&tvBegin);
        ulaw_encode(sample_pointer, output_sample_pointer, sample_number);
        // printf("The first value of the sample is %d\n", samples[0]);
        // printBits(samples[0], "int16_t");
        // printf("The first value of the mucode is %d\n", output_samples[0]);
        // printBits(output_samples[0], "uint8_t");
        // ulaw_encode(sample_pointer, output_sample_pointer, sample_number);
        gettimeofday(&tvEnd, NULL);
        printf("Compression finished at: \n");
        timeval_print(&tvEnd);
        printf("Time elapsed: \n");
        timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
        printf("%ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);
        create_mu_header(sample_number);
        wavwrite(argc == 3? argv[2]: "output.wav", output_samples, sample_number);
        free(output_samples);

    }
  
    free(header);
    free(samples);
    return 0;
}