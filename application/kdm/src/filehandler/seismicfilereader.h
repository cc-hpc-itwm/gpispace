/***************************************************************************
                          seismicfilereader.h  -  description

   Handler for (velocity-) model files.

                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef SEISMICFILEREADER_H
#define SEISMICFILEREADER_H


/**
  *@author Dirk Merten
  */

#include "include/migtypes.h"
#include "include/defs.h"
#include "utils/swap_bytes.h"
#include "SegYEBCHeader.h"
#include "SegYBHeader.h"
#include "SegYHeader.h"



#include <fstream>

class SeismicFileReader {

// public methods
 public: 
    SeismicFileReader();
    SeismicFileReader(const char* FName, FILE_MODE _F_MODE, const int _Nt, int& ierr);// = SEGY_BIGENDIAN);
    SeismicFileReader(const char* FName, FILE_MODE _F_MODE, const int _Nt, const int _stripe, const int _offset, int& ierr);// = SEGY_BIGENDIAN);
    ~SeismicFileReader();

    void Init(const char* FName, FILE_MODE _F_MODE, const int _Nt, int& ierr);

    int Open(const char* FName, FILE_MODE _F_MODE);
    int Close();

    /** Read the next N2 z-stencils/traces of length N1 to Mem */
    int Read(SegYHeader* Header, float* Mem);
    int Read(SegYHeader* Header);

    void GoTo(long trace_number);
    std::ifstream::pos_type GetDataSize();

    void SetNt(int _Nt);

// public attributes
 public:

// private methods
 private:

// private attributes
 private:
    int endianess;
    int Nt;
    int stripe;
    int offset;
    std::ifstream FInput;
    FILE_MODE F_MODE;

    std::ifstream::pos_type file_offset, bytes_per_trace;
    std::ifstream::pos_type file_size;

    
};

#endif
