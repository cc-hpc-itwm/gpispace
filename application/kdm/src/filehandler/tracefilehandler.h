/***************************************************************************
                          tracefilehandler.h  -  description

    Handler for trace data files in SegY, SU format.

                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2009-06-29
               using class UTM instead of float for sx, etc. coordinates
 ***************************************************************************/


#ifndef TRACEFILEHANDLER_H
#define TRACEFILEHANDLER_H


/**
  *@author Dirk Merten
  */
#include "include/defs.h"
#include "include/migtypes.h"
#include "structures/utm.h"
#include "utils/swap_bytes.h"
#include "SegYHeader.h"
#include "SegYBHeader.h"
#include "SegYEBCHeader.h"
#include "rawfloatheader.h"
#include <fstream>
#include <iostream>

//

struct briefheadercoord
{
  float sx;
  float sy;
  float gx;
  float gy;
};

struct briefheader
{
    int Nt;
    float dt;
    float T0;
    float sx, sy, gx, gy;
    float selev, gelev;
    int CDP;
    int swdep;
    int gwdep;

    float CDPx;
    float CDPy;
    float Offx;
    float Offy;
};

class TraceFileHandler {

// public methods
public:
    TraceFileHandler():	LengthOfFileHeader(0),LengthOfTraceHeader(0),LengthOfTraceData(0),LengthOfTrace(0)
{};
  /// Initialize from first header of given file and file mode and set file pointer to the first trace header.
  TraceFileHandler(const char* FName, FILE_MODE _F_MODE, int& ierr);
  ~TraceFileHandler();

  /// Read header from current position and Initialize to next trace data in tracefile.
  int Next();
  /// Read header from current position and move to beginning of next header.
  int NextHeader(briefheader* bHeader);
  /// Read header from current position.
  int ReadHeader(briefheader* bHeader);
  /// Read Ntr headers from current position.
  int ReadHeaders(briefheadercoord* HeaderMem, const int Ntr);
  /// Read trace data from current position.
  int ReadTraceData(float* Trace);
  /// Read Ntr full traces (header and data) from current position.
  int ReadTraces(char* Trace, const int Ntr);
  /// Read Ntr traces (header and data) from current position. First N0 and last Ne samples are discarded.
  int ReadTraces(char* TraceMem, const int Ntr, const int N0, const int Ne);
  /// Return number of samples in current trace.
  int getNt(){return CurrentHeader.Nt;}
  /// Return sample interval of current trace (in seconds).
  float getdt(){return CurrentHeader.dt;}
  /// Return delay recording time (in s).
  float getT0(){return CurrentHeader.T0;}
  /// Return delay recording time (in s).
  UTM getsx(){return CurrentHeader.sx;}
  /// Return delay recording time (in s).
  UTM getsy(){return CurrentHeader.sy;}
  /// Return delay recording time (in s).
  UTM getgx(){return CurrentHeader.gx;}
  /// Return delay recording time (in s).
  UTM getgy(){return CurrentHeader.gy;}

  /// Return total number of traces in current file.
  unsigned long gettotnoftraces(){return TotNofTraces;}
  /// Rewind to header of first trace.
  void Rewind();
  /// Rewind N traces from current position.
  void Rewind(const int N);

  unsigned long long Seek(const int Itrace);

  /// Return current position in file.
  unsigned long long getpos(){return FInput.tellg();}

  /// appoint given header as current header
  int AppointCurrentHeader(SegYHeader Header);

// public attributes
 public:
  RawFloatHeader RawInfo;

// private methods
 private:
  /// Read SegY header from current position.
  int ReadHeader();

  void ConvertSegYHeaderBE(SegYHeader* Header);

  void ConvertSegYHeaderLE(SegYHeader* Header);

  void ConvertSegYHeaderBEsxfloat(SegYHeader* Header);

// private attributes
 private:
  std::ifstream FInput;

  briefheader CurrentHeader;

  unsigned long long LengthOfFileHeader;
  unsigned long long LengthOfTraceHeader;
  unsigned long long LengthOfTraceData;
  unsigned long long LengthOfTrace;

  unsigned long TotNofTraces;
  // number of current trace
  // unused:  int tracl;
  FILE_MODE F_MODE;
  int endianess;

  //
};

#endif
