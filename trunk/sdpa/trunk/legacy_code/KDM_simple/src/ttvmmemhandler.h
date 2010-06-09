/***************************************************************************
                          ttvmmemhandler.h  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef TTVMMEMHANDLER_H
#define TTVMMEMHANDLER_H


/**
 *@author Dirk Merten
 */
#include "filehandler/SegYHeader.h"
#include "structures/recsig.h"
#include "structures/grid3d.h"
#include "structures/grid2d.h"
#include "structures/migrationjob.h"
#include "utils/swap_bytes.h"
#include "utils/Acq_geometry.hpp"
#include "include/defs.h"

#include <fvm-pc/pc.hpp>

#include <iostream>
#include <fstream>

class TTVMMemHandler {
 public:
  TTVMMemHandler();
  TTVMMemHandler(const int _Nx, const int _Ny, const int _Nz);
  TTVMMemHandler(const MigrationJob &_Job, int _Ntid, char *);
  void Reset();
  /// Estimate total amount of memory needed
  unsigned long GetMem(const char* NameBase, const grid2D& GSrc, const grid3D& GVol, const int NodeCount, const int CoreCount);
  void InitVol(const MigrationJob& MigJob, const char* NameBase, const grid2D& GSrc, const grid3D& GVol, const int _Ntid, const int mtid, const long myPart, const long numPart, const fvmAllocHandle_t handle_TT);
  bool Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
            float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
            float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
	    SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
           SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11, const int mtid, const fvmAllocHandle_t
                         , const MigrationJob & Job
);
  bool Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
	    float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
	    float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
	    SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
	    SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
	    const int N0x, const int Npartx, const int N0y, const int Nparty, const int mtid                         , const fvmAllocHandle_t handle_TT
                         , const MigrationJob & Job
);
 ~TTVMMemHandler();


 private:
  /// 3-dim array of runtimes
  float* Src00;
  unsigned long Src00_Address;
  float* Src01;
  unsigned long Src01_Address;
  float* Src10;
  unsigned long Src10_Address;
  float* Src11;
  unsigned long Src11_Address;
  float* Rcv00;
  unsigned long Rcv00_Address;
  float* Rcv01;
  unsigned long Rcv01_Address;
  float* Rcv10;
  unsigned long Rcv10_Address;
  float* Rcv11;
  unsigned long Rcv11_Address;

  void clear();
  void TTVMMemHandleralloc(const int Nx, const int Ny, const int Nz, const int mtid);

 protected:
  /// Number of elements in each direction
  int Nx, Ny, Nz;
  int NSrfx, NSrfy;

  int Ntid;
  int PRank;
  int PSize;
  char* VMem;
  unsigned long TTLength;

  int endianess;     // <--

 private:
  static volatile int threadcom;

  int ixS, iyS, ixR, iyR;
  SegYHeader HeaderS00, HeaderS01, HeaderS10, HeaderS11;
  SegYHeader HeaderR00, HeaderR01, HeaderR10, HeaderR11;
  bool retS00, retS01, retS10, retS11;
  bool retR00, retR01, retR10, retR11;
  // Private methods
  bool ReadRT(const int& ix, const int& iy, float* buffer, unsigned long bufferaddress, SegYHeader& Header, const int mtid, const fvmAllocHandle_t handle_TT           , const MigrationJob & Job);
  bool ReadRT( const int& ix, const int& iy, float* buffer, unsigned long bufferaddress, SegYHeader& Header,
             const int N0x, const int Npartx, const int N0y, const int Nparty, const int mtid, const fvmAllocHandle_t handle_TT
                            , const MigrationJob & Job);

  /** operator = (const TTFileHandler& ) */
  TTVMMemHandler& operator = (const TTVMMemHandler& );
  /** TTFileHandler (const TTFileHandler& ) */
  TTVMMemHandler (const TTVMMemHandler& );

/// Prefix of TTT files.
  char* PreFileName;


};

#endif
