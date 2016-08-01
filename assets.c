
#include "assets.h"

#define M_LOG2E 1.44269504088896340736 //log2(e)

int Sum(int a, int b)
{
    return a+b;
}

double logBase2(int x){
	printf("Log(%d)", x) ;

	printf(" = %f \n", log(x) ) ;
    return log(x) * M_LOG2E;
}

// Debug
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


