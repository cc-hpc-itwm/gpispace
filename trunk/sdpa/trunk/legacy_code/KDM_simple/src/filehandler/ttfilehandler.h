/***************************************************************************
                          ttfilehandler.h  -  description

    Handler for travel time table files.

                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef TTFILEHANDLER_H
#define TTFILEHANDLER_H


/**
 *@author Dirk Merten
 */
#include "include/defs.h"
#include "structures/migrationjob.h"
#include "structures/tracingjob.h"
#include "structures/grid3d.h"
#include "structures/grid2d.h"
#include "utils/swap_bytes.h"
#include "filehandler/checkreadtracingjob.h"
#include "filehandler/SegYHeader.h"
#include "SeismicRayTracer/receivergrid.h"

#include <iostream>
#include <fstream>

//

#if (defined(__ALTIVEC__) || defined(__SPU__))
#include </opt/cell/sdk/usr/include/malloc_align.h>
#include </opt/cell/sdk/usr/include/free_align.h>
#endif

class TTFileHandler {

// public methods
 public:
  TTFileHandler();
  TTFileHandler(const int _Nx, const int _Ny, const int _Nz);
  void InitVol(const int _Nx, const int _Ny, const int _Nz);
  bool Analyse(const char* NameBase, grid3D& GVol, grid2D& GSrc);
  bool Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
            float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
            float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
	    SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
	    SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11);
  bool Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
	    float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
	    float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
	    SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
	    SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
	    const int N0x, const int Npartx, const int N0y, const int Nparty);
  bool Init(const MigrationJob& Job, const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
	    float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
	    float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
	    SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
	    SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
	    const int N0x, const int Npartx, const int N0y, const int Nparty);
 ~TTFileHandler();

/// Set directory and additional prefix; DirName and/or PreFix = NULL are treated as empty string; return value != 0 if combined PreFileName is too long
  int SetDirectory(const char* DirName, const char* PreFix);
/// Write XML TTT master file with parameters given in Job; return value = 0 on success.
  int WriteXML(const TracingJob& Job);
  void WriteRcvSegY(const point3D<float>& Srcpos, const ReceiverGrid& RcvGrid, Acq_geometry<float>& Geom, const int& SrcX, const int& SrcY);

// public attributes
 public:

// private methods
 private:
  void clear();
  void TTFileHandleralloc(const int Nx, const int Ny, const int Nz);
  bool ReadRT(const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header);
  bool ReadRT(const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header,
	      const int N0x, const int Npartx, const int N0y, const int Nparty);
  bool ReadRT(const MigrationJob & MigJob, const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header,
	      const int N0x, const int Npartx, const int N0y, const int Nparty);

  /** operator = (const TTFileHandler& ) */
  TTFileHandler& operator = (const TTFileHandler& );
  /** TTFileHandler (const TTFileHandler& ) */
  TTFileHandler (const TTFileHandler& );

// private attributes
 private:
  /// 3-dim array of runtimes
  float* Src00;
  float* Src01;
  float* Src10;
  float* Src11;
  float* Rcv00;
  float* Rcv01;
  float* Rcv10;
  float* Rcv11;

  /// Number of elements in each direction
  int Nx, Ny, Nz;

  int ixS, iyS, ixR, iyR;
  SegYHeader HeaderS00, HeaderS01, HeaderS10, HeaderS11;
  SegYHeader HeaderR00, HeaderR01, HeaderR10, HeaderR11;
  bool retS00, retS01, retS10, retS11;
  bool retR00, retR01, retR10, retR11;
  int endianess;

/// Prefix of TTT files.
  char* PreFileName;

  //  
};

#endif
