#include "wav.h"


// 
float fpwlog2( float x) {
    /* fpwlog2 = piecewise log2 */
    if( x < 1.0){
        return( - 1.0); /* error */
    }else if( x < 2.0){
        return( x- 1.0);
    }else if( x < 4.0){
        return( 1.0 + (x- 2.0)/2.0);
    }else if( x < 8.0){
        return( 2.0 + (x- 4.0)/4.0);
    }else if( x < 16.0){
        return( 3.0 + (x- 8.0)/8.0);
    }else if( x < 32.0){
        return( 4.0 + (x-16.0)/16.0);
    }else if( x < 64.0){
        return( 5.0 + (x-32.0)/32.0);
    }else if( x < 128.0){
        return( 6.0 + (x-64.0)/64.0);
    }else if( x < 256.0){
        return( 7.0 + (x-128.0)/128.0);
    }else{
        return( - 1.0); /* error */
    }
}


void wavread(char *file_name, int16_t **samples)
{
    int fd;
    if (!file_name)
        errx(1, "Filename not specified");
    if ((fd = open(file_name, O_RDONLY)) < 1)
        errx(1, "Error opening file");
    if (!header)
        header = (WavHeader*)malloc(sizeof(WavHeader));
    if (read(fd, header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "File broken: header");
    if (strncmp(header->chunk_id, "RIFF", 4) ||
        strncmp(header->format, "WAVE", 4))
        errx(1, "Not a wav file");
    if (header->audio_format != 1 && header->audio_format != 7 ){
        errx(1, "Only PCM encoding supported");
    }
    printf("Audio Format = %d\n", header->audio_format);
    if (*samples) free(*samples);
    *samples = (int16_t*)malloc(header->datachunk_size); // samples are 8 bits
    if (!*samples)
        errx(1, "Error allocating memory");
    printf("datachunk_size = %d\n", header->datachunk_size);

    if (read(fd, *samples, header->datachunk_size) < header->datachunk_size)
        errx(1, "File broken: samples");
    close(fd);
}

void wavwrite(char *file_name, int16_t *samples)
{
    int fd;
    if (!file_name)
        errx(1, "Filename not specified");
    if (!samples)
        errx(1, "Samples buffer not specified");
    if ((fd = creat(file_name, 0666)) < 1)
        errx(1, "Error creating file");
    if (write(fd, header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "Error writing header");
    if (write(fd, samples, header->datachunk_size) < header->datachunk_size)
        errx(1, "Error writing samples");
    close(fd);
}


void wavcompress(char *file_name, int16_t *samples)
{
    int fd,i;
    unsigned int data_length = header->datachunk_size / sizeof(int16_t);

    unsigned int *output = ( uint8_t *) malloc(data_length);
    // *output =  // samples are 8 bits
    // output = (uint16_t*)malloc(data_length);  // Size of Output
    if (!output)
        errx(1, "Error allocating memory");

    unsigned int size = 0;
    unsigned int count = 0;

    ulaw(samples, output);

    // output_header->datachunk_size  = header->datachunk_size / 2;
    // output_header->audio_format = 0x11;

    if (!file_name)
        errx(1, "Filename not specified");
    if (!output)
        errx(1, "Samples buffer not specified");
    if ((fd = creat(file_name, 0666)) < 1)
        errx(1, "Error creating file");
    if (write(fd, header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "Error writing header");
    if (write(fd, output,  data_length) <  data_length)
        errx(1, "Error writing samples");
   
    free(output);
}


void ulaw(int16_t *samples, int8_t *output)
{
    const int16_t MULAW_MAX =  0x1FFF;
    const int16_t MULAW_BIAS = 33;
    // const short MULAW_BIAS = 0x84 ;
    register int i, current_sample, sign, exponent, ms1, mantissa;  
    unsigned short magnitude = 0;
    printf("header->datachunk_size = %d\n", header->datachunk_size);
    // for(i=0; i< header->datachunk_size/2; i++)
    // {
    //      printf("#%d:        %d\n", i, samples[i]);
    // }   
    for(i=0; i< header->datachunk_size/2; i++)
    {   
        current_sample = samples[i];
        magnitude = current_sample;
        // printf("Current sample = : %d \n", current_sample);
        // printBits(current_sample, "int16_t");
        /*Save the sign bit by shifting it to the 8th spot used for the output.*/
        /*ANDing 0x80 clear all other 7 bits behind it */
        sign = (current_sample >> 7) & 0x80;
        // printBits(magnitude, "int16_t");
        /*Invert the sign of our sample if it's positive*/
        if (sign)
            current_sample = (short)-current_sample;
        /*Clip the current sample to max if it overflows.*/
        if (current_sample > MULAW_MAX)
            current_sample = MULAW_MAX;
        /*Add bias to our current sample to eliminate any samples without a '1' as MSB */
        // current_sample = 0x2F1F;
        magnitude = (short)(magnitude + MULAW_BIAS);
        // printf("current_sample = %d\n", magnitude);        
        /*exponent = eight bits on the right side of the sign bit. */
        exponent = magnitude >> 7;
        
        /*ms1 = Where the most significant 1 is. (exponent & 0xFF) gets rid of the extra 8th digit at the front*/
        ms1 = fpwlog2((exponent) & 0xFF);

        /*mantissa = 4 bits on the right side of the most significant 1*/
        mantissa = (magnitude >> (ms1+3)) & 0x0F;
        
        /*"concatenate" everything together, and then invert it for better transmission*/
        output[i] = ~(sign | (ms1 << 4) | mantissa);
        // printf("#%d , Compressed sample = : %d \n", i,  output[i] << 5);
        // printBits(~(sign | (ms1 << 4) | mantissa), "uint8_t");
    }
}


void printBits(short num, char *type){
    unsigned int size;
    if(type == "uint8_t"){
        // printf("uint8\n");
        size = sizeof(uint8_t);
        num  = (uint8_t)num;
    }else if(type == "int16_t"){
        // printf("int16\n");
        size = sizeof(int16_t);
        num  = (int16_t)num; 
    }else{
        // printf("short int\n");
        size = sizeof(short int);
        num  = (short int)num; 
    }

    unsigned int maxPow = 1<<(size*8-1);
    // printf("MAX POW : %u\n",maxPow);

    int i=0,j;
    for(;i<size*8;++i){
        // print last bit and shift left.
        printf("%u ",num&maxPow ? 1 : 0);
        num = num<<1;
    }
    printf("\n");

}

void updateWAVheader(){}




short int pwlog2(short int x){
    if(x < ( 1 <<8 ))
        return (-1); //ERROR
    if(x < (1<<9))
        return (x < (1 << 8));
    if(x < (1<<10))
        return (x < (1 >> 1));
    if(x < (1<<11))
        return ( (x >> 2) + (1 << 8));
    if(x < (1<<12))
        return ( (x >> 3) + (1 << 9));
}




void print16bitToOutput(uint16_t number , unsigned int size){
    unsigned char bytes[4];
    unsigned long n = number;
    bytes[0] = (n >> 24) & 0xFF;
    bytes[1] = (n >> 16) & 0xFF;
    bytes[2] = (n >> 8) & 0xFF;
    bytes[3] = n & 0xFF;

    printf("Size = %d , 16 bit number %d: %x %x %x %x\n",size, number,bytes[0], bytes[1], bytes[2], bytes[3]);
}

void print8bitToOutput(uint8_t number){
    unsigned char bytes[2];
    uint8_t n = number;
    bytes[0] = (n >> 8) & 0xF;
    bytes[1] = n & 0xF;

    printf("8 bit number %d: %x %x\n",n,bytes[0], bytes[1]);

}

