#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "polynomial.h"
#include "math.h"

/*
typedef struct {
	double coefficient;
	double exponent;
} Component;

typedef struct {
	Component **components;
	int num_components;
	int order;
} Polynomial;
*/

//#define DEBUG

//#define DEBUG_MULTIPLY
//#define DEBUG_PARSER
//#define DEBUG_COMPONENT
//#define DEBUG_DIVIDE
//#define DEBUG_SUBTRACT
//
#define G_DBG_STRBUF_LEN 10240
char g_dbg_strbuf[G_DBG_STRBUF_LEN];

#ifdef DEBUG
	#define DBG(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG(...) {};
	#define DBGX(...) {};
#endif

#ifdef DEBUG_PARSER
	#define DBG_PARSER(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_PARSER(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_PARSER(...) {};
	#define DBGX_PARSER(...) {};
#endif

#ifdef DEBUG_MULTIPLY
	#define DBG_MULTIPLY(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_MULTIPLY(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_MULTIPLY(...) {};
	#define DBGX_MULTIPLY(...) {};
#endif

#ifdef DEBUG_DIVIDE
	#define DBG_DIVIDE(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_DIVIDE(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_DIVIDE(...) {};
	#define DBGX_DIVIDE(...) {};
#endif

#ifdef DEBUG_SUBTRACT
	#define DBG_SUBTRACT(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_SUBTRACT(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_SUBTRACT(...) {};
	#define DBGX_SUBTRACT(...) {};
#endif

#ifdef DEBUG_ADD
	#define DBG_ADD(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_ADD(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_ADD(...) {};
	#define DBGX_ADD(...) {};
#endif

#ifdef DEBUG_COMPONENT
	#define DBG_COMPONENT(...) fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n")
	#define DBGX_COMPONENT(...) fprintf(stderr,__VA_ARGS__);
#else
	#define DBG_COMPONENT(...) {};
	#define DBGX_COMPONENT(...) {};
#endif

int polynomial_aux_is_letter(char c) {
	return c=='x';
}

int polynomial_aux_is_number(char c) {
	return (48<=c && c<=57) || c=='.';
}

int polynomial_aux_is_symbol(char c) {
	return c=='x'||c=='+'||c=='-'||c=='^'||c==0x00;
}

/// Construct a new polynomial with no components.
Polynomial* polynomial_new() {
	Polynomial *p = (Polynomial*)malloc(sizeof(Polynomial));
	p->components = NULL;
	p->num_components = 0;
	p->order = 0;
	return p;
}

/// Free the memory associated with an existing polynomial.
/**
 * @param p A pointer to the polynomial to free.
 */
void polynomial_free(Polynomial *p) {
	for(int i=0; i<p->num_components; i++) {
		component_free(p->components[i]);
		p->components[i] = NULL;
	}
	
	free(p->components);
	free(p);
}

/// Add a component to a polynomial, don't copy the component.
/**
 * Add the component c to the polynomial p, by using the pointer
 * directly, rather than taking a copy of the component.
 *
 * @param p The polynomial to operate on.
 * @param c The component to add to the polynomial.
 */
void polynomial_add_component_nocopy(Polynomial *p, Component *c) {
	polynomial_add_component_control_copy(p,c,0);
}

/// Add a component to a polynomial, copy the component.
/**
 * Add the component c to the polynomial p, by copying the component c.
 *
 * @param p The polynomial to operate on.
 * @param c The component to add to the polynomial.
 */
void polynomial_add_component(Polynomial *p, Component *c) {
	polynomial_add_component_control_copy(p,c,1);
}

/// Copy the coefficient and exponent of component src to component dst, store in dst.
void component_copy_values(Component *src, Component *dst) {
	dst->coefficient = src->coefficient;
	dst->exponent = src->exponent;
}

/// Add the coefficients and exponents of src and dst, store in dst.
void component_add_values(Component *src, Component *dst) {
	dst->coefficient += src->coefficient;
	dst->exponent += src->exponent;
}

/// Return the index of where the component should be in an ascending exponent writeout of the polynomial.
int polynomial_find_component_insert_position(Polynomial *p, Component *c) {
	int i = 0;
	for(i=0; i<p->num_components; i++) {
		if(p->components[i]->exponent<c->exponent) {
			return i;
		}
	}
	return i;
}

