/***************************************************************************
                          CharTraceDataSet.hpp  -  description
*/

/**
    Doxygen Style
    Multi-Line Documentation
**/

/*                           -------------------
    begin                :
    copyright            : (C) 2010 by Daniel Grünewald
    email                : Daniel.Gruenewald@itwm.fraunhofer.de
 ***************************************************************************/


#ifndef TRACEDATA_H
#define TRACEDATA_H

// user defined includes
#include "include/defs.h"
#include "include/consts.h"
#include "structures/migrationjob.h"
#include "structures/mod.h"
#include "filehandler/tracefilehandler.h"
#include "filehandler/tracememhandler.h"
#include "utils/sincinterpolator.h"
#include "utils/fft.h"
#include "utils/Acq_geometry.hpp"

// standard includes
#include <stdio.h>
#include <string.h>
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

/**
  *@author Daniel Grünewald doxygen stale
  */

class TraceData {

// public methods
 public:
    //enum filetype{SEGY};
    TraceData();
    TraceData(volatile char * _pTraceData, const int &_Nt);
    //TraceData(TraceMemHandler& TFHandler);
    //TraceData(std::ifstream& Input, filetype ft = SEGY);
    ~TraceData();

    void SetNewCharPtr(volatile char *_pTraceData,const int &_Nt);
    volatile char * getDataPtr();

  void LoadFromDisk(TraceFileHandler& TFHandler, const MigrationJob& Job);

    int getNt() const {return Nt;};
    float getT0() const {return T0;};
    float getdtbin() const {return dtbin;};
    MOD getsx() const {return sx;};
    MOD getsy() const {return sy;};
    MOD getgx() const {return gx;};
    MOD getgy() const {return gy;};
    float GetData(const int& it) const;
    float GetData(const int& it, const float Frequ) const;
    float GetData(const float& t, SincInterpolator &SincInt) const;
    void GetData(float* t, float* Amp, SincInterpolator &SincInt) const;
    void GetFilteredData(float* t, float* Amp, const float Frequ, SincInterpolator &SincInt) const;
    void Dump() const;
    char* getTPtr();

// public attributes
 public:

// private methods
 private:

    TraceData(const TraceData& T);

    TraceData& operator = (const TraceData& T);

    void PrepareData(const MigrationJob& Job);

    void setNtToChar(const int &Nt);
    int  getNtFromChar();
    void setdtToChar(const float &dt);
    float getdtFromChar();
    void setT0ToChar(float &T0);
    float getT0FromChar();
    void setTmaxToChar(const float &Tmax);
    float getTmaxFromChar();
    void setsxToChar(const float &sx);
    float getsxFromChar();
    void setsyToChar(const float &sy);
    float getsyFromChar();
    void setgxToChar(const float &gx);
    float getgxFromChar();
    void setgyToChar(const float &gy);
    float getgyFromChar();
//     void setFrequmin(const float &Freqmin);
//     float getFrequmin();
//     void setFrequ(const float &Freq);
//     float getFrequ();
    float * getTraceFromChar();
    void getMemVarsFromChar();

// private attributes
 private:

  volatile char * pTraceData;

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

  int TraceMemRqst;

};

#endif

