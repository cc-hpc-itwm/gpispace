/***************************************************************************
                          migrationfilehandler.h  -  description
                             -------------------
    begin                : Mon Apr 3 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2009-08-03
               Adapted to modified seisgrid3D class internal structure
 ***************************************************************************/


#ifndef MIGRATIONFILEHANDLER_H
#define MIGRATIONFILEHANDLER_H


/**
  *@author Dirk Merten
  */
#include "include/migtypes.h"
#include "include/defs.h"
#include "structures/grid3d.h"
#include "structures/Point2d.hpp"
#include "structures/migrationjob.h"
#include "structures/anglemigrationjob.h"
#include "memory/memman3d_malloc.h"
#include "utils/swap_bytes.h"
#include "utils/Acq_geometry.hpp"
#include "SegYHeader.h"
#include "SegYBHeader.h"
#include "SegYEBCHeader.h"
#include "seismicfilewriter.h"
#include "structures/BlockVolume.h"

#include <fstream>



#define MVOL_TYPE float*

class MigrationFileHandler {

// public methods
public: 
  MigrationFileHandler();
  ~MigrationFileHandler();

  void Write(const char* FName, const grid3D& G, const MVOL_TYPE M, const FILE_MODE MODE = SEGY_BIGENDIAN);
  /// Write Offset Classes N0Off -- N0Off+NOff to file FName.
  int WriteOffsetClass(const char* FName, const grid3D& G, const MVOL_TYPE M, const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE = SEGY_BIGENDIAN);
  int WriteOffsetClass(const char* FName, const grid3D& G, const MVOL_TYPE M, 
		       const int N0x, const int Nx, 
		       const int N0y, const int Ny,
		       const int N0Off, const int NOff, const int NtotOff,const FILE_MODE MODE = SEGY_BIGENDIAN);
  int WriteOffsetClassForSubVol(const char* FName, const grid3D& G, const MVOL_TYPE M, 
		       const int N0x, const int Nx, 
		       const int N0y, const int Ny,
		       const int N0Off, const int NOff, const int NtotOff,const FILE_MODE MODE = SEGY_BIGENDIAN);
  int TouchOffsetGather(const MigrationJob& Job, const char* FName, const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE, const float initval=0.0f);
  void WritePart(const char* FName, const grid3D& G, point3D<int> N0, point3D<int> Ntot, const int N0Offx, const int NtotOffx, const MVOL_TYPE M, const FILE_MODE MODE);
  void WriteCutY(const char* FName, const int& iy, const grid3D& G, const MVOL_TYPE M, const FILE_MODE MODE = SEGY_BIGENDIAN);
  void WriteMoveOut(const char* FName, const int& Nz, const float& dz, float* M, const FILE_MODE MODE = SEGY_BIGENDIAN);
  /// Touch all stencils of gathers to migration file. FileName, #gathers is taken from job, volume dimensions from BoxVolume 
  int TouchStack(const AngleMigrationJob& Job, const char* FileName, const float initval=0.0f);
  /// Touch all stencils of gathers to migration file. FileName, #gathers is taken from job, volume dimensions from BoxVolume 
  int TouchAngGather(const AngleMigrationJob& Job, const char* FileName, const float initval=0.0f);
  /// Write stencil ibx,iby of gathers to migration file. FileName, #gathers is taken from job, volume dimensions from BoxVolume 
  int WriteAngGather(float *Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume, const int ibx, const int iby);
  int WriteAngGather(float *Result, const char* FileName, const FILE_MODE FileMode, 
		     const int Nxtotal, const int Nytotal, const int Nztotal, const int Nopentotal, 
		     const int ixbeg, const int iybeg, const int izbeg, const int iopenbeg, 
		     const int Nx, const int Ny, const int Nz, const int Nopen, const bool flip = true);
  int WriteWindAngGather(float *Result, const char* FileName, const FILE_MODE FileMode, 
			 const int Nxtotal, const int Nytotal, const int Nztotal, const int Nopentotal, 
			 const int ixbeg, const int iybeg, const int izbeg, const int iopenbeg, 
			 const int Nx, const int Ny, const int Nz, const int Nopen, 
			 const int ixwindbeg, const int iywindbeg, const int izwindbeg, const int iopenwindbeg, 
			 const int Nxwind, const int Nywind, const int Nzwind, const int Nopenwind, 
			 const bool flip = true);
  /// Write part ibz_beg to ibz_end from stencil ibx,iby of gathers to migration file. FileName, #gathers is taken from job, volume dimensions from BoxVolume 
  int WriteAngGather(float *Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz_beg, const int ibz_end);
  bool CheckAngGather(const AngleMigrationJob& Job, const BlockVolume& BoxVolume, const int ibx, const int iby);

  int TouchRestartAngGather(const char* FName, const BlockVolume& BoxVolume);
  int SetRestartAngGather(const char* FileName, const BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz_beg, const int ibz_end);
  /// Return true if ibx, iby, ibz has to be calculated in restart mode, false otherwise
  bool CheckRestartAngGather(const char* FName, const BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz);
  
  /**
   * \brief Check of the restart file for existance and determination of the first entry to be calculated
   * \param FName name of the restart file
   * \param return error flag
   * 
   * The restart file is opened and read completely. The index of the first empty flag
   * that reflects a part that has not been calculated up to now is determined.
   * In the case of an error while handling the file, err is set to FATAL and 0 is returned.
   * If the restart file has not been found, err is set to WARNING and 0 is returned.
   * If the end of the file is reached and no empty flag has been found, i.e. the
   * whole volume job has been finished in a previous run, err is set to WARNING
   * and the total number of flags is returned.
   */
  unsigned long InitialCheckRestartAngGather(const char* FName, ERROR_TYPE& err);

// public attributes
 public:

// private methods
 private:
  /// Write all stencils of gathers to migration file. FileName, #gathers is taken from job, volume dimensions from BoxVolume 
  int WriteAngGather(float **Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume);


// private attributes
 private:
  int endianess;
  int index;  
  
};

#endif
