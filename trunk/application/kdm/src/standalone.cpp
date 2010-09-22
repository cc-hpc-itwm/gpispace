/***************************************************************************
                          main_smooth.cpp  -  description

    Smoothing a velocity field by 3-step average of slowness.
    Used for stat-oil velocity field vpMod
                             -------------------
    begin                : Fri Feb 17 16:48:00 CET 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "structures/migrationjob.h"
#include "TraceData.hpp"
#include "TraceBunch.hpp"

#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>


// ---------------------------------------------
// dummy in-place Bunch-in -- Bunch-out function
void Manipulate( TraceBunch& Bunch )
{
    const int NTraces( Bunch.getNTB() );

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,Bunch.getTrace(i)->getNt());

	float* Data_ptr = (float*) Trace.getTPtr();
	const int NSample(  Trace.getNt() );

	for (int it = 0; it < NSample; it++)
	{ // shrink the samples to  a half ...
	    Data_ptr[it/2] = Data_ptr[it];
	}
	for (int it = NSample/2; it < NSample; it++)
	{ // and set the secopnd half to 0
	    Data_ptr[it] = 0.f;
	}
    }
}
// ---------------------------------------------

int main(int argc, char *argv[])
{
    std::cout << "Lets do something.\n";

    // initialize a bunch of traces
    const int Ntrace = 10;
    const int Nt = 15;
    char * pTraceData = new char[Ntrace * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float))];
    for (int itrace = 0; itrace < Ntrace; itrace++)
    {
	{ // set the header values
	    char* Header_ptr = &pTraceData[itrace  * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float))];
	    *((int*)(Header_ptr)) = 4000;
	    *((float*)(Header_ptr + sizeof(int))) = 0.f;
	    *((float*)(Header_ptr + sizeof(int) + 1*sizeof(float))) = Nt * 0.004f;
	    *((float*)(Header_ptr + sizeof(int) + 2*sizeof(float))) = 0.f;
	    *((float*)(Header_ptr + sizeof(int) + 3*sizeof(float))) = 0.f;
	    *((float*)(Header_ptr + sizeof(int) + 4*sizeof(float))) = 0.f;
	    *((float*)(Header_ptr + sizeof(int) + 5*sizeof(float))) = 0.f;
	    *((float*)(Header_ptr + sizeof(int) + 6*sizeof(float))) = 0.f;
	}

	{ // set the data
	    float* Data = (float*) (&pTraceData[itrace  * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float)) + sizeof(int)+7*sizeof(float)]);
	    for (int isample = 0; isample < Nt; isample++)
	    {
		Data[isample] = itrace + isample;
	    }
	}
    }
    
    // initialize the job description
    int oid(1);
    int pid(1);
    int bid(1);

    MigrationJob Job;
    Job.dOffVol = 10.f;
    Job.d_offset = 10.f;
    Job.Off0Vol = 0.f;
    Job.first_offset = 0.f;
    Job.NOffVol = 1;
    Job.n_offset = 1;
    Job.traceNt = Nt;
    Job.tracesperbunch = 10;
    Job.n_xlines_CDP = 5;
    Job.n_inlines_CDP = 2;

    // generate my dummy Bunch
    TraceBunch MyBunch(pTraceData, oid, pid, bid, Job);

    // ------------------
    // call to the in-place Bunch-in -- Bunch-out function
    std::cout << "\nCall the in-place Bunch-in -- Bunch-out function.\n\n";
    Manipulate(MyBunch);
    // ------------------


    // dump the result
    std::cout << "dump the result.\n";
    for (int itrace = 0; itrace < Ntrace; itrace++)
    {
	float* Data = (float*) (&pTraceData[itrace  * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float)) + sizeof(int)+7*sizeof(float)]);
	for (int isample = 0; isample < Nt; isample++)
	{
	    std::cout << itrace << " " << isample << " : " << Data[isample] << std::endl;
	}
    }

    return EXIT_SUCCESS;
}
