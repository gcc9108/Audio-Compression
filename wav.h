
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <math.h>


typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;

    char     datachunk_id[4];  // Subchunk2 Id
    uint32_t datachunk_size;   // Subchunk2 
}WavHeader;

WavHeader *header;

void wavread(char *file_name, int16_t **samples);

void wavwrite(char *file_name, int16_t *samples);

int8_t MuLaw_Encode(int16_t number);

void wavcompress(char *file_name, int16_t *samples);

void print16bitToOutput(uint16_t number, unsigned int size);

void print8bitToOutput(uint8_t number);

short int pwlog2(short int x);

int16_t MuLaw_Decode(int8_t number);

void ulaw(short *sample, const int count);

