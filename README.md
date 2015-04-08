# polyop
c library for representing and performing mathematical operations upon polynomials

Example:

~~~{.c}
int extra_division_iterations = 0, comparison = 0, precision = 3;
char out_str_a[128], out_str_b[128], out_str_c[128], out_str_d[128];
Polynomial *a=NULL,*b=NULL,*c=NULL,*d=NULL;

a = polynomial_from_string("x^2+5x-6");
b = polynomial_from_string("x-1");
c = polynomial_divide(a,b,extra_division_iterations);
d = polynomial_multiply(c,b);

polynomial_to_string(a,out_str_a,128,precision);
polynomial_to_string(b,out_str_b,128,precision);
polynomial_to_string(c,out_str_c,128,precision);
polynomial_to_string(d,out_str_d,128,precision);

printf("Division of \"%s\" and \"%s\": \"%s\"\r\n",out_str_a,out_str_b,out_str_c);
printf("Comparison of \"%s\" and \"%s\": %d\r\n",out_str_a,out_str_d,polynomial_is_equal(a,d));

polynomial_free(a);
polynomial_free(b);
polynomial_free(c);
polynomial_free(d);
~~~								

Produces:

~~~
Division of "x^2.000+5.000x-6.000" and "x-1.000": "x+6.000"
Comparison of "x^2.000+5.000x-6.000" and "x^2.000+5.000x-6.000": 1
~~~								