/// Return the index of the component which matches the provided exponent.
int polynomial_find_component_by_exponent(Polynomial *p, double exponent) {
	for(int i=0; i<p->num_components; i++) {
		if(p->components[i]->exponent==exponent) {
			return i;
		}
	}
	return -1;
}

/// Remove the component at the specified index cindex from the polynomial p, reallocate memory accordingly.
void polynomial_remove_component_at_index(Polynomial *p, int cindex) {
	DBG_COMPONENT("Removing component at index %d from polynomial at address %x",cindex,p);
	if(p->num_components==0) {
		DBG_COMPONENT("Polynomial has no components, returning");
		return;
	}

	if(cindex>=p->num_components) {
		DBG_COMPONENT("Index of component is greater than number of components, returning");
		return;
	}

	// free the component
	DBG_COMPONENT("Freeing the component at the specified index");
	component_free(p->components[cindex]);
	p->components[cindex] = NULL;

	// move everything above it down
	for(int i=cindex; i<p->num_components-1; i++) {
		DBG_COMPONENT("Moving component at position %d to %d",i+1,i);
		p->components[i] = p->components[i+1];
	}

	DBG_COMPONENT("Changing number of components from %d to %d",p->num_components,p->num_components -1);
	p->num_components -= 1;

	// realloc
	Component **replacement = (Component**)realloc(p->components,p->num_components*sizeof(Component*));

	if(replacement==NULL&&p->num_components!=0) {
		fprintf(stderr,"Error reallocating memory for polynomial components");
		return;
	}

	DBG_COMPONENT("Replacing p->components (%x) with %x",p->components,replacement);

	p->components = replacement;
}

/// Add the component c to the polynomial p, use the component directly, or copy it depending on the bool copy_component.
void polynomial_add_component_control_copy(Polynomial *p, Component *c, int copy_component) {
	#ifdef DEBUG_COMPONENT
	char strA[64];
	char strB[64];
	component_to_string(c,(char*)strA,64,4);
	polynomial_to_string(p,(char*)strB,64,4);
	DBG_COMPONENT("Adding component %s to polynomial %s",(char*)strA,(char*)strB);
	#endif

	// if the component has a zero coefficient, normalize it
	if(c->coefficient==0) {
		c->exponent = 0;
		// only allow a zero coefficient to represent zero
		if(p->num_components>0) {
			// if a component was to be added without copying but wasn't added, free the memory
			if(!copy_component) {
				component_free(c);
			}
			return;
		}
	}

	// determine if component with same exponent already exists
	int cindex = polynomial_find_component_by_exponent(p,c->exponent);
	if(cindex!=-1) {
		DBG_COMPONENT("Found component at index %d with the same exponent",cindex);
		// polynomial already has a component with this exponent
		// add the component to this one
		p->components[cindex]->coefficient += c->coefficient;

		// if the coefficient is now 0, the component should be deleted
		// unless it's the only component in which case we need something to
		// represent the concept of zero
		if(p->components[cindex]->coefficient==0) {
			if(p->num_components>1) {
				DBG_COMPONENT("Cofficient is zero, removing this component");
				polynomial_remove_component_at_index(p,cindex);
			} else {
				// normalise
				p->components[cindex]->exponent = 0;
			}
		}

		// if a component was to be added without copying but wasn't added, free the memory
		if(!copy_component) {
			component_free(c);
		}

		return;
	}

	// find at which position to insert the component
	int iindex = polynomial_find_component_insert_position(p,c);
	DBG_COMPONENT("Inserting component at position %d",iindex);

	// increment the number of components and make space for the component
	p->num_components++;
	Component **tc = NULL;
	tc = (Component**)realloc(p->components,p->num_components*sizeof(Component*));
	if(!tc) {
		DBG("TC was null");
	}
	p->components = tc;

	// move all the proceeding components up
	for(int i=(p->num_components-1); i>iindex; i--) {
		p->components[i] = p->components[i-1];
	}
	
	// insert the component in the desired place
	if(copy_component) {
		p->components[iindex] = component_copy(c);
	} else {
		p->components[iindex] = c;
	}

	if(c->exponent>p->order) {
		p->order = c->exponent;
	}

	#ifdef DEBUG_COMPONENT
	polynomial_to_string(p,(char*)strB,64,2);
	DBG_COMPONENT("Polynomial after addition: %s",strB);
	polynomial_print(p);
	#endif
}

