/***************************************************************************
 *   Copyright (C) 2007 by Maxim Ilyasov   *
 *   ilyasov@itwm.fhg.de   *
 ***************************************************************************/
#ifndef FFT_H
#define FFT_H

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __ALTIVEC__
#include <altivec.h>
#include <spu2vmx.h>
#endif

/*!
 \fn int getptz(int N)
 \brief get dimension that passes to the Fast Fourier Transform
 \param N length of the array, that is to fourier transformed
 \warning 0 <= N <= 720720 (it will not be checked in the function)
*/
int getptz(int N);

/*!
 \fn void fft(int isign, int n, float cz[]);
 \brief 1D fast fourier transform complex->complex
 \param isign sign of exponent in fourier transform kernel
 \param n length of the array cz[]
 \param cz[] array of the length 2n float values - complex elements: cz[i] = {real, imag}
 \warning n must be passed to fast fourier transform
*/
void fft(int isign, int n, float cz[]);

/*!
 \fn void sse_fft(int isign, int n, __m128 cz[]);
 \brief 1D fast fourier transform complex->complex with sse technology
 \param isign sign of exponent in fourier transform kernel
 \param n length of the array cz[]
 \param cz[] array of the length 2n __m128 values - complex elements: cz[i] = {real, imag}
 \warning n must be passed to fast fourier transform
*/
#ifdef __SSE__
void sse_fft(int isign, int n, __m128 cz[]);
#endif

#ifdef __ALTIVEC__
void sse_fft(int isign, int n, vector float cz[]);
#endif

#endif
