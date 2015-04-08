#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "polynomial.h"

//#define DEBUG

#ifdef DEBUG
	#define DBG(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG(...) {};
	#define DBGX(...) {};
#endif

char g_strbuf[10240];
#define G_STRBUF_LEN 10240

int test_out_of_order_addition_of_components() {
	int result = 0;
	Polynomial *p = polynomial_new();
	Component *c = component_new();
	c->coefficient = 1; c->exponent = 0;
	polynomial_add_component(p,c);
	c->coefficient = 1; c->exponent = 1;
	polynomial_add_component(p,c);
	c->coefficient = 4; c->exponent = 4;
	polynomial_add_component(p,c);
	c->coefficient = 5; c->exponent = 5;
	polynomial_add_component(p,c);
	c->coefficient = 2; c->exponent = 2;
	polynomial_add_component(p,c);
	c->coefficient = 3; c->exponent = 3;
	polynomial_add_component(p,c);
	c->coefficient = 6; c->exponent = 6;
	polynomial_add_component(p,c);

	char str_a[128];
	char *expected_str_a = "6.00x^6.00+5.00x^5.00+4.00x^4.00+3.00x^3.00+2.00x^2.00+x+1.00";
	polynomial_to_string(p,str_a,128,2);
	if(strcmp(expected_str_a,str_a)!=0) {
		fprintf(stderr,"Error in constructed polynomial, expected \"%s\", got \"%s\"\r\n",expected_str_a,str_a);
		result = 1;
	}

	/*
	polynomial_free(p);
	component_free(c);

	char *expected_str_b = "+6.00x^7.00+4.00x^5.00-3.00x^2.00+6.00x-34.00";
	p = polynomial_from_string("-34+2x-3x^2+6x^7+4x+4x^5");
	polynomial_to_string(p,str,128,2);

	polynomial_free(p);

	if(strcmp(expected_str_b,str)!=0) {
		fprintf(stderr,"Error in constructed polynomial, expected \"%s\", got \"%s\"\r\n",expected_str_b,str);
		result = 1;
	}
	*/

	return result;
}

int check_polynomial_multiplication(char *lhs, char *rhs, char *expected, int precision) {
	DBG("Polynomial multiplication");
	int ret = 0;
	Polynomial *a = polynomial_from_string(lhs);
	Polynomial *b = polynomial_from_string(rhs);
	Polynomial *r = polynomial_multiply(a,b);
	polynomial_to_string(r,g_strbuf,G_STRBUF_LEN,precision);

	if(strcmp(expected,g_strbuf)!=0) {
		printf("FAIL: Expected that multiplying %s with %s would give %s, got %s\r\n",rhs,lhs,expected,g_strbuf);
		ret = 1;
	}

   #ifdef DEBUG
	polynomial_print(a);
	polynomial_print(b);
	polynomial_print(r);
	#endif

	polynomial_free(a);
	polynomial_free(b);
	polynomial_free(r);

	return ret;
}

int test_polynomial_multiplication() {
	int ret = 0;
	ret += check_polynomial_multiplication("3x^2+2x+1","x+1","3.00x^3.00+5.00x^2.00+3.00x+1.00",2);
	ret += check_polynomial_multiplication("5x^2+2x^3+x","3x^4+x^3+2x^2+1","6.00x^7.00+17.00x^6.00+12.00x^5.00+11.00x^4.00+4.00x^3.00+5.00x^2.00+x",2);
	ret += check_polynomial_multiplication("5.5x^2.2+x","3.3x^4.4","18.15x^6.60+3.30x^5.40",2);
	ret += check_polynomial_multiplication("x","x","x^2",0);
	ret += check_polynomial_multiplication("x+x","x+x","4x^2",0);
	ret += check_polynomial_multiplication("x","0","0",0);
	ret += check_polynomial_multiplication("x","1","x",0);
	ret += check_polynomial_multiplication("0","0","0",0);
	return ret;
}

int check_polynomial_parse(char *input, char *expected, int precision) {
	int ret = 0;
	Polynomial *p = polynomial_from_string(input);
	polynomial_to_string(p,g_strbuf,G_STRBUF_LEN,precision);
	if(strncmp(expected,g_strbuf,G_STRBUF_LEN)!=0) {
		printf("FAIL: Expected that parsing \"%s\" would give \"%s\", got \"%s\"\r\n",input,expected,g_strbuf);
		ret = 1;
	}
	polynomial_free(p);
	return ret;
}

