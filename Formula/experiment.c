#include <stdio.h>
#include <math.h>
#include <string.h>     /* strcat */
#include <stdlib.h>     /* strtol */

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

float fpwlog2(float x)
{
	if(x < 1.0){
		return -1.0;
	}
	if(x < 2.0){
		return x - 1.0;
	}
	if(x < 4.0){
		return 1.0 + (x - 2.0) / 2.0;
	}
	if(x < 8.0){
		return 2.0 + (x - 4.0) / 4.0;
	}
	if(x < 16.0){
		return 3.0 + (x - 8.0) / 8.0;
	}
	if(x < 32.0){
		return 4.0 + (x - 16.0)/ 16.0;
	}
	if(x < 64.0){
		return 5.0 + (x - 32.0)/ 32.0;
	}
	if(x < 128.0){
		return 6.0 + (x - 64.0)/ 64.0;
	}
	if(x < 256.0){
		return 7.0 + (x - 128.0)/ 128.0;
	}
	if(x < 512.0){
		return 8.0 + (x - 256.0)/ 256.0;
	}
	if(x < 1024.0){
		return 9.0 + (x - 512.0)/ 512.0;
	}
	if(x < 2048.0){
		return 10.0 + (x - 1024.0)/ 1024.0;
	}
	if(x < 4096.0){
		return 11.0 + (x - 2048.0)/ 2048.0;
	}
	return -1;
}

unsigned char flin2mu(short input_frame, unsigned char mu)
{
	unsigned char sign_bit = ((unsigned short)input_frame) >> 15; 
	unsigned short magnitude = (sign_bit)? -input_frame: input_frame;
	float x = magnitude / 32768.0;
	float result = (log(1.0 + mu * x)/log(1.0 + mu));
	unsigned char return_value = result * 128.0;
	return_value = return_value | (sign_bit << 7);
	return return_value;
}

int main()
{
	float i = 0.0;
//	for(i = 0.0; i < 4096.0; i++){
//		printf("log(%g, 2) = %.10f\n", i, fpwlog2(i));
//	}
	short input = -16384;
	unsigned char mu = 255;
	unsigned char result = flin2mu(input, mu);
	printf("flin2mu(%d) = %x (%s)\n", input, result, byte_to_binary(result));
	return 0;
}
