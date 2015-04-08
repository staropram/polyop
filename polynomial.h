/**
 * (c) Ashley Mills, 2015
 * Licensed under BSD simplified.
 */

#pragma once
typedef struct {
	double coefficient;
	double exponent;
} Component;

typedef struct {
	Component **components;
	int num_components;
	int order;
} Polynomial;

Polynomial* polynomial_new();
Polynomial* polynomial_from_string(char *s);
void polynomial_add_component_control_copy(Polynomial *p, Component *c, int copy_component);
void polynomial_add_component_nocopy(Polynomial *p, Component *c);
void polynomial_add_component(Polynomial *p, Component *c);
void polynomial_print(Polynomial *p);
void polynomial_free(Polynomial *p);
void polynomial_remove_component_at_index(Polynomial *p, int cindex);
void polynomial_remove_zero_components(Polynomial *p);

// auxilliary internal functions
int polynomial_aux_is_letter(char c);
int polynomial_aux_is_number(char c);
int polynomial_aux_is_symbol(char c);

// arithmetic operations
Polynomial* polynomial_add(Polynomial *a, Polynomial *b);
Polynomial* polynomial_subtract(Polynomial *a, Polynomial *b);
Polynomial* polynomial_multiply(Polynomial *a, Polynomial *b);
Polynomial* polynomial_divide(Polynomial *n, Polynomial *d, int max_iterations);

void polynomial_evaluate_vector(Polynomial *p, double *input, double *output, int input_len);
double polynomial_evaluate_point(Polynomial *p, double input);

// comparitors
int polynomial_is_zero(Polynomial *p);
int polynomial_is_equal(Polynomial *a, Polynomial *b);
int component_is_equal(Component *a, Component *b);

int polynomial_to_string(Polynomial *p, char *dst, int dst_len, int precision);
int component_to_string(Component *c, char *dst, int dst_len, int precision);

Component* component_new();
Component* component_new_params(double coefficient, double exponent);
Component* component_copy(Component *c);

Polynomial* polynomial_copy(Polynomial *p);

void component_copy_values(Component *src, Component *dst);
void component_add_values(Component *src, Component *dst);
void component_free(Component *c);
void component_print();

int is_number(char c);
int is_symbol(char c);
int is_letter(char c);
