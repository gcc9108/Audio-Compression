#include "wav.h"


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
    *samples = (int8_t*)malloc(header->datachunk_size); // samples are 8 bits
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
    FILE *fp = NULL;

    unsigned int *output = NULL;
    unsigned int data_length = header->datachunk_size / sizeof(int16_t);
    // unsigned int new_data_length = header->datachunk_size / sizeof(int16_t) /2;
    output = (int16_t*)malloc(data_length);
    if (!output)
        errx(1, "Error allocating memory");

    // printf("The size of output is %zu\n", sizeof(output));

    unsigned int size = 0;
    unsigned int count = 0;
    printf("The size of samples is %d\n", data_length);
    printf("size of half samples is %d\n", data_length/2);

    fp = fopen(file_name , "w+" );
    if(fp != NULL)
    {
        printf("Size of wave header = %x\n", sizeof(WavHeader) );

        fwrite(header , 1 , sizeof(WavHeader) , fp );
    }
    else
        errx(1, "Error writing file");


    while(size < data_length){     //header->datachunk_size 
        int8_t d1 = MuLaw_Encode(samples[size]);
        printf("D1 = %d\n" , d1);
        print8bitToOutput(d1);
        fwrite(&d1 , 1 , sizeof(d1) , fp );
        size ++;
    }
    fclose(fp);
    free(output);
}

void updateWAVheader(){
    
}

int8_t MuLaw_Encode(int16_t number){
    const uint16_t MULAW_MAX = 0x1FFF;   // Maximum value 2^13 -1  = 8191
    const uint16_t MULAW_BIAS = 33;      // Bias value
    uint16_t mask = 0x1000;
    uint8_t mu_sign = 0;
    uint8_t position = 12;
    uint8_t lsb = 0;
    if(number < 0)
    {
        number = -number;
        mu_sign   = 0x80;   // 1000 0000
    } // Getting mu-sgn bit

    number += MULAW_BIAS;   // Get magnitude + BIAS value
    printf("bias integer %d\n", number);
    if(number > MULAW_MAX){
        number= MULAW_MAX;   // Ensure number does not exceed MAX value for 8 bit encoding
    }
    printf("number =  %d\n", number);


    for(; ((number & mask) != mask && position >= 5); mask >>= 1, position--  );
    lsb = (number >> (position -4)) & 0x0f;
    return (~(mu_sign | ((position) << 4) | lsb));
}


int16_t MuLaw_Decode(int8_t number)
{
   const uint16_t MULAW_BIAS = 33;
   uint8_t sign = 0, position = 0;
   int16_t decoded = 0;
   number = ~number;
   if (number & 0x80)
   {
      number &= ~(1 << 7);
      sign = -1;
   }
   position = ((number & 0xF0) >> 4) + 5;
   decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
             | (1 << (position - 5))) - MULAW_BIAS;
   return (sign == 0) ? (decoded) : (-(decoded));
}


void printBits(unsigned int num, char *type){
    unsigned int size;
    if(type == "uint8_t"){
        printf("uint8\n");
        size = sizeof(uint8_t);
        num  = (uint8_t)num;
    }else if(type == "int16_t"){
        printf("int16\n");

        size = sizeof(int16_t);
        num  = (int16_t)num; 
    }else{
        printf("short int\n");

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
}


void ulaw(short *sample, const int count)
{
    const int16_t MULAW_MAX =  0x1FFF;
    // const int16_t MULAW_BIAS = 33;
    const int16_t MULAW_BIAS = 0x84 ;
    register int i, current_sample, sign, exponent, ms1, mantissa;  
    
    for(i=0; i<count; i++)
    {
        current_sample = sample[i];

        printf("\ncurrent sample = : %d \n", current_sample);

        printBits(current_sample, "int16_t");

        /*Save the sign bit by shifting it to the 8th spot used for the output.*/
        /*ANDing 0x80 clear all other 7 bits behind it */
        sign = (current_sample >> 7) & 0x80;
        
        /*Invert the sign of our sample if it's positive*/
        if (sign)
            current_sample = (short)-current_sample;
        
        /*Clip the current sample to max if it overflows.*/
        if (current_sample > MULAW_MAX)
            current_sample = MULAW_MAX;
        printf("current_sample = %d\n", current_sample);

        /*Add bias to our current sample to eliminate any samples without a '1' as MSB */
        // current_sample = 0x2F1F;
        current_sample = (short)(current_sample + MULAW_BIAS);

        printf("Value = %d\n", 0x2F1F);

        printf("current_sample = %d\n", current_sample);

        printBits(0x2F1F, "");
        
        /*exponent = eight bits on the right side of the sign bit. */
        exponent = current_sample >> 7;
        
        /*ms1 = Where the most significant 1 is. (exponent & 0xFF) gets rid of the extra 8th digit at the front*/
        ms1 = log2((exponent) & 0xFF);

        /*mantissa = 4 bits on the right side of the most significant 1*/
        mantissa = (current_sample >> (ms1+3)) & 0x0F;
        
        /*"concatenate" everything together, and then invert it for better transmission*/
        sample[i] = ~(sign | (ms1 << 4) | mantissa);
        printf("Compressed sample = : %d \n", (uint8_t)sample[i] << 5);
        printBits(sample[i], "uint8_t");

    }
}

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
    }   
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