/// Print the polynomial p to stdout
void polynomial_print(Polynomial *p) {
	if(p==NULL) {
		printf("Polynomial is NULL, dummy!\r\n");
		return;
	}
	printf("polynomial has %d components:\r\n",p->num_components);
	for(int i=0; i<p->num_components; i++) {
		printf("%d   ",i);
		component_print(p->components[i]);
		printf("\r\n");
	}

	for(int i=0; i<p->num_components; i++) {
		if(i!=0&&p->components[i]->coefficient>0) {
			printf(" + ");
		}
		component_print(p->components[i]);
	}
	printf("\r\n");
}

/// Parse a string representation of a polynomial and return the polynomial.
/**
 * The string must be in the format:
 *
 * [Ax^B (+|-) ]+
 *
 * Where A is a flaoting point number or empty.
 * And B is a floating point number or empty.
 *
 * For example:
 *
 * 2x^2.232 + 3x^343.22 + 6x
 *
 * When the polynomial is parsed, the components will be ordered in ascending
 * order of exponent.
 */
Polynomial* polynomial_from_string(char *s) {
	DBG_PARSER("Parsing \"%s\"",s);

	char *p = s;
	typedef enum {
		SIGN,
		COEFFICIENT,
		X_MARKS_THE_SPOT,
		EXPONENT_MARKER,
		EXPONENT_SIGN,
		EXPONENT,
		STORE_COMPONENT,
		FINISHED
	} ParserState;

	ParserState state = SIGN;
	int count = 30;

	int negative_component = 0;
	double coefficient = 0, exponent = 0;

	// for parsing numbers
	double multiplier = 10;
	int fraction_part = 0;
	int exponent_sign_multiplier = 1;

	Polynomial *poly = polynomial_new();
	DBGX_PARSER("looking for sign ... ");

	while(state!=FINISHED) {
		switch(state) {
			case SIGN:
				if(*p==0x00) {
					DBG_PARSER("end of stream");
					state = FINISHED;
					continue;
				}

				// look for sign
				if(*p!='-'&&*p!='+') {
					if(polynomial_aux_is_number(*p)) {
						DBG_PARSER("Positive sign");
						DBG_PARSER("looking for coefficient");
						state = COEFFICIENT;
						continue;
					}
					if(*p=='x') {
						DBG_PARSER("Positive sign");
						coefficient = 1;
						state = X_MARKS_THE_SPOT;
						DBGX_PARSER("looking for x ... ");
						continue;
					}
				}
				if(*p=='-') {
					DBG_PARSER("Negative sign");
					negative_component = 1;
					p++;
					DBG_PARSER("looking for coefficient");
					state = COEFFICIENT;
					continue;
				}
				if(*p!='+') {
					fprintf(stderr,"Error parsing polynomial. Expected '+', got '%c'\r\n",*p);
					state = FINISHED;
					break;
				}
				p++;

			break;

			case COEFFICIENT:
				if(*p==0x00) {
					state = STORE_COMPONENT;
					continue;
					/*
					if(coefficient!=0) {
						state = STORE_COMPONENT;
						continue;
					}
					*/
					DBG_PARSER("Error parsing polynomial, expected coefficient, got end of stream");
					state = FINISHED;
					continue;
				}

				// look for coefficient
				if(polynomial_aux_is_number(*p)) {
					if(*p=='.') {
						fraction_part = 1;
						multiplier = 0.1;
						p++;
						continue;
					}
					DBG_PARSER("digit %c",*p);
					if(!fraction_part) {
						coefficient *= 10;
						coefficient += (*p-'0');
					} else {
						coefficient += (*p-'0')*multiplier;
						multiplier /= 10;
					}
					p++;
					continue;
				}
				DBG_PARSER("not number: %c",*p);
				DBG_PARSER("end of coefficient: %lf",coefficient);
				multiplier = 10;
				fraction_part = 0;
				DBGX_PARSER("looking for x ... ");
				state = X_MARKS_THE_SPOT;
			break;

			case X_MARKS_THE_SPOT:
				if(*p=='x') {
					DBG_PARSER("found x"); 
					if(coefficient==0) {
						coefficient = 1;
					}
					DBGX_PARSER("looking for exponent marker ... ");
					state = EXPONENT_MARKER;
					p++;
					continue;
				}

				if(*p=='+'||*p=='-') {
					state = STORE_COMPONENT;
					continue;
				}

				// must be a number, which would have already been parsed. This is an error
				DBG_PARSER("Error parsing polynomial. Expected 'x', got %c",*p);
				count = 0;
			break;

			case EXPONENT_MARKER:
				if(*p=='^') {
					DBG_PARSER("found exponent marker");
					p++;
				   DBGX_PARSER("looking for exponent ... ");
					state = EXPONENT_SIGN;
					continue;
				}
				// there is no exponent, but there is an x, so the exponent is 1
				exponent = 1;
				DBG_PARSER("no exponent marker");
				state = STORE_COMPONENT;
			break;

			// check if the exponent is negative
			case EXPONENT_SIGN:
				DBG_PARSER("Checking exponent sign");
				if(*p=='-') {
					DBG_PARSER("Exponent is negative");
					exponent_sign_multiplier = -1;
					p++;
				} else {
					DBG_PARSER("Exponent is positive");
					exponent_sign_multiplier = 1;
				}
				state = EXPONENT;
			break;

			case EXPONENT:
				if(polynomial_aux_is_number(*p)) {
					if(*p=='.') {
						multiplier = 0.1;
						fraction_part = 1;
						p++;
						continue;
					}

					DBG_PARSER("digit %c",*p);
					if(!fraction_part) {
						exponent *= 10;
						exponent += (*p-'0');
					} else {
						exponent += (*p-'0')*multiplier;
						multiplier /= 10;
					}
					p++;
					continue;
				}
				exponent *= exponent_sign_multiplier;
				DBG_PARSER("end of exponent: %lf",exponent);

				// end of exponent
			   state = STORE_COMPONENT;
			break;

			case STORE_COMPONENT:
				DBG_PARSER("store component: %.1lfx^%.1lf",coefficient,exponent);
				if(negative_component) {
					coefficient *= -1;
				}

				polynomial_add_component_nocopy(poly,component_new_params(coefficient,exponent));
				if(coefficient>poly->order) {
					poly->order = coefficient;
				}

				// reset helper variables
				coefficient = 0;
				exponent = 0;
				negative_component = 0;
				multiplier = 10;
				fraction_part = 0;
				// look for next component
				DBGX_PARSER("looking for sign ... ");
				state = SIGN;
			break;

			default:
				DBG_PARSER("default");
			break;
		}
	}
	return poly;
}

