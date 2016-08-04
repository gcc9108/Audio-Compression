
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <math.h>

#ifndef MU
#define MU 255
#endif

typedef struct {
    char        RIFF[4];
    uint32_t    riff_chunk_size;
    char        WAVE[4];

    char        fmt[4];
    uint32_t    fmt_chunk_size;
    uint16_t    fmt_code; // 1: Uncompressed PCM 7: mu-law
    uint16_t    n_channel;
    uint32_t    sample_rate;
    uint32_t    byte_rate;
    uint16_t    block_size;
    uint16_t    bits_per_sample;
} WAVHeader;

WAVHeader* header;

void wavread(const char *file_name, uint8_t **samples, uint32_t *data_chunk_size);

void wavwrite(const char* file_name, uint8_t* samples, uint32_t data_chunk_size);

void create_mu_header(uint32_t sample_number);

int8_t MuLaw_Encode(int16_t number);

void wavcompress(char *file_name, int16_t *samples  , unsigned int size);

short int pwlog2(short int x);

int16_t MuLaw_Decode(int8_t number);


unsigned char ulaw_encode_one(int16_t sample);

void ulaw_encode(int16_t *samples,  uint8_t *output, unsigned int number);

void ulaw(int16_t *samples,  uint8_t *output, unsigned int size);

void wavread_old(char *file_name, int16_t **samples);

void wavwrite_old(char *file_name, int16_t *samples);

void printBits(short num, char *type);



