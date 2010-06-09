/***************************************************************************
                          tracememhandler.h  -  description

   Handler for trace data in the local memory.

                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef TRACEMEMHANDLER_H
#define TRACEMEMHANDLER_H


/**
  *@author Dirk Merten
  */
#include "SegYHeader.h"
#include "rawfloatheader.h"
#include "utils/swap_bytes.h"
#include "include/defs.h"
#include "include/migtypes.h"
#include <fstream>
#include <iostream>



class TraceMemHandler {
public: 
  TraceMemHandler():Nt(0),dt(0),T0(0){};
  TraceMemHandler(char* MemPtr, FILE_MODE _F_MODE);
  ~TraceMemHandler();

  // Initialize to next trace in trace.
  int Next();
  // Read the current Header, scale and store the corresponding values in the briefheader
  void ReadTraceData(float* Trace);
  int getNt(){return Nt;}
  float getdt(){return dt;}
  float getT0(){return T0;}
  int getTracesize();

  // Go back for N traces
  void Rewind();
  void Rewind(const int N);

  RawFloatHeader RawInfo;

private:
  char* MemInput;
  char* MemInputBegin;
  int Nt;
  float dt;
  float T0;

  // number of current trace
  int tracl;
  FILE_MODE F_MODE;
  int endianess;

  
};

#endif
