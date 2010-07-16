#include "sincinterpolator.h"

/* ################################################################ */
/* ################################################################ */
/* ################################################################ */

/* ################################################################ */
int SincInterpolator::init(float dt_ext, int width_normal_extern, 
			    float fc_limit_extern) {
  int i, ii, count, iaux_first;
  float aux;

  if ( fc_limit_extern < 10.0 )
    return -1;
  if ( fc_limit_extern > 250.0 )
    return -2;

  dt = dt_ext;
  oversamp = 100;
  dt_sub = dt / (float)oversamp;

  width_normal = width_normal_extern;
  width_hilbert = 11;
  width_filter = 21;

  wt_normal = 2 * width_normal + 1;
  wt_hilbert = 2 * width_hilbert + 1;
  wt_filter = 2 * width_filter + 1;

  width_max = width_normal;  // wegen der Vertraeglichkeit mit fast beam

  width_max_sse = std::max(width_normal, width_hilbert);
  width_max_sse = std::max(width_max_sse, width_filter);


  fc_limit = fc_limit_extern;
  Freqs0 = 5.0;
  dFreqs = 5.0;
  nFreqs = std::max(1, (int)(( fc_limit + dFreqs/10.0 - Freqs0 ) / dFreqs) );

  terminate();

  sinc_normal_pre = new float [wt_normal * oversamp];
  sinc_hilbert_pre = new float [wt_hilbert * oversamp];
  sinc_filter_pre_allHz = new float [wt_filter * oversamp * nFreqs];
  sinc_filterhilbert_pre_allHz = new float [wt_filter * oversamp * nFreqs];
  Freqs = new float [nFreqs];


  iaux_first = width_normal;
  count=0;
  for ( ii=0; ii<oversamp; ii++ ) {
    aux = (float)ii/(float)oversamp;
    for ( i=0; i<wt_normal; i++ )
      sinc_normal_pre[count++] = mysincgetnormalvalue((float)(iaux_first-i) + aux);
  }

  iaux_first = width_hilbert;
  count=0;
  for ( ii=0; ii<oversamp; ii++ ) {
    aux = (float)ii/(float)oversamp;
    for ( i=0; i<wt_hilbert; i++ )
      sinc_hilbert_pre[count++] = mysincgethilbertvalue((float)(iaux_first-i) + aux);
  }



  iaux_first = width_filter;
  n_oneBand = 0;
  for ( int iHz=0; iHz < nFreqs; iHz++ ) { /* loop over frequency bands */

    count=0;
    Freqs[iHz] = Freqs0 + (float)iHz * dFreqs;
    for ( ii=0; ii<oversamp; ii++ ) {
      aux = (float)ii/(float)oversamp;
      for ( i=0; i<wt_filter; i++ ) {

	sinc_filter_pre_allHz[iHz * n_oneBand + count] = 
	  mysincgetfiltervalue((float)(iaux_first-i) + aux, Freqs[iHz]);
	sinc_filterhilbert_pre_allHz[iHz * n_oneBand + count] = 
	  mysincgetfilterhilbertvalue((float)(iaux_first-i) + aux, Freqs[iHz]);

	count++;
      }
    }
    n_oneBand = count;

  }

  
#if (defined(__ALTIVEC__) || defined(__SPU__))
  dtbin_vec =  spu_splats(dt);
  dtbin_inv_vec =  spu_splats(1.0f/dt);
  dtsub_inv_vec =  spu_splats(1.0f/dt_sub);
  oversamp_1_vec = (vector signed int) spu_splats(oversamp-1);
  width_normal_vec = ( vector unsigned int) spu_splats(width_normal);
  width_hilbert_vec = ( vector unsigned int) spu_splats(width_hilbert);
  width_max_vec = ( vector unsigned int) spu_splats(width_max_sse);
  wt_normal_vec = ( vector signed int) spu_splats(wt_normal);
#endif

  return 0;
}

/* ################################################################ */
void SincInterpolator::terminate() {
  if (sinc_normal_pre != NULL)
    delete [] sinc_normal_pre;
  if (sinc_hilbert_pre != NULL)
    delete [] sinc_hilbert_pre;
  if (sinc_filter_pre_allHz != NULL)
    delete [] sinc_filter_pre_allHz;
  if (sinc_filterhilbert_pre_allHz != NULL)
    delete [] sinc_filterhilbert_pre_allHz;
  if (Freqs != NULL)
    delete [] Freqs;

}

