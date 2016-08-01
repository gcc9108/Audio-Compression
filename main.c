#include "stdio.h"
#include "stdint.h"

/* To use the functions defined in Functions.c I need to #include Functions.h */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "wav.h"


int main(int argc, char **argv)
{
    int16_t *samples = NULL;
    if (argc != 2)
    {   puts ("\nEncode a single input file into a number of different output ") ;
        puts ("encodings. These output encodings can then be moved to another ") ;
        puts ("OS for testing.\n") ;
        puts ("    Usage : test <filename>\n") ;
        exit (1) ;
    };
    unsigned int x;
    char* filename = argv[1];
    printf("%s\n", filename);
    wavread(filename, &samples);
    printf("No. of channels: %d\n",     header->num_channels);
    printf("Chunk Size:      %d\n",     header->chunk_size);

    printf("Sample rate:     %d\n",     header->sample_rate);
    printf("Bit rate:        %dkbps\n", header->byte_rate*8 / 1000);
    printf("Bits per sample: %d\n\n",     header->bps);

    printf("Subchunk size : %d\n" , header-> datachunk_size );
    unsigned int i;
    unsigned int data_length = header->datachunk_size;

    uint16_t mask = 0x1000;
    short int sample[1];
    sample[0] = 0x2E9B;

    // ulaw(&sample, 10);
    printf("Sample 0:        %d\n", sample[0]);
    ulaw(sample, 1);
    // print8bitToOutput(MuLaw_Encode(sample));
    // printf("Size of INT16 is %zu\n", sizeof(int16_t) );
    // printf("Size of INT8 is %zu\n", sizeof(int8_t) );
    // for(i = 0; i < header->datachunk_size/2; i++){
    //     printf("#%d:        %d\n", i, samples[i]);
    //     printf("Encoded Sample = :        %d\n", MuLaw_Encode(samples[i]) );
    // }
    printf("Size of i is %d\n", i );

    // wavcompress("output-mu.wav", samples); //Write result to output 
    // wavwriteMu("track-mu.wav", samples);


    free(header);
    free(samples);

    // printf("Piecewise linear approximation of %d =  %lf \n", x,logBase2(x));
    return 0;
}