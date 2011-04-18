#include "tools.h"


/*
 * Minimum value in a vector
 */
float minval(float *vec, int n) {
  float min = vec[0];
  int i;
  for (i=1;i<n;i++) {
    if (vec[i]<min) {
      min=vec[i];
    }
  }
  return min;
}

/*
 * Maximum value in a vector
 */
float maxval(float *vec, int n) {
  float max = vec[0];
  int i;
  for (i=1;i<n;i++) {
    if (vec[i]>max) {
      max=vec[i];
    }
  }
  return max;
}

void minmaxval (const float * const vec, const int n, float * min, float * max)
{
	*min = *max = vec[0];

	for (int i = 1; i < n; ++i)
	{
		*min = (vec[i] < *min) ? vec[i] : *min;
		*max = (vec[i] > *max) ? vec[i] : *max;
	}

	return;
}

// TODO: Add checks for errors and return NULL


/*
 *   1D int
 */
int *allocint1(int n1) {
  int *a;
  a = (int *)malloc(n1*sizeof(int));
  return a;
}
void freeint1(int *a) {
  free(a);
}


/*
 *   1D float
 */
float *allocfloat1(int n1) {
  float *a;
  a       = (float *)malloc(n1      *sizeof(float  ));
  return a;
}

void freefloat1(float *a) {
  free(a);
}


/*
 *   2D float
 */
float **allocfloat2(int n1, int n2) {
  long i1;
  float **a;
  a       = (float **)malloc(n1      *sizeof(float *));
  a[0]    = (float  *)malloc(n1*n2   *sizeof(float  ));
  for (i1=1; i1<n1; i1++) {
    a[i1] = a[i1-1] +n2;
  }
  return a;
}

void freefloat2(float **a) {
  free(a[0]);
  free(a);
}

/*
 *   3D float
 */
float ***allocfloat3(int n1, int n2, int n3) {
  long i1, i2;
  float ***a;
  a       = (float ***)malloc(n1      *sizeof(float **));
  a[0]    = (float  **)malloc(n1*n2   *sizeof(float  *));
  a[0][0] = (float   *)malloc(n1*n2*n3*sizeof(float   ));

  for(i1=0; i1<n1; i1++) {
    a[i1] = a[0] +n2*i1;

    for(i2=0; i2<n2; i2++) {
      a[i1][i2] = a[0][0] +n3*(i2 + n2*i1);
    }
  }

  return a;
}


void freefloat3(float ***a) {
  free(a[0][0]);
  free(a[0]);
  free(a);
}

/*
 *   1D float complex
 */
float complex *allocfloatcomplex1(int n1) {
  float complex *a;
  a       = (float complex *)malloc(n1      *sizeof(float complex *));
  return a;
}

void freefloatcomplex1(float complex *a) {
  free(a);
}

/*
 *   2D float complex
 */
float complex **allocfloatcomplex2(int n1, int n2) {
  long i1;
  float complex **a;
  a       = (float complex **)malloc(n1      *sizeof(float complex *));
  a[0]    = (float complex  *)malloc(n1*n2   *sizeof(float complex  ));
  for (i1=1; i1<n1; i1++) {
    a[i1] = a[i1-1] +n2;
  }
  return a;
}

void freefloatcomplex2(float complex **a) {
  free(a[0]);
  free(a);
}

void clearfloatcomplex3 (float complex ***a, int n1, int n2, int n3)
{
  for (long i = 0; i < n1 * n2 * n3; ++i)
    {
      a[0][0][i] = 0.0f + 0*I;
    }
}

/*
 *   3D float complex
 */
float complex ***allocfloatcomplex3(int n1, int n2, int n3) {
  long i1, i2;
  float complex ***a;
  a       = (float complex ***)malloc(n1      *sizeof(float complex **));
  a[0]    = (float complex  **)malloc(n1*n2   *sizeof(float complex  *));
  a[0][0] = (float complex   *)malloc(n1*n2*n3*sizeof(float complex   ));
  for(i1=0; i1<n1; i1++) {
    a[i1] = a[0] +n2*i1;
    for(i2=0; i2<n2; i2++) {
      a[i1][i2] = a[0][0] +n3*(i2+n2*i1);
    }
  }
  return a;
}


void freefloatcomplex3(float complex ***a) {
  free(a[0][0]);
  free(a[0]);
  free(a);
}