/* ################################################################ */
float SincInterpolator::get_normaloplength_halftime() const{
  return (  (float)(width_normal+1) * dt );
}
/* ################################################################ */
float SincInterpolator::get_hilbertoplength_halftime() const{
  return (  (float)(width_hilbert+1) * dt );
}
/* ################################################################ */
float SincInterpolator::get_filteroplength_halftime() const{
  return (  (float)(width_filter+1) * dt );
}
/* ################################################################ */
float SincInterpolator::get_alloplength_halftime() const{
  float aux = 0.0;
  aux = std::max(get_normaloplength_halftime(),
		 get_hilbertoplength_halftime());
  aux = std::max(aux,
		 get_filteroplength_halftime());

  return aux;
}



/* ################################################################ */
/* ################################################################ */

#ifdef STANDALONE
/* ################################################################ */
float SincInterpolator::interp_lin(float *S, int nt, float t0, float t) {
  t -= t0;
  int it = (int)(t/dt);
  float ss1, ss2, ss;
  
  if ( it >= 0 && it < nt-1 ) {
    ss1 = S[it];
    ss2 = S[it+1];
    ss = ss1 + (ss2-ss1)/dt * ( t - (float)it*dt );
  }
  else
    ss = 0.0;

  return ss;
}
#endif


/* ################################################################ */
/* ################################################################ */

/* ################################################################ */
float SincInterpolator::mysincgetnormalvalue(float arg) const {
  if ( fabs(arg) < 1e-6 )
    return 1.0;
  else
    return sin(M_PI*arg) / (M_PI*arg);
}
/* ################################################################ */
float SincInterpolator::mysincgethilbertvalue(float arg) const {
  if ( fabs(arg) < 1e-6 )
    return 0.0;
  else
    return ( 1.0 - cos(M_PI*arg) ) / (M_PI*arg);
}
/* ################################################################ */
float SincInterpolator::mysincgetfiltervalue(float arg, float fc) const {
  int L = 40;
  float aux = 0.0;
  float fac = 2. * fc * dt;
  
  for ( int i=-L; i<=L; i++ ) {
    aux += mysincgetnormalvalue(arg + (float)i) *
      mysincgetnormalvalue(fac * (float)i);
  }

  aux *= fac;

  return aux;
}
/* ################################################################ */
float SincInterpolator::mysincgetfilterhilbertvalue(float arg, float fc) const {
  int L = 40;
  float aux = 0.0;
  float fac = 2. * fc * dt;
  
  for ( int i=-L; i<=L; i++ ) {
    aux += mysincgethilbertvalue(arg + (float)i) *
      mysincgetnormalvalue(fac * (float)i);
  }

  aux *= fac;

  return aux;
}


/* ################################################################ */
float SincInterpolator::interp_sinc_exact(float * S, int nt, float t0, 
					  float t, short modus, 
					  float fc) const{
  int i, ii, i0, len;
  float arg, tdt, ss;
  ss = 0.0f;

  float eps = 0.01;
  
  if ( ( fc >= fc_limit-eps || fc < 0.0 ) && modus > 1 )
    modus -= 2;


  t -= t0;
  tdt = t/dt;
  i0 = (int)(tdt);

  
  if (  modus == 0 ) {/* ordinary */

    len = std::min(i0, nt-i0-1);
    len = std::min(width_normal, len);

    ii = i0 - len -1;

    for ( i=-len; i<=len; i++ ) {
      ii++;
      arg = tdt-(float)ii;
      ss += S[ii] * mysincgetnormalvalue(arg);
    }

  }
  else if (  modus == 1 ) {/* picking in Hilbert transform */

    len = std::min(i0, nt-i0-1);
    len = std::min(width_hilbert, len);

    ii = i0 - len -1;

    for ( i=-len; i<=len; i++ ) {
      ii++;
      arg = tdt-(float)ii;
      ss += S[ii] * mysincgethilbertvalue(arg);
    }

  }
  else if (  modus == 2 ) {/* picking in filtered trace*/

    len = std::min(i0, nt-i0-1);
    len = std::min(width_filter, len);

    ii = i0 - len -1;

    for ( i=-len; i<=len; i++ ) {
      ii++;
      arg = tdt-(float)ii;
      ss += S[ii] * mysincgetfiltervalue(arg, fc);
    }

  }
  else if (  modus == 3 ) {/* picking in filtered Hilbert transformed trace*/

    len = std::min(i0, nt-i0-1);
    len = std::min(width_filter, len);

    ii = i0 - len -1;

    for ( i=-len; i<=len; i++ ) {
      ii++;
      arg = tdt-(float)ii;
      ss += S[ii] * mysincgetfilterhilbertvalue(arg, fc);
    }

  }
 
  return ss;
}


