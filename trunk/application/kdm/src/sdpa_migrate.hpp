/***************************************************************************
                          sdpa_routines.hpp  -  description
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


#ifndef SDPA_MIGRATE_H
#define SDPA_MIGRATE_H

// user defined includes
#include <MigSubVol.hpp>
#include <TravTimeTab.hpp>
#include <TraceBunch.hpp>
#include <./structures/migrationjob.h>
#include <./structures/coordinates.h>
#include <./utils/taper.h>

#include <fvm-pc/pc.hpp>

// standard includes
#include <iostream>

typedef struct {
  const MigrationJob * Job;
  TraceBunch * Bunch;
  MigSubVol3D * SubVol;
  SincInterpolator * SincInt;
  int Ntid;
  int mtid;
  int NThreads;
  char * _VMem;
  fvmAllocHandle_t handle_TT;
} thread_param_t;

typedef struct {
  const MigrationJob * Job;
  TraceData * Trace;
  volatile char * TTData;
  MigSubVol3D * SubVol;
  SincInterpolator * SincInt;
  int Ntid;
  int mtid;
} thread_param2_t;

typedef struct {
  const MigrationJob * Job;
  TraceData * Trace;
  MigSubVol3D * SubVol;
  TravTimeTab * TT;
  int mtid;
} thread_param_load_t;

/**
  *@author Daniel Grünewald doxygen stale
  */

// bool SDPA_mig_and_pref(int _oid, int _pid2mig,int _bid2mig, int _pid2load, int _bid2load, int actbuf,
//                        MigrationJob &_Job, SincInterpolator ** _SincIntA,int NThreads = 1);

bool MigBunch2SubVol(const MigrationJob &Job, TraceBunch &Bunch,
                    MigSubVol3D &SubVol,int NThreads, char *, const fvmAllocHandle_t);
// bool MigBunch2SubVol2(const MigrationJob &Job, TraceBunch &Bunch,
// 		      MigSubVol3D &SubVol,SincInterpolator &SincInt);

void *MigBunch2SubVol_ST(void *_param);
void *MigBunch2SubVol_Test_ST(void *_param);
void *MigTrace2SubVol_ST(void *_param);
void *LoadTT_ST(void *_param);

bool MigTrace2SubVol(const MigrationJob &Job, TraceData &Trace,
		     TravTimeTab &TT, MigSubVol3D &SubVol, SincInterpolator &SincInt);

#endif
