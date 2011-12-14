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
#include <tracemem.h>
#include <./structures/coordinates.h>

// standard includes
#include <vector>

typedef struct {
  TraceMem * pTraceVec;
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
    TraceBunch(TraceMem& Trace);
    TraceBunch(int _oid, int _pid, int _bid, int _Ntib);
    ~TraceBunch();  

    TraceMem operator[] (const int i);

    TraceMem* getTrace(const int i);

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

  //std::vector <TraceMem *> pTraceVec;
  char * pTraceData;

};

#endif 
 
