#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "polynomial.h"

//#define DEBUG 1

#ifdef DEBUG
	#define DBG(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG(...) {};
	#define DBGX(...) {};
#endif

#define MAX_STRLEN 10240

int main(int argc, char **argv) {
	Polynomial *a = NULL, *b = NULL, *result = NULL;
	char strA[MAX_STRLEN],strB[MAX_STRLEN];

	if(argc!=4) {
		printf("USAGE\r\n   polydiv operation poly_a poly_b precision\r\n");
		exit(1);
	}
	printf("operation: %s\r\n",argv[1]);

	int precision = 2;
	if(argc==5) {
		precision = atoi(argv[4]);
	}

	a = polynomial_from_string(argv[2]); 
	polynomial_to_string(a,strA,MAX_STRLEN,precision);
	printf("poly_a: \"%s\" converted as \"%s\"\r\n",argv[2],strA);
	polynomial_print(a);
	b = polynomial_from_string(argv[3]);
	polynomial_to_string(b,strB,MAX_STRLEN,precision);
	printf("poly_b: \"%s\" converted as \"%s\"\r\n",argv[3],strB);
	polynomial_print(b);

	if(strcmp(argv[1],"m")==0) {
		// multiplication
		result = polynomial_multiply(a,b);
	} else if(strcmp(argv[1],"d")==0) {
		// division
		result = polynomial_divide(a,b,100);
	} else if(strcmp(argv[1],"a")==0) {
		// addition
		result = polynomial_add(a,b);
	} else {
		// subtraction
		result = polynomial_subtract(a,b);
	}
	int slen = polynomial_to_string(result,strA,MAX_STRLEN,precision);
	printf("Result: %s (%d)\r\n",strA,slen);

	polynomial_free(a);
	polynomial_free(b);
	polynomial_free(result);

	return 0;
}
