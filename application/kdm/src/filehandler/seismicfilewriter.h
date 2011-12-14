/***************************************************************************
                          seismicfilewriter.h  -  description

   Handler for (velocity-) model files.

                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef SEISMICFILEWRITER_H
#define SEISMICFILEWRITER_H


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

class SeismicFileWriter {

// public methods
 public: 
    SeismicFileWriter();
    SeismicFileWriter(const char* FName, FILE_MODE _F_MODE, const short _Nt, const short _dt, int& ierr);// = SEGY_BIGENDIAN);
    ~SeismicFileWriter();

    int Open(const char* FName, FILE_MODE _F_MODE, const short _Nt, const short _dt);
    int Close();

    /** Read the next N2 z-stencils/traces of length N1 to Mem */
    int Write(const SegYHeader* Header, const float* Mem);

    /** jump to z-stencil/trace number "trace_number" */
    void GoTo(long trace_number);

// public attributes
 public:

// private methods
 private:

// private attributes
 private:
    int endianess;
    std::ofstream FOutput;
    FILE_MODE F_MODE;
    short Nt;

    /** information necessary for GoTo */
    std::ofstream::pos_type file_offset, bytes_per_trace;

    
};

#endif