/// Construct a new empty component.
Component* component_new() {
	return (Component*)malloc(sizeof(Component));
}

/// Construct a new component having the specified coefficient and exponent.
Component* component_new_params(double coefficient, double exponent) {
	Component *c = (Component*)malloc(sizeof(Component));
	c->coefficient = coefficient;
	c->exponent = exponent;
	return c;
}

/// Return a copy of the component c.
Component* component_copy(Component *c) {
	return component_new_params(c->coefficient,c->exponent);
}

/// Free the memory associated with component c.
void component_free(Component *c) {
	free(c);
}

/// Convert polynomial p to a string and store in dst.
/**
 * @param p The polynomial to convert
 * @param dst Pointer to destination char buffer
 * @param dst_len The length of the destination char buffer
 * @param precision The number of digits after the decimal point that
 * numbers in the conversion should have.
 */
int polynomial_to_string(Polynomial *p, char *dst, int dst_len, int precision) {
	if(dst_len==0) {
		return 0;
	}
	if(p->num_components==0) {
		sprintf(dst,"<EMPTY>");
		return strlen(dst);
	}
	// detect the first component to print it slightly differently
	bzero(dst,dst_len);
	// use the dst argument to inform the parser that it is the first arg
	dst[0] = 'f';
	char *pDst = dst;
	int len = dst_len, chars_used = 0;
	for(int i=0; i<p->num_components; i++) {
		chars_used = component_to_string(p->components[i],pDst,len,precision);
		len -= chars_used;
		pDst += chars_used;
	}
	return dst_len - len;
}

