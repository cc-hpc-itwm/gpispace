/***************************************************************************
                          TraceBunch.hpp  -  description
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


#ifndef TRACEBUNCH_H
#define TRACEBUNCH_H

// user defined includes
#include <TraceData.hpp>
#include <tracemem.h>
#include <./structures/coordinates.h>

// standard includes
#include <vector>

typedef struct {
  volatile char       * pTraceData;
  int          SizeOfOneTD;
  int          Nt;
  TraceData ** pTraceVec;
  MigrationJob * Job;
  int oid; 	// offset id 	range 1..N
  int pid;	// package id	range 1..
  int bid;	// bunch id 
  int Nbip;     // Number of bunches in package
  int Ntib;	// Number of traces in bunch
  int Ntid;     // Total number of threads
  int mtid;     // thread id
} thread_param_tb_t;

/**
  *@author Daniel Grünewald doxygen stale
  */

class TraceBunch {

// public methods
 public:
    TraceBunch(int _oid, int _pid, int _bid, MigrationJob &_Job);
    TraceBunch(volatile char * _pTraceData, int _oid, int _pid, int _bid, MigrationJob &Job);
    ~TraceBunch();  

    TraceData* operator[] (const int i);

    TraceData* getTrace(const int i);

    // get the offset id       
    int getOID() 	{return oid;};

    // get the package id;
    int getPID()	{return pid;};

    // get the bunch id;
    int getBID()	{return bid;};

    // get the number of traces in bunch
    int getNTB() 	{return Ntib;};

    // io routines
    bool LoadFromDisk_CO(MigrationJob& Job);
    bool LoadFromDisk_CO_MT(MigrationJob& Job); // Multi threaded version

    volatile char * getDataPtr();

// public attributes
 public:

// private methods
 private:

    TraceBunch(const TraceBunch& T);
    
    TraceBunch& operator = (const TraceBunch& T);

// private attributes
 private:
  int oid; 	// offset id
  int pid;	// package id
  int bid;	// bunch id
  int Nbip;     // Number of bunches in package
  int Ntib;	// Number of traces in bunch
  int Nt;

  //std::vector <TraceMem *> pTraceVec;
  volatile char * pTraceData;
  TraceData ** pTraceVec;
 
  int TraceBunchMemRqst;

  int SizeOfOneTD;
};

int getSizeOfOneTD(const int &_Nt);
int getSizeofTD(const MigrationJob&);
int NTrace_in_oid(int oid,const MigrationJob &Job);
int Npid_in_oid(int oid,const MigrationJob &Job);
int Nbid_in_pid(int oid, int pid, const MigrationJob &Job);
int NTrace_in_bid(int oid, int pid, int bid, const MigrationJob &Job);
int getfile_offset(int _oid, int _pid,int _bid,int _tid,const MigrationJob &_Job);

#endif 
 
