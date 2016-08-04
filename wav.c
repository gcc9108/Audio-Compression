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


// Wav file write mu
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

void wavcompress(char *file_name, int16_t *samples , unsigned int size)
{
    int fd,i;
    unsigned int data_length = size;

    unsigned int *output = ( uint8_t *) malloc(data_length);
    // *output =  // samples are 8 bits
    // output = (uint16_t*)malloc(data_length);  // Size of Output
    if (!output)
        errx(1, "Error allocating memory");

    unsigned int count = 0;
    ulaw(samples, output, data_length);

    // output_header->datachunk_size  = header->datachunk_size / 2;
    // output_header->audio_format = 0x11;

    if (!file_name)
        errx(1, "Filename not specified");
    if (!output)
        errx(1, "Samples buffer not specified");
    if ((fd = creat(file_name, 0666)) < 1)
        errx(1, "Error creating file");
    if (write(fd, header, sizeof(WAVHeader)) < sizeof(WAVHeader))
        errx(1, "Error writing header");
    if (write(fd, output,  data_length) <  data_length)
        errx(1, "Error writing samples");
   
    free(output);
}


void ulaw_encode(int16_t *samples, uint8_t *output_samples, unsigned int number)
{
    const uint16_t MULAW_MAX =  32767;
    const uint16_t MULAW_BIAS = 33;
    register unsigned int i, sign, exponent, ms_bit, mantissa;  
    unsigned short magnitude = 0; 
    unsigned short sample; 
    for(i = number; i > 0; i--){
        sample = samples[i];
        /*Save the sign bit by shifting it to the 8th spot used for the output.*/
        /*ANDing 0x80 clear all other 7 bits behind it */
        sign = (sample >> 8) & 0x80;
        /*Invert the sign of our sample if it's positive*/
        if (sign)
            sample = (uint16_t)-sample;
            /*Clip the current sample to max if it overflows.*/
        if (sample > MULAW_MAX)
            sample = MULAW_MAX;
        /*Add bias to our current sample to eliminate any samples without a '1' as MSB */
        // current_sample = 0x2F1F;
        magnitude = (short)(sample + MULAW_BIAS);
        // printf("current_sample = %d\n", magnitude);        
        /*exponent = eight bits on the right side of the sign bit. */
        exponent = magnitude >> 7;
        
        /*ms1 = Where the most significant 1 is. (exponent & 0xFF) gets rid of the extra 8th digit at the front*/
        ms_bit = log2((exponent) & 0xFF);

        /*mantissa = 4 bits on the right side of the most significant 1*/
        mantissa = (magnitude >> (ms_bit+3)) & 0x0F;
            /*"concatenate" everything together, and then invert it for better transmission*/
        output_samples[i] = ~(sign | (ms_bit << 4) | mantissa);
    }
}


unsigned char ulaw_encode_one(int16_t sample)
{
    const uint16_t MULAW_MAX =  32767;
    const uint16_t MULAW_BIAS = 33;
    register unsigned int i, sign, exponent, ms_bit, mantissa;  
    unsigned short magnitude = 0; 
    magnitude = sample;
    // printf("Current sample = : %d \n", current_sample);
    // printBits(current_sample, "int16_t");
    /*Save the sign bit by shifting it to the 8th spot used for the output.*/
    /*ANDing 0x80 clear all other 7 bits behind it */
    sign = (sample >> 8) & 0x80;
    /*Invert the sign of our sample if it's positive*/
    if (sign)
        sample = -sample;
        /*Clip the current sample to max if it overflows.*/
    if (sample > MULAW_MAX)
        sample = MULAW_MAX;
    /*Add bias to our current sample to eliminate any samples without a '1' as MSB */
    // current_sample = 0x2F1F;
    magnitude = (short)(sample + MULAW_BIAS);
    // printf("current_sample = %d\n", magnitude);        
    /*exponent = eight bits on the right side of the sign bit. */
    exponent = magnitude >> 7;
    
    /*ms1 = Where the most significant 1 is. (exponent & 0xFF) gets rid of the extra 8th digit at the front*/
    ms_bit = log2((exponent) & 0xFF);  

    /*mantissa = 4 bits on the right side of the most significant 1*/
    mantissa = (magnitude >> (ms_bit+3)) & 0x0F;
    
    /*"concatenate" everything together, and then invert it for better transmission*/
    return (sign | (ms_bit << 4) | mantissa );
    // printf("#%d , Compressed sample = : %d \n", i,  output[i] << 5);

}
void ulaw(int16_t *samples, uint8_t *output, unsigned int size)
{
    const int16_t MULAW_MAX =  0x1FFF;
    const int16_t MULAW_BIAS = 33;
    register unsigned int i, current_sample, sign, exponent, ms1, mantissa;  
    unsigned short magnitude = 0;
    for(i= 0; i < size; i++)
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