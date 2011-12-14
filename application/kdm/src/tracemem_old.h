/***************************************************************************
                          tracemem.h  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2009-08-03
               UTM has been changed to MOD; 
               Coordinates are stored and returned in model system
      merten | 2009-06-29
               using class UTM instead of float for sx, etc. coordinates
***************************************************************************/


#ifndef TRACEMEM_H
#define TRACEMEM_H


/**
 *@author Dirk Merten
 */
#include "include/defs.h"
#include "include/consts.h"
#include "structures/migrationjob.h"
#include "structures/mod.h"
#include "filehandler/tracefilehandler.h"
#include "filehandler/tracememhandler.h"
#include "utils/sincinterpolator.h"
#include "utils/fft.h"
#include "utils/Acq_geometry.hpp"

#include <iostream>
#include <fstream>
#ifdef __SSE__
#include <xmmintrin.h>
typedef  __m128 vector_float;
#endif

#ifdef __ALTIVEC__
#include <altivec.h>
#include <spu2vmx.h>
typedef vector float vector_float;
#endif


class TraceMem {
 public:

  enum filetype{SEGY};
  
  TraceMem();
  void Init(TraceFileHandler& TFHandler, const MigrationJob& Job);
  TraceMem(TraceMemHandler& TFHandler);
  TraceMem(std::ifstream& Input, filetype ft = SEGY);
  ~TraceMem();

  int getNt() const {return Nt;};
  float getT0() const {return T0;};
  float getdtbin() const {return dtbin;};
  MOD getsx() const {return sx;};
  MOD getsy() const {return sy;};
  MOD getgx() const {return gx;};
  MOD getgy() const {return gy;};
  float GetData(const int& it) const;
  float GetData(const int& it, const float Frequ) const;
  float GetData(const float& t) const;

  void GetData(float* t, float* Amp) const
  {
      float* Tptr[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  Tptr[icont] = &Trace[0];
      }
      
      int modus[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  modus[icont] = 0;
      }
      
      vector_float Amp_sse;
#ifdef __ALTIVEC__
      vector_float t_sse = {t[0], t[1], t[2], t[3]}; 
      SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t_sse, modus, &Amp_sse);
#endif
#ifdef __SSE__
      Amp_sse = SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t, modus);
#endif      
      for (int icont = 0; icont < 4; icont++)
      {
	  Amp[icont] = ((float*)(&Amp_sse))[icont];
      }
      
  };

  void GetFilteredData(float* t, float* Amp, const float Frequ) const
  {
      float* Tptr[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  unsigned int FilterIndex = std::max(std::min((int)((Frequ-Frequmin)/dFrequ), ( int) 4), 0);
	  Tptr[icont] = FilteredTraces[FilterIndex];
      }
      
      int modus[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  modus[icont] = 0;
      }
      
      vector_float Amp_sse;
#ifdef __ALTIVEC__
      vector_float t_sse = {t[0], t[1], t[2], t[3]}; 
      SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t_sse, modus, &Amp_sse);
#endif
#ifdef __SSE__
      Amp_sse = SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t, modus);
#endif
     
      for (int icont = 0; icont < 4; icont++)
      {
	  Amp[icont] = ((float*)(&Amp_sse))[icont];
      }
  };


  void Dump() const;
  char* getTPtr()
    {
      return (char*)Trace;
    }
 private:
  void PrepareData(const MigrationJob& Job);
  
  SincInterpolator SincInt;

  float * Trace;
  float ** FilteredTraces;
  int Nt;
  float dtbin;

  float T0, Tmax;
  MOD sx, sy, gx, gy;

  float Frequmin;
  float dFrequ;

  float* frac_array;
  float* filter20;
  float* filter40;
  float* filter60; 
  float* filter80;
  float* filter_unlimited;
  float* bandpass_filter;

  vector_float* fftarray_filtered;
  float* fftarray;
  float* fftarray20;
  float* fftarray40;
  float* fftarray60;
  float* fftarray80;
};

#endif