/// Convert component c of a polynomial to a string and store in dst.
/**
 * @param c The component to convert
 * @param dst Pointer to destination char buffer
 * @param dst_len The length of the destination char buffer
 * @param precision The number of digits after the decimal point that
 * numbers in the conversion should have.
 */
int component_to_string(Component *c, char *dst, int dst_len, int precision) {
   #define TMP_LEN 128
	char tmp[TMP_LEN];
	char *pTmp = tmp;
	char str_format[32];

	if(c->exponent==0) {
		if(c->coefficient<0||dst[0]=='f') {
			sprintf(str_format,"%%.%df",precision);
		} else {
			sprintf(str_format,"+%%.%df",precision);
		}
		sprintf(tmp,str_format,c->coefficient);
		strncpy(dst,tmp,dst_len-1);
		dst[dst_len-1] = 0x00;
		return strlen(tmp);
	}

	if(c->coefficient==1||c->coefficient==-1) {
		if(c->coefficient==-1) {
			sprintf(pTmp,"-x");
			pTmp += 2;
		} else if(dst[0]=='f') {
			sprintf(pTmp,"x");
			pTmp += 1;
		} else {
			sprintf(pTmp,"+x");
			pTmp += 2;
		}
		if(c->exponent!=1) {
			sprintf(str_format,"^%%.%df",precision);
			sprintf(pTmp,str_format,c->exponent);
		}
		strncpy(dst,tmp,dst_len-1);
		dst[dst_len-1] = 0x00;
		return strlen(tmp);
	}

	if(c->exponent==1) {
		if(c->coefficient<0||dst[0]=='f') {
			sprintf(str_format,"%%.%dfx",precision);
		} else {
			sprintf(str_format,"+%%.%dfx",precision);
		}
		sprintf(pTmp,str_format,c->coefficient);
	} else {
		if(c->coefficient<0||dst[0]=='f') {
			sprintf(str_format,"%%.%dfx^%%.%df",precision,precision);
		} else {
			sprintf(str_format,"+%%.%dfx^%%.%df",precision,precision);
		}
		sprintf(pTmp,str_format,c->coefficient,c->exponent);
	}
	strncpy(dst,tmp,dst_len-1);
	dst[dst_len-1] = 0x00;
	return strlen(tmp);
}

/// Print the component c to stdout
void component_print(Component *c) {
	if(c->exponent==0) {
		printf("%lf",c->coefficient);
		return;
	}

	if(c->coefficient==1||c->coefficient==-1) {
		if(c->coefficient==-1) {
			printf("-x");
		} else {
			printf(" x");
		}
		if(c->exponent==1) {
			return;
		} else {
			printf("^%lf",c->exponent);
			return;
		}
	}

	if(c->exponent==1) {
		printf("%lfx",c->coefficient);
	} else {
		printf("%lfx^%lf",c->coefficient,c->exponent);
	}
}

/// Multiply the polynomials a and b, and return a new polynomial containing the result.
Polynomial* polynomial_multiply(Polynomial *a, Polynomial *b) {
	#ifdef DEBUG_MULTIPLY
	char tmpA[64],tmpB[64];
	polynomial_to_string(a,tmpA,64,4);
	polynomial_to_string(b,tmpB,64,4);
	DBG_MULTIPLY("Multiplying: %s by %s",tmpA,tmpB);
	#endif

	Polynomial *r = polynomial_new();
	// start with the factor with the highest order
	Polynomial *f1 = a, *f2 = b;
	if(b->order>a->order) {
		f1 = b;
		f2 = a;
	}

	Component *c = component_new();
	Component *f1c = NULL, *f2c = NULL;
	// iterate over components of first polynomial
	for(int i=0; i<f1->num_components; i++) {
		f1c = f1->components[i];
		// multiply by the components of the second polynomial
		for(int j=0; j<f2->num_components; j++) {
			DBG_MULTIPLY("1st component %d: %lf %lf",i,f1c->coefficient,f1c->exponent);
			f2c = f2->components[j];
		   DBG_MULTIPLY("2nd component %d: %lf %lf",j,f2c->coefficient,f2c->exponent);
			c->coefficient = f1c->coefficient*f2c->coefficient;
			c->exponent = f1c->exponent+f2c->exponent;
		   DBG_MULTIPLY("Result %d: %lf %lf",j,c->coefficient,c->exponent);
			#ifdef DEBUG_MULTIPLY
			component_to_string(c,tmpA,64,2);
			DBG_MULTIPLY("multiplication sub-result %s",tmpA);
			#endif
			polynomial_add_component(r,c);
		}
	}
	component_free(c);
	return r;
}

