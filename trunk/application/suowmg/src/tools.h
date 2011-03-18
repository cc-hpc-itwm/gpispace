#ifndef STATOIL_TOOL
#define STATOIL_TOOL

#ifdef _cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif
#include </usr/include/complex.h>
#include <string.h>

#define IMIN(a,b) ((a)<(b) ? (a) : (b))
#define IMAX(a,b) ((a)>(b) ? (a) : (b))
#define NINT(x) ((int)((x)>0.0?(x)+0.5:(x)-0.5))

float minval(float *vec, int n);
float maxval(float *vec, int n);

int *allocint1(int n1);
void freeint1(int *a);

float   *allocfloat1(int n1);
void freefloat1(float   *a);
float  **allocfloat2(int n1, int n2);
void freefloat2(float  **a);
float ***allocfloat3(int n1, int n2, int n3);
void freefloat3(float ***a);

float complex *allocfloatcomplex1(int n1);
void freefloatcomplex1(float complex *a);
float complex **allocfloatcomplex2(int n1, int n2);
void freefloatcomplex2(float complex **a);
float complex ***allocfloatcomplex3(int n1, int n2, int n3);
void freefloatcomplex3(float complex ***a);
void clearfloatcomplex3(float complex***, int, int, int);

#endif
