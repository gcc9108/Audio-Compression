#include <stdio.h>


short int pwlog2(short int X) {
    /* pwlog2 = piecewise log2 */
    printf("1 shift left 9 bits = %d\n", (1<<10));
    if( X < (1<<8)) // 256
        return(-1); /* error */
    if( X < (1<<9)) // 512
        return( X - (1<<8));
    if( X < (1<<10))  // 1024
        return( X >> 1);
    if( X < (1<<11)) // 2048
        return( (X >> 2) + (1<<8));
    if( X < (1<<12)) // 4096
        return( (X >> 3) + (1<<9));
    return( -1); /* error */
}

int main(int argc, char **argv)
{
    short int temp = 1000;
    short int x = pwlog2(temp);
    printf("Piecewise linear approximation of log(%d) =  %hd \n", temp, x );
    return 0;
}