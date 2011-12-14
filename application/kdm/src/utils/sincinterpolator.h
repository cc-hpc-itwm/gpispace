/***************************************************************************
                          sincinterpolator.h  -  description

    Perform sinc interpolation.

                             -------------------
    begin                : Fri Feb 01 2008
    copyright            : (C) 2008 by Norman Ettrich
    email                : merten@itwm.fhg.de
***************************************************************************/

#ifndef SINCINTERPOLATOR_H
#define SINCINTERPOLATOR_H

/**
 *@author Dirk Merten
 */

#include <math.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __ALTIVEC__
#include <altivec.h>
#include <spu2vmx.h>
#endif

#ifdef __SPU__
#include <spu_intrinsics.h>
#endif

//#include <iostream>
#include <algorithm>


#ifdef STANDALONE
#include "utils2.hpp"
#endif

class SincInterpolator {
private:
  int oversamp;
  int width_normal; 
  float *sinc_normal_pre;
  int wt_normal;
  float dt, dt_sub;
  int width_hilbert;
  float *sinc_hilbert_pre;
  int wt_hilbert;
  int width_filter;
  float *sinc_filter_pre_allHz, *sinc_filterhilbert_pre_allHz;
  int wt_filter, nFreqs, n_oneBand;
  float * Freqs, Freqs0, dFreqs, fc_limit;


  int return_fbandIndex(float f) const {
    int iaux = std::min(nFreqs, 
		   std::max(
			    0, (int)( ( f+ dFreqs/100.0 - Freqs0 ) / dFreqs )
			    )
		   );

    return iaux;
  };
  

#if (defined(__ALTIVEC__) || defined(__SPU__))
  vector float dtbin_vec;
  vector float dtbin_inv_vec;
  vector float dtsub_inv_vec;
  vector unsigned int width_max_vec;
  vector unsigned int width_normal_vec;
  vector unsigned int width_hilbert_vec;
  vector signed int oversamp_1_vec;
  vector signed int wt_normal_vec;
#endif

public:
  SincInterpolator(int width_normal_extern=5, float fc_limit_extern=50):sinc_normal_pre(NULL),sinc_hilbert_pre(NULL),sinc_filter_pre_allHz(NULL), sinc_filterhilbert_pre_allHz(NULL), Freqs(NULL){};
  ~SincInterpolator(){terminate();};

  int width_max;
  int width_max_sse;
  
  int init(float, int width_normal_extern=5, float fc_limit_extern=50);
  void terminate();

#ifdef STANDALONE
  float interp_lin(float *, int, float, float);
#endif


/* ################################################################ */
  float interp_sinc(float * S, int nt, float t0, float t, short modus,
		    float fc = -1.0) const {
    int i, i0, iarg, itdt_rest, iFreqs, len, LEN2;
    float tdt, ss, ss2, tdt_rest;
    
    const float eps = 0.01;
    
    if ( ( fc >= fc_limit-eps || fc < 0.0 ) && modus > 1 )
      modus -= 2;
    
    
    
    t -= t0;
    tdt = t/dt;
    i0 = (int)(tdt);
    


    
    if (  modus == 0 ) {/* ordinary, 
			   with 1-0.5 tapering at end of operator */

      len = std::min(i0, nt-1-i0);
      if ( len < 1 )
	return 0.0;
      len = std::min(width_normal, len);
      LEN2 = 2 * len; 

      tdt_rest = t - (float)i0 * dt;
      itdt_rest = std::min(oversamp-1, (int)(tdt_rest / dt_sub));
      
      
      iarg = itdt_rest * wt_normal + (width_normal - len);
      i0 -= len;

      ss = 0.5 * S[i0++] * mysincgetnormalvalue_pre(iarg++);
      
      for ( i=1; i<LEN2; i++ )
	ss += S[i0++] * mysincgetnormalvalue_pre(iarg++);

      ss += 0.5 * S[i0] * mysincgetnormalvalue_pre(iarg);
      
    }
    
    else if (  modus == 1 ) {/* picking in Hilbert transform, 
				with 1-0.66-0.33 tapering at end of operator */

      len = std::min(i0, nt-1-i0);
      if ( len < 2 )
	return 0.0;
      len = std::min(width_hilbert, len);
      LEN2 = 2 * len; 
      
      tdt_rest = t - (float)i0 * dt;
      itdt_rest = std::min(oversamp-1, (int)(tdt_rest / dt_sub));
      
      
      iarg = itdt_rest * wt_hilbert + (width_hilbert - len);
      i0 -= len;
      
      ss2 = ( S[i0++] * mysincgethilbertvalue_pre(iarg++) );
      ss2 += 2.0 * ( S[i0++] * mysincgethilbertvalue_pre(iarg++) );
      
      ss = 0.0;
      for ( i=2; i<LEN2-1; i++ )
	ss += S[i0++] * mysincgethilbertvalue_pre(iarg++);
      
      ss2 += 2.0 * ( S[i0++] * mysincgethilbertvalue_pre(iarg++) );
      ss2 += ( S[i0] * mysincgethilbertvalue_pre(iarg) );
    
      ss += ss2 / 3.0;
    
    }
  
    else if (  modus == 2 ) {/* picking in trace low-pass filtered with fc
      				with 1-0.66-0.33 tapering at end of operator */
      
      len = std::min(i0, nt-1-i0);
      if ( len < 2 )
	return 0.0;
      len = std::min(width_filter, len);
      LEN2 = 2 * len; 

      tdt_rest = t - (float)i0 * dt;
      itdt_rest = std::min(oversamp-1, (int)(tdt_rest / dt_sub));
      
      
      iFreqs = return_fbandIndex(fc) * n_oneBand;
      

      iarg = itdt_rest * wt_filter + iFreqs + (width_filter - len);
      i0 -= len;
      
      ss2 = ( S[i0++] * mysincgetfiltervalue_pre(iarg++) );
      ss2 += 2.0 * ( S[i0++] * mysincgetfiltervalue_pre(iarg++) );
      
      ss = 0.0;
      for ( i=2; i<LEN2-1; i++ )
	ss += S[i0++] * mysincgetfiltervalue_pre(iarg++);
      
      ss2 += 2.0 * ( S[i0++] * mysincgetfiltervalue_pre(iarg++) );
      ss2 += ( S[i0] * mysincgetfiltervalue_pre(iarg) );
    
      ss += ss2 / 3.0;

    }
  
    else if (  modus == 3 ) {/* picking in Hilbert transformed trace 
				low-pass filtered with fc 
				with 1-0.66-0.33 tapering at end of operator */

      len = std::min(i0, nt-1-i0);
      if ( len < 2 )
	return 0.0;
      len = std::min(width_filter, len);
      LEN2 = 2 * len; 
     
      tdt_rest = t - (float)i0 * dt;
      itdt_rest = std::min(oversamp-1, (int)(tdt_rest / dt_sub));

      
      iFreqs = return_fbandIndex(fc) * n_oneBand;
      

      iarg = itdt_rest * wt_filter + iFreqs + (width_filter - len);
      i0 -= len;
      
      ss2 = ( S[i0++] * mysincgetfilterhilbertvalue_pre(iarg++) );
      ss2 += 2.0 * ( S[i0++] * mysincgetfilterhilbertvalue_pre(iarg++) );
      
      ss = 0.0;
      for ( i=2; i<LEN2-1; i++ )
	ss += S[i0++] * mysincgetfilterhilbertvalue_pre(iarg++);
      
      ss2 += 2.0 * ( S[i0++] * mysincgetfilterhilbertvalue_pre(iarg++) );
      ss2 += ( S[i0] * mysincgetfilterhilbertvalue_pre(iarg) );
      
      ss += ss2 / 3.0;
      
    }
    
    else {/* forbidden */
      ss = 0.0;
    }
    
    return ss;
  }
  
  



#ifdef __SSE__
  __m128 interpolate_sinc_sse( float **  S, const int nt, const float t0, const float _t[4], const int modus[4]) const
  {
    float t_0, t_1, t_2, t_3;
    float tdt_0, tdt_1, tdt_2, tdt_3;
    float tdt_rest_0, tdt_rest_1, tdt_rest_2, tdt_rest_3;
    int i0_0, i0_1, i0_2, i0_3;
    int iarg_0, iarg_1, iarg_2, iarg_3;
    int itdt_rest_0, itdt_rest_1, itdt_rest_2, itdt_rest_3;
    __m128 ss = _mm_setzero_ps();
    {
	t_0 = _t[0] - t0;
	tdt_0 = t_0/dt;
	i0_0 = (int)tdt_0;
	tdt_rest_0 = t_0 - (float)(i0_0) * dt;
	itdt_rest_0 = std::min(oversamp-1, (int)(tdt_rest_0 / dt_sub));
    }

    {
	t_1 = _t[1] - t0;
	tdt_1 = t_1/dt;
	i0_1 = (int)tdt_1;
	tdt_rest_1 = t_1 - (float)(i0_1) * dt;
	itdt_rest_1 = std::min(oversamp-1, (int)(tdt_rest_1 / dt_sub));
    }

    {
	t_2 = _t[2] - t0;
	tdt_2 = t_2/dt;
	i0_2 = (int)tdt_2;
	tdt_rest_2 = t_2 - (float)(i0_2) * dt;
	itdt_rest_2 = std::min(oversamp-1, (int)(tdt_rest_2 / dt_sub));
    }

    {
	t_3 = _t[3] - t0;
	tdt_3 = t_3/dt;
	i0_3 = (int)tdt_3;
	tdt_rest_3 = t_3 - (float)(i0_3) * dt;
	itdt_rest_3 = std::min(oversamp-1, (int)(tdt_rest_3 / dt_sub));
    }

    if ( (i0_0 < width_max_sse )
	 || (i0_1 < width_max_sse )
	 || (i0_2 < width_max_sse )
	 || (i0_3 < width_max_sse ))
      {
	return ss;
      }
    if ( (i0_0 >= nt-width_max_sse )
	 || (i0_1 >= nt-width_max_sse )
	 || (i0_2 >= nt-width_max_sse )
	 || (i0_3 >= nt-width_max_sse ))
      {
	return ss;
      }

    

    if (  (modus[0] != 0) || (modus[1] != 0) || (modus[2] != 0) || (modus[3] != 0) ) {/* picking in Hilbert tranform */
	const float A0 = interp_sinc(S[0],  nt,  t0,  _t[0],  modus[0]);
	const float A1 = interp_sinc(S[1],  nt,  t0,  _t[1],  modus[1]);
	const float A2 = interp_sinc(S[2],  nt,  t0,  _t[2],  modus[2]);
	const float A3 = interp_sinc(S[3],  nt,  t0,  _t[3],  modus[3]);
	ss = _mm_set_ps(A3, A2, A1, A0);
    }
    else 
      {/* ordinary */
      
	  iarg_0 = itdt_rest_0 * wt_normal;
	  i0_0 -= width_normal;
	  iarg_1 = itdt_rest_1 * wt_normal;
	  i0_1 -= width_normal;
	  iarg_2 = itdt_rest_2 * wt_normal;
	  i0_2 -= width_normal;
	  iarg_3 = itdt_rest_3 * wt_normal;
	  i0_3 -= width_normal;

      
	ss = _mm_set_ps(0.5 * S[3][i0_3] * sinc_normal_pre[iarg_3],
			 0.5 * S[2][i0_2] * sinc_normal_pre[iarg_2],
			 0.5 * S[1][i0_1] * sinc_normal_pre[iarg_1],
			 0.5 * S[0][i0_0] * sinc_normal_pre[iarg_0]);


	i0_0++;
	iarg_0++;
	i0_1++;
	iarg_1++;
	i0_2++;
	iarg_2++;
	i0_3++;
	iarg_3++;

      
	int i = 0;
	for ( i=1; i<4*((wt_normal-1)/4); i+=4 )
	{
	    
	    __m128 Amp_t0_tmp =  _mm_setzero_ps();
	    Amp_t0_tmp = _mm_loadl_pi(Amp_t0_tmp, (__m64*) &(S[0][i0_0]));
	    Amp_t0_tmp = _mm_loadh_pi(Amp_t0_tmp, (__m64*) &(S[1][i0_1]));
	    
	    __m128 Amp_t1_tmp =  _mm_setzero_ps();
	    Amp_t1_tmp = _mm_loadl_pi(Amp_t1_tmp, (__m64*) &(S[2][i0_2]));
	    Amp_t1_tmp = _mm_loadh_pi(Amp_t1_tmp, (__m64*) &(S[3][i0_3]));
	    
	    __m128 Amp_t2_tmp =  _mm_setzero_ps();
	    Amp_t2_tmp = _mm_loadl_pi(Amp_t2_tmp, (__m64*) &(S[0][i0_0 + 2]));
	    Amp_t2_tmp = _mm_loadh_pi(Amp_t2_tmp, (__m64*) &(S[1][i0_1 + 2]));
    
	    __m128 Amp_t3_tmp =  _mm_setzero_ps();
	    Amp_t3_tmp = _mm_loadl_pi(Amp_t3_tmp, (__m64*) &(S[2][i0_2 + 2]));
	    Amp_t3_tmp = _mm_loadh_pi(Amp_t3_tmp, (__m64*) &(S[3][i0_3 + 2]));
	      
	    __m128 weight_t0_tmp =  _mm_setzero_ps();
	    weight_t0_tmp = _mm_loadl_pi(weight_t0_tmp, (__m64*) &( sinc_normal_pre[iarg_0]));
	    weight_t0_tmp = _mm_loadh_pi(weight_t0_tmp, (__m64*) &( sinc_normal_pre[iarg_1]));
	      
	    __m128 weight_t1_tmp =  _mm_setzero_ps();
	    weight_t1_tmp = _mm_loadl_pi(weight_t1_tmp, (__m64*) &( sinc_normal_pre[iarg_2]));
	    weight_t1_tmp = _mm_loadh_pi(weight_t1_tmp, (__m64*) &( sinc_normal_pre[iarg_3]));

	    __m128 weight_t2_tmp =  _mm_setzero_ps();
	    weight_t2_tmp = _mm_loadl_pi(weight_t2_tmp, (__m64*) &( sinc_normal_pre[iarg_0 + 2]));
	    weight_t2_tmp = _mm_loadh_pi(weight_t2_tmp, (__m64*) &( sinc_normal_pre[iarg_1 + 2]));
	      
	    __m128 weight_t3_tmp =  _mm_setzero_ps();
	    weight_t3_tmp = _mm_loadl_pi(weight_t3_tmp, (__m64*) &( sinc_normal_pre[iarg_2 + 2]));
	    weight_t3_tmp = _mm_loadh_pi(weight_t3_tmp, (__m64*) &( sinc_normal_pre[iarg_3 + 2]));

	    const __m128 wAmp_t0_tmp = _mm_mul_ps(weight_t0_tmp, Amp_t0_tmp);
	    const __m128 wAmp_t1_tmp = _mm_mul_ps(weight_t1_tmp, Amp_t1_tmp);
	    const __m128 wAmp_t2_tmp = _mm_mul_ps(weight_t2_tmp, Amp_t2_tmp);
	    const __m128 wAmp_t3_tmp = _mm_mul_ps(weight_t3_tmp, Amp_t3_tmp);

	    const __m128 wAmp_t0 = _mm_shuffle_ps( wAmp_t0_tmp, wAmp_t1_tmp, _MM_SHUFFLE( 2, 0, 2, 0));
	    const __m128 wAmp_t1 = _mm_shuffle_ps( wAmp_t0_tmp, wAmp_t1_tmp, _MM_SHUFFLE( 3, 1, 3, 1));
	    
	    const __m128 wAmp_t2 = _mm_shuffle_ps( wAmp_t2_tmp, wAmp_t3_tmp, _MM_SHUFFLE( 2, 0, 2, 0));
	    const __m128 wAmp_t3 = _mm_shuffle_ps( wAmp_t2_tmp, wAmp_t3_tmp, _MM_SHUFFLE( 3, 1, 3, 1));
	      
	    ss = _mm_add_ps( ss, wAmp_t0);
	    ss = _mm_add_ps( ss, wAmp_t1);
	    ss = _mm_add_ps( ss, wAmp_t2);
	    ss = _mm_add_ps( ss, wAmp_t3);

	    i0_0+=4;
	    iarg_0+=4;
	    i0_1+=4;
	    iarg_1+=4;
	    i0_2+=4;
	    iarg_2+=4;
	    i0_3+=4;
	    iarg_3+=4;
	}

	for (; i<(wt_normal-1); i++ )
	{
	    ss =  _mm_add_ps(ss, _mm_set_ps(S[3][i0_3] * sinc_normal_pre[iarg_3],
					      S[2][i0_2] * sinc_normal_pre[iarg_2],
					      S[1][i0_1] * sinc_normal_pre[iarg_1],
					      S[0][i0_0] * sinc_normal_pre[iarg_0]));
	    i0_0++;
	    iarg_0++;
	    i0_1++;
	    iarg_1++;
	    i0_2++;
	    iarg_2++;
	    i0_3++;
	    iarg_3++;
	}
	
	ss =  _mm_add_ps(ss, _mm_set_ps(0.5 * S[3][i0_3] * sinc_normal_pre[iarg_3],
					  0.5 * S[2][i0_2] * sinc_normal_pre[iarg_2],
					  0.5 * S[1][i0_1] * sinc_normal_pre[iarg_1],
					  0.5 * S[0][i0_0] * sinc_normal_pre[iarg_0]));
	
      }

    return ss;
  }
#endif

#if (defined(__ALTIVEC__) || defined(__SPU__))
  void interpolate_sinc_sse( float **  S, const int nt, const float t0, const vector float t_sse, const int modus[4], vector float* ss) const
  {
      unsigned short i0_0, i0_1, i0_2, i0_3;
      unsigned short iarg_0, iarg_1, iarg_2, iarg_3;
      int itdt_rest_0, itdt_rest_1, itdt_rest_2, itdt_rest_3;

      vector float T0_vec =  spu_splats(t0);  
      vector float t_0_vec = spu_sub(t_sse, T0_vec); 
      vector unsigned int it0vec = spu_convtu( spu_mul(t_0_vec, dtbin_inv_vec), 0); 

      vector unsigned int nt_width_max_vec = ( vector unsigned int) spu_splats(nt - width_max_sse); 
      vector float T_width_max_vec = spu_splats(t0 + width_max_sse * dt); 

      vector unsigned int mask = spu_and(spu_cmpgt(it0vec, width_max_vec), spu_cmpgt(nt_width_max_vec, it0vec)); 
      vector unsigned int it0vec_sel = spu_sel(width_max_vec, it0vec, mask);
      vector float t_0_vec_sel = spu_sel(T_width_max_vec, t_0_vec, mask);

      vector float tdt_rest = spu_sub(t_0_vec_sel, spu_mul(spu_convtf(it0vec_sel, 0), dtbin_vec));
      vector signed int itdt_rest_tmp = spu_convts(spu_mul( tdt_rest, dtsub_inv_vec), 0);
      vector signed int idt_rest = spu_sel(oversamp_1_vec, itdt_rest_tmp, spu_cmpgt(oversamp_1_vec, itdt_rest_tmp));

      {
	  i0_0 = ((unsigned int*)&it0vec_sel)[0] - width_normal; 
	  iarg_0 = ((int*)&idt_rest)[0] * wt_normal;
      }

      {
	  i0_1 = ((unsigned int*)&it0vec_sel)[1] - width_normal; 
	  iarg_1 = ((int*)&idt_rest)[1] * wt_normal;
      }

      {
	  i0_2 = ((unsigned int*)&it0vec_sel)[2] - width_normal; 
	  iarg_2 = ((int*)&idt_rest)[2] * wt_normal;
      }

      {
	  i0_3 = ((unsigned int*)&it0vec_sel)[3] - width_normal; 
	  iarg_3 = ((int*)&idt_rest)[3] * wt_normal;
      }

    
      vector float tmp =  {0.5f * S[0][i0_0] * sinc_normal_pre[iarg_0],
			   0.5f * S[1][i0_1] * sinc_normal_pre[iarg_1],
			   0.5f * S[2][i0_2] * sinc_normal_pre[iarg_2],
			   0.5f * S[3][i0_3] * sinc_normal_pre[iarg_3]};
      *ss = tmp;


      i0_0++;
      iarg_0++;
      i0_1++;
      iarg_1++;
      i0_2++;
      iarg_2++;
      i0_3++;
      iarg_3++;

      
      const vector unsigned char pattern1 = {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23};
      const vector unsigned char pattern2 = {8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31};

      const vector unsigned char pattern3 = {0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27};
      const vector unsigned char pattern4 = {4, 5, 6, 7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31};
	      
      vector float wAmp0_tmp = spu_splats(0.0f);
      vector float wAmp1_tmp = spu_splats(0.0f);
      vector float wAmp2_tmp = spu_splats(0.0f);
      vector float wAmp3_tmp = spu_splats(0.0f);

      const unsigned long A0_shift = (unsigned long) &(S[0][i0_0]) & 15;
      const unsigned long A1_shift = (unsigned long) &(S[1][i0_1]) & 15;
      const unsigned long A2_shift = (unsigned long) &(S[2][i0_2]) & 15;
      const unsigned long A3_shift = (unsigned long) &(S[3][i0_3]) & 15;

      const unsigned long w0_shift = (unsigned long) &(sinc_normal_pre[iarg_0]) & 15;
      const unsigned long w1_shift = (unsigned long) &(sinc_normal_pre[iarg_1]) & 15;
      const unsigned long w2_shift = (unsigned long) &(sinc_normal_pre[iarg_2]) & 15;
      const unsigned long w3_shift = (unsigned long) &(sinc_normal_pre[iarg_3]) & 15;

      unsigned short i = 1;
      const unsigned short ilimit = ((wt_normal-1)>>2)<<2;
      for ( ; i < ilimit; i+=(unsigned short)4 )
      {

	  vector float Amp0_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(S[0][i0_0]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      Amp0_tmp =  spu_or(spu_slqwbyte(qw0, A0_shift),spu_rlmaskqwbyte(qw1, A0_shift-16));
	  }

	  vector float Amp1_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(S[1][i0_1]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      Amp1_tmp =  spu_or(spu_slqwbyte(qw0, A1_shift),spu_rlmaskqwbyte(qw1, A1_shift-16));
	  }

	  vector float Amp2_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(S[2][i0_2]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      Amp2_tmp =  spu_or(spu_slqwbyte(qw0, A2_shift),spu_rlmaskqwbyte(qw1, A2_shift-16));
	  }

	  vector float Amp3_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(S[3][i0_3]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      Amp3_tmp =  spu_or(spu_slqwbyte(qw0, A3_shift),spu_rlmaskqwbyte(qw1, A3_shift-16));
	  }


	  vector float weight0_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(sinc_normal_pre[iarg_0]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      weight0_tmp =  spu_or(spu_slqwbyte(qw0, w0_shift),spu_rlmaskqwbyte(qw1, w0_shift-16));
	  }

	  vector float weight1_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(sinc_normal_pre[iarg_1]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      weight1_tmp =  spu_or(spu_slqwbyte(qw0, w1_shift),spu_rlmaskqwbyte(qw1, w1_shift-16));
	  }

	  vector float weight2_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(sinc_normal_pre[iarg_2]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      weight2_tmp =  spu_or(spu_slqwbyte(qw0, w2_shift),spu_rlmaskqwbyte(qw1, w2_shift-16));
	  }

	  vector float weight3_tmp;
	  {
	      vector float qw0, qw1;
	      vector float* ptr = (vector float*)(&(sinc_normal_pre[iarg_3]));
	      qw0 = *ptr;
	      qw1 = *(ptr+1);
	      weight3_tmp =  spu_or(spu_slqwbyte(qw0, w3_shift),spu_rlmaskqwbyte(qw1, w3_shift-16));
	  }

	  wAmp0_tmp = spu_madd(Amp0_tmp, weight0_tmp, wAmp0_tmp);
	  wAmp1_tmp = spu_madd(Amp1_tmp, weight1_tmp, wAmp1_tmp);
	  wAmp2_tmp = spu_madd(Amp2_tmp, weight2_tmp, wAmp2_tmp);
	  wAmp3_tmp = spu_madd(Amp3_tmp, weight3_tmp, wAmp3_tmp);


	  i0_0+=4;
	  iarg_0+=4;
	  i0_1+=4;
	  iarg_1+=4;
	  i0_2+=4;
	  iarg_2+=4;
	  i0_3+=4;
	  iarg_3+=4;
      }

      vector float wAmp01_01_tmp = spu_shuffle( wAmp0_tmp, wAmp1_tmp, pattern1);
      vector float wAmp01_23_tmp = spu_shuffle( wAmp0_tmp, wAmp1_tmp, pattern2);
      vector float wAmp23_01_tmp = spu_shuffle( wAmp2_tmp, wAmp3_tmp, pattern1);
      vector float wAmp23_23_tmp = spu_shuffle( wAmp2_tmp, wAmp3_tmp, pattern2);
	
      vector float wAmp01_02_13_tmp = spu_add( wAmp01_01_tmp, wAmp01_23_tmp);
      vector float wAmp23_02_13_tmp = spu_add( wAmp23_01_tmp, wAmp23_23_tmp);
	
      vector float wAmp0123_02_tmp = spu_shuffle( wAmp01_02_13_tmp,  wAmp23_02_13_tmp, pattern3);
      vector float wAmp0123_13_tmp = spu_shuffle( wAmp01_02_13_tmp,  wAmp23_02_13_tmp, pattern4);
	
      vector float wAmp0123_tmp = spu_add( wAmp0123_02_tmp, wAmp0123_13_tmp);
	
      *ss = spu_add( *ss, wAmp0123_tmp);

      for (; i<(wt_normal-1); i++ )
      {
	  vector float tmp1 = {S[0][i0_0] * sinc_normal_pre[iarg_0],
			       S[1][i0_1] * sinc_normal_pre[iarg_1],
			       S[2][i0_2] * sinc_normal_pre[iarg_2],
			       S[3][i0_3] * sinc_normal_pre[iarg_3]};
	  *ss =  spu_add(*ss, tmp1);
	  i0_0++;
	  iarg_0++;
	  i0_1++;
	  iarg_1++;
	  i0_2++;
	  iarg_2++;
	  i0_3++;
	  iarg_3++;
      }
	
      vector float tmp2 = {0.5f * S[0][i0_0] * sinc_normal_pre[iarg_0],
			   0.5f * S[1][i0_1] * sinc_normal_pre[iarg_1],
			   0.5f * S[2][i0_2] * sinc_normal_pre[iarg_2],
			   0.5f * S[3][i0_3] * sinc_normal_pre[iarg_3]}; 
      *ss =  spu_sel( (vector float) spu_splats(0), spu_add(*ss, tmp2), mask);
  }
#endif

  float interp_sinc_exact(float*, int, float, float, short, float fc=-1.) const;

  float mysincgetnormalvalue(float) const;
  float mysincgethilbertvalue(float) const;
  float mysincgetfiltervalue(float, float) const;
  float mysincgetfilterhilbertvalue(float, float) const;

/* ################################################################ */
  inline float mysincgetnormalvalue_pre(const int iarg) const {
    return sinc_normal_pre[iarg];
  }
  inline float mysincgethilbertvalue_pre(const int iarg) const {
    return sinc_hilbert_pre[iarg];
  }
  inline float mysincgetfiltervalue_pre(const int iarg) const {
    return sinc_filter_pre_allHz[iarg];
  }
  inline float mysincgetfilterhilbertvalue_pre(const int iarg) const {
    return sinc_filterhilbert_pre_allHz[iarg];
  }
  
  float get_normaloplength_halftime() const;
  float get_hilbertoplength_halftime() const;
  float get_filteroplength_halftime() const;
  float get_alloplength_halftime() const;

};
#endif