int test_polynomial_parse() {
	int ret = 0;
	ret += check_polynomial_parse("2x^2+3","2.00x^2.00+3.00",2);
	ret += check_polynomial_parse("32433.033x^2.2002+2x+3.1452","32433.0330x^2.2002+2.0000x+3.1452",4);
   ret += check_polynomial_parse("x^32.225+23.32x^2.02+233","x^32.225+23.320x^2.020+233.000",3);	
   ret += check_polynomial_parse("x^32.225+23.32x^2.02+233","x^32.23+23.32x^2.02+233.00",2);
   ret += check_polynomial_parse("x^32.225+23.32x^2.02+233","x^32.2+23.3x^2.0+233.0",1);
	ret += check_polynomial_parse("x^3-x^2-x-2","x^3.00-x^2.00-x-2.00",2);
	ret += check_polynomial_parse("-2.2x^-3.3-4.4x^-5.5-6.6x-7.7","-6.6x-7.7-2.2x^-3.3-4.4x^-5.5",1);
	ret += check_polynomial_parse("x+x+x+x+x","5x",0);
	ret += check_polynomial_parse("x+x+x+x+x-x-x-x-x","x",0);
	ret += check_polynomial_parse("x+x+x+x+x-x-x-x-x-x","0",0);
	ret += check_polynomial_parse("x^3+x^3-x^3","x^3",0);
	ret += check_polynomial_parse("x","x",0);
	ret += check_polynomial_parse("0","0",0);
	return ret;
}

int test_programmatic_construction() {
	char str_tmp[128];
	Polynomial *p = NULL;
	Component *c = NULL;

	p = polynomial_from_string("2x^2");
	c = component_new_params(2,2);
	polynomial_add_component(p,c);

	polynomial_to_string(p,str_tmp,128,2);
	char *expected_result_a = "4.00x^2.00";

	polynomial_free(p);
	component_free(c);

 	if(strcmp(str_tmp,expected_result_a)!=0) {
		printf("Expected %s, got %s\r\n",expected_result_a,str_tmp);
		return 1;
	}

	// 3x^2 + 2x + 3
	p = polynomial_new();
	c = component_new();
	c->coefficient = 3;
	c->exponent = 2;
	polynomial_add_component(p,c);
	c->coefficient = 2;
	c->exponent = 1;
	polynomial_add_component(p,c);
	c->coefficient = 3;
	c->exponent = 0;
	polynomial_add_component(p,c);

	char *expected_result_b = "3.00x^2.00+2.00x+3.00";
	polynomial_to_string(p,str_tmp,128,2);

	polynomial_free(p);
	component_free(c);

 	if(strcmp(str_tmp,expected_result_b)!=0) {
		printf("Expected %s, got %s\r\n",expected_result_b,str_tmp);
		return 1;
	}


	// try adding zero
	p = polynomial_from_string("0");
	polynomial_add_component_nocopy(p,component_new_params(0,0));
	polynomial_add_component_nocopy(p,component_new_params(0,0));
	char *expected_result_c = "0.00";
	polynomial_to_string(p,str_tmp,128,2);
	polynomial_free(p);

 	if(strcmp(str_tmp,expected_result_c)!=0) {
		printf("Expected %s, got %s\r\n",expected_result_c,str_tmp);
		return 1;
	}

	return 0;
}

int check_polynomial_subtraction(char *lhs, char *rhs, char *expected, int precision) {
	DBG("Polynomial subtraction");
	int ret = 0;
	Polynomial *a = polynomial_from_string(lhs);
	Polynomial *b = polynomial_from_string(rhs);
	Polynomial *r = polynomial_subtract(a,b);
	polynomial_to_string(r,g_strbuf,G_STRBUF_LEN,precision);

	if(strcmp(expected,g_strbuf)!=0) {
		printf("FAIL: Expected that subtracting %s from %s would give %s, got %s\r\n",rhs,lhs,expected,g_strbuf);
		ret = 1;
	}

   #ifdef DEBUG
	polynomial_print(a);
	polynomial_print(b);
	polynomial_print(r);
	#endif

	polynomial_free(a);
	polynomial_free(b);
	polynomial_free(r);

	return ret;
}

int test_polynomial_subtraction() {
	int ret = 0;
	ret += check_polynomial_subtraction("x^2+5x-6","x-1","x^2+4x-5",0);
	ret += check_polynomial_subtraction("6x^2-6","6x^2-6","0.00",2);
	ret += check_polynomial_subtraction("0","6x^2-6","-6x^2+6",0);
	ret += check_polynomial_subtraction("1","0","1",0);
	ret += check_polynomial_subtraction("0","0","0",0);
	return ret;
}

