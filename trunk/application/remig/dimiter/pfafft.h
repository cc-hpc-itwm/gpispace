/*****************************************************************************
PFAFFT - Functions to perform Prime Factor (PFA) FFT's, in place
npfao		return optimal n for complex-to-complex PFA
pfacc		1D PFA complex to complex

          HEADERS

***********************************************************/
#ifndef C_PFAFFT_H
#define C_PFAFFT_H

#include "cwp.h"

int npfao (int nmin, int nmax);
void fftolen_(int* nmin, int* nmax, int* npt);
void pfacc(int isign, int n, complex cz[]);

#endif
