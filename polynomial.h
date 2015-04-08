/*-
 * Copyright (c) 2015
 * Ashley Mills
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