/// Switch the signs of each component in the polynomial a and return a new polynomial containing the result.
Polynomial* polynomial_invert(Polynomial *a) {
	Polynomial *p = polynomial_copy(a);
	for(int i=0; i<p->num_components; i++) {
		p->components[i]->coefficient *= -1;
	}
	return p;
}

/// Return a copy of the polynomial p.
Polynomial* polynomial_copy(Polynomial *p) {
	Polynomial *r = polynomial_new();
	for(int i=0; i<p->num_components; i++) {
		polynomial_add_component(r,p->components[i]);
	}

	return r;
}

/// Subtract polynomial b from the polynomial a and return the result as a new polynomial.
Polynomial* polynomial_subtract(Polynomial *a, Polynomial *b) {
	// lazy method, invert b and add it to a
	Polynomial *b_inv = polynomial_invert(b);
	Polynomial *p = polynomial_add(a,b_inv);

   #ifdef DEBUG_SUBTRACT
	polynomial_print(b);
	polynomial_print(b_inv);
	#endif

	polynomial_free(b_inv);
	return p;
}

/// Add the polynomial a to the polynomial b and return the result as a new polynomial.
Polynomial* polynomial_add(Polynomial *a, Polynomial *b) {
	Polynomial *r = polynomial_new();
	for(int i=0; i<a->num_components; i++) {
		polynomial_add_component(r,a->components[i]);	
	}
	for(int i=0; i<b->num_components; i++) {
		polynomial_add_component(r,b->components[i]);	
	}
	return r;
}

/// Evaluate a polynomial for each point in an input vector and store in an output vector.
/**
 * @param p The polynomial to evaluate.
 * @param input The vector of x values to put into p.
 * @param output Vector to store the evaluation outputs.
 * @param input_len The length of the input vector.
 */
void polynomial_evaluate_vector(Polynomial *p, double *input, double *output, int input_len) {
	for(int i=0; i<input_len; i++) {
		output[i] = polynomial_evaluate_point(p,input[i]);
	}
}

/// Evaluate a single point input of the polynomial p.
double polynomial_evaluate_point(Polynomial *p, double input) {
	double result = 0;
	for(int i=0; i<p->num_components; i++) {
		result += ( p->components[i]->coefficient * pow(input,p->components[i]->exponent) );
	}
	return result;
}

/// Remove any extra zero components from a polynomial, leaves only one.
void polynomial_remove_zero_components(Polynomial *p) {
	// only remove zero component when there are other components
	if(p->num_components==1) {
		return;
	}

	int removal_index = 0;
	while(p->num_components>1) {
		for(removal_index=0; removal_index<p->num_components; removal_index++) {
			if(p->components[removal_index]->coefficient==0 &&
				p->components[removal_index]->exponent==0) {
				break;
			}
		}
		// no component was found to remove so we are done
		if(removal_index==p->num_components) {
			break;
		}

		// otherwise remove the component and continue
		polynomial_remove_component_at_index(p,removal_index);
	}
}

/// Divide the numerator polynomial n with the divisor d and return a new polynomial.
/**
 * @param n The numerator
 * @param d The divisor
 * @param extra_iterations By default, the number of iterations performed 
 * equals the number of components in the divisor, since this will produce
 * the result sans remainder.
 *
 * By setting extra_iterations to a number more than zero, the division is 
 * continued to reduce the remainder. By setting this number to a high value,
 * the effect is that you get a kind of "fractional" polynomial, that more
 * accurately represents the result of the division.
 */