int test_polynomial_memory() {
	Polynomial *p = NULL;

	// remove 
	p = polynomial_from_string("3x^2+2x+1");
	for(int i=0; i<10; i++) {
		polynomial_remove_component_at_index(p,0);
	}
	polynomial_free(p);

	char str_a[128];
	for(int a=0; a<10; a++) {
		for(int b=0; b<10; b++) {
			for(int c=0; c<10; c++) {
				sprintf(str_a,"%dx^2+%dx+%d",a,b,c);
				p = polynomial_from_string(str_a);
				polynomial_free(p);
			}
		}
	}
	return 0;
}

int check_polynomial_addition(char *lhs, char *rhs, char *expected, int precision) {
	int ret = 0;
	DBG("Polynomial addition");
	Polynomial *a = polynomial_from_string(lhs);
	Polynomial *b = polynomial_from_string(rhs);
	Polynomial *r = polynomial_add(a,b);
	polynomial_to_string(r,g_strbuf,G_STRBUF_LEN,precision);

	if(strcmp(expected,g_strbuf)!=0) {
		printf("FAIL: Expected result of adding \"%s\" to \"%s\" is \"%s\", but got: \"%s\"\r\n",lhs,rhs,expected,g_strbuf);
		ret = 1;
	}

   #ifdef DEBUG
	polynomial_print(a);
	polynomial_print(b);
	polynomial_print(r);
	#endif

	polynomial_free(a);
	polynomial_free(b);
	polynomial_free(r);
	return ret;
}

int test_polynomial_addition() {
	int ret = 0;
	ret += check_polynomial_addition("x^2+5x+6","x-1","x^2+6x+5",0);
	ret += check_polynomial_addition("x^2.3+5.6x+6.6","x-1.1","x^2.3+6.6x+5.5",1);
	ret += check_polynomial_addition("x^2","x^-2","x^2+x^-2",0);
	ret += check_polynomial_addition("x^-2","x^-2","2x^-2",0);
	ret += check_polynomial_addition("3.14x^-2.22-333","93.3x^-2333+333","3.140x^-2.220+93.300x^-2333.000",3);

	return ret;
}

int check_polynomial_division(char *numerator, char *divisor, char *result, int precision, int extra_division_iterations) {
	int ret = 0;
	Polynomial *a = polynomial_from_string(numerator);
	Polynomial *b = polynomial_from_string(divisor);

   #ifdef DEBUG
	polynomial_print(a);
	polynomial_print(b);
	#endif

	Polynomial *r = polynomial_divide(a,b,extra_division_iterations);
	polynomial_to_string(r,g_strbuf,G_STRBUF_LEN,precision);

   #ifdef DEBUG
	polynomial_print(r);
	#endif

	if(strcmp(result,g_strbuf)!=0) {
		printf("FAIL: Expected result of dividing \"%s\" by \"%s\" is \"%s\", but got: \"%s\"\r\n",numerator,divisor,result,g_strbuf);
		ret = 1;
	}

	polynomial_free(a);
	polynomial_free(b);
	polynomial_free(r);
	return ret;
}

int test_polynomial_division() {
	int ret = 0;
	ret += check_polynomial_division("x^2+5x-6","x-1","x+6.00",2,0);
	ret += check_polynomial_division("x^2+5x-6","x-1","x+6.00",2,10);
	ret += check_polynomial_division("x^2+5x-6","x-1.5","x+6.50+3.75x^-1.00",2,0);
	ret += check_polynomial_division("x^2+5x-6","x-1.5","x+6.5000+3.7500x^-1.0000+5.6250x^-2.0000+8.4375x^-3.0000",4,2);
	return ret;
}

typedef int (*test_function)(void);

test_function tests[] = {
	test_polynomial_memory,
	test_programmatic_construction,
	test_out_of_order_addition_of_components,
	test_polynomial_addition,
	test_polynomial_subtraction,
	test_polynomial_multiplication,
	test_polynomial_division,
	NULL
};

char* test_names[] = {
	"Allocation and deallocation of memory",
	"Programmatic addition of components to new polynomial",
	"Out of order addition of components to new polynomial",
	"Addition of polynomials",
	"Subtraction of polynomials",
	"Multiplication of polynomials",
	"Division of polynomials"
};

int main(int argc, char **argv) {
	//return test_polynomial_division();
	//return test_polynomial_parse();
	//return test_programmatic_construction();
	//return test_out_of_order_addition_of_components();
	//return test_polynomial_addition();
	//return test_polynomial_subtraction();
	//return test_polynomial_multiplication();

	int num_tests = 0;
	while(tests[++num_tests]);
	printf("Number of tests: %d\r\n",num_tests);

	for(int i=0; i<num_tests; i++) {
		printf("Test %d (%s) ... ",i,test_names[i]);
		if(tests[i]()==0) {
			printf("PASSED\r\n");
		} else {
			printf("FAILED\r\n");
		}
	}
	return 0;
}