Polynomial* polynomial_divide(Polynomial *n, Polynomial *d, int extra_iterations) {
	// locals
	double n_coefficient = 0, d_coefficient = 0, f_coefficient = 0;
	double n_exponent = 0, d_exponent = 0, f_exponent = 0;

	Polynomial *numerator = polynomial_copy(n);
	Polynomial *factor = NULL, *partial_result = NULL, *new_numerator = NULL;
	Component *c = NULL;
	#ifdef DEBUG_DIVIDE
	int dbg_precision = 4;
	polynomial_to_string(n,g_dbg_strbuf,G_DBG_STRBUF_LEN,2);
	DBGX_DIVIDE("dividing %s",g_dbg_strbuf);
	polynomial_to_string(d,g_dbg_strbuf,G_DBG_STRBUF_LEN,2);
	DBG_DIVIDE(" by %s\r\n",g_dbg_strbuf);
	#endif

	int max_iterations = n->num_components + extra_iterations;

	// result
	Polynomial *result = polynomial_new();

	// division is an iterative process that continues until the remainder is zero
	// if the remainder is not zero, then the process can continue indefinitely in the
	// same way floating point division can, so we need some idea of "precision" which
	// is controlled by the maximum number of iterations
	for(int k=0; k<max_iterations; k++) {
		DBG_DIVIDE("Iteration %d",k);

		// get first component of numerator
		n_coefficient = numerator->components[0]->coefficient;
		n_exponent = numerator->components[0]->exponent;
		DBG_DIVIDE("numerator: %.2lfx^%.2lf",n_coefficient,n_exponent);
		
		// get first component of divisor
		d_coefficient = d->components[0]->coefficient;
		d_exponent = d->components[0]->exponent;
		DBG_DIVIDE("partial divisor: %.2lfx^%.2lf",d_coefficient,d_exponent);

		// determine factor
		f_coefficient = n_coefficient/d_coefficient;
		f_exponent = n_exponent-d_exponent;
		DBG_DIVIDE("factor: %.2lfx^%.2lf",f_coefficient,f_exponent);
		// construct polynomial for it
		factor = polynomial_new();
		c = component_new_params(f_coefficient,f_exponent);
		polynomial_add_component(factor,c);
		polynomial_add_component(result,c);

		// multiply the divisor by the factor
		partial_result = polynomial_multiply(factor,d);
		#ifdef DEBUG_DIVIDE
		polynomial_to_string(partial_result,g_dbg_strbuf,G_DBG_STRBUF_LEN,dbg_precision);
		DBG_DIVIDE("Product of full divisor and factor: %s",g_dbg_strbuf);
		#endif

		// subtract this partial result from the numerator to construct the "new" numerator
		new_numerator = polynomial_subtract(numerator,partial_result);
		polynomial_remove_zero_components(new_numerator);

		#ifdef DEBUG_DIVIDE
		polynomial_to_string(new_numerator,g_dbg_strbuf,G_DBG_STRBUF_LEN,dbg_precision);
		DBG_DIVIDE("New numerator: %s",g_dbg_strbuf);
		#endif

		// free the old numerator and repeat
		polynomial_free(numerator);
		numerator = new_numerator;

		// clean up temporary polynomials
		polynomial_free(factor);
		polynomial_free(partial_result);
		component_free(c);

		if(polynomial_is_zero(numerator)) {
			break;
		}
	}

	#ifdef DEBUG_DIVIDE
	DBG_DIVIDE("Result");
	polynomial_print(result);
	#endif

	return result;
}

/// Is polynomial p zero.
int polynomial_is_zero(Polynomial *p) {
	if(p->num_components!=1) {
		return 0;
	}
	return p->components[0]->coefficient==0 && p->components[0]->exponent==0;
}

/// Returns 1 if components a and b have equal coefficients and exponents, otherwise 0.
int component_is_equal(Component *a, Component *b) {
	return a->coefficient==b->coefficient && a->exponent==b->exponent;
}

/// Returns 1 if polynomials a and b have numerically the same components, otherwise 0.
int polynomial_is_equal(Polynomial *a, Polynomial *b) {
	if(a->num_components!=b->num_components) {
		return 0;
	}

	for(int i=0; i<a->num_components; i++) {
		if(!component_is_equal(a->components[i],b->components[i])) {
			return 0;
		}
	}
	return 1;
}
