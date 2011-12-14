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


static void bandpass_impl (TraceBunch & Bunch, 
			   const float & frequ1, const float & frequ2, const float & frequ3, const float & frequ4)
{
//     if ( (frequ1 < 0.f) || (frequ2 < 0.f) || (frequ3 < 0.f) && (frequ4 < 0.f))
// 	throw std::runtime_error ("bandpass called but not configured");

    const int NTraces( Bunch.getNTB() );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = Bunch.getTrace(0)->getNt();
    const float dt = Bunch.getTrace(0)->getdtbin();

    const int Nfft = getptz(NSample);

    // initialize the band pass filter for each sample
    float * filterarray = new float[Nfft];
    for (int iarray = 0; iarray < Nfft; iarray++)
    { 
	const float frequ( iarray/(dt*Nfft) );

	filterarray [iarray] = 1.0f;
	if ( (frequ < frequ1) || (frequ >= frequ4))
	    filterarray[iarray] = 0;
	else
	    if ( (frequ >= frequ1) && (frequ < frequ2))
	    {
		const float val = (frequ - frequ1)/(frequ2-frequ1);
		const float s_val = sinf((2*val-1)*M_PI/2);
		filterarray [iarray] =  0.25f*(s_val + 1.f)*(s_val + 1.f);
	    }
	    else
		if ( (frequ >= frequ3) && (frequ < frequ4))
		{
		    const float val = (frequ4 - frequ)/(frequ4-frequ3);
		    const float s_val = sinf((2*val-1)*M_PI/2);
		    filterarray [iarray] =  0.25f*(s_val + 1.f)*(s_val+ 1.f);
		}
    }

    float * fftarray = new float[2*Nfft];

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,NSample);
	
	float* Data_ptr = (float*) Trace.getTPtr();

	memset(fftarray, 0, 2*Nfft*sizeof(float));
	for (int it = 0; it < NSample; it++)
	{
	    fftarray[2*it] = Data_ptr[it];
	}
	
	fft(-1, Nfft, fftarray);
	for (int iarray = 0; iarray < Nfft; iarray++)
	{
	    const float newim = fftarray[2*iarray]  * filterarray[iarray];
	    const float newre = -fftarray[2*iarray+1] * filterarray[iarray];
	    fftarray[2*iarray] = newre;
	    fftarray[2*iarray+1] = newim;
	}
	fft(1, Nfft, fftarray);
	
	for (int it = 0; it < NSample; it++)
	    Data_ptr[it] = fftarray[2*it]/Nfft;
    }

    delete[] fftarray;
    delete[] filterarray;
}

static void frac_impl (TraceBunch & Bunch)
{
    const int NTraces( Bunch.getNTB() );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = Bunch.getTrace(0)->getNt();
    const float dt = Bunch.getTrace(0)->getdtbin();

    const int Nfft = getptz(NSample);

    // initialize the frac array to omega for each sample
    float * filterarray = new float[Nfft];
    for (int iarray = 0; iarray < Nfft/2; iarray++)
    { 
	float omega = 2.f*2.f*M_PI*iarray/(dt*Nfft);
	filterarray[iarray] = omega;
    }
    for (int iarray = Nfft/2; iarray < Nfft; iarray++)
    { 
	filterarray[iarray] = 0.f;
    }

    float * fftarray = new float[2*Nfft];

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,NSample);
	
	float* Data_ptr = (float*) Trace.getTPtr();

	memset(fftarray, 0, 2*Nfft*sizeof(float));
	for (int it = 0; it < NSample; it++)
	{
	    fftarray[2*it] = Data_ptr[it];
	}
	
	fft(-1, Nfft, fftarray);
	for (int iarray = 0; iarray < Nfft; iarray++)
	{
	    const float newim = fftarray[2*iarray]  * filterarray[iarray];
	    const float newre = -fftarray[2*iarray+1] * filterarray[iarray];
	    fftarray[2*iarray] = newre;
	    fftarray[2*iarray+1] = newim;
	}
	fft(1, Nfft, fftarray);
	
	for (int it = 0; it < NSample; it++)
	    Data_ptr[it] = fftarray[2*it]/Nfft;
    }

    delete[] fftarray;
    delete[] filterarray;
}

static void tpow_impl (TraceBunch & Bunch, 
		       const float & tpow)
{
//     if (tpow == 0.f)
// 	throw std::runtime_error ("tpow called but not configured");

    const int NTraces( Bunch.getNTB() );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = Bunch.getTrace(0)->getNt();
    const float dt = Bunch.getTrace(0)->getdtbin();
    const float t0 = Bunch.getTrace(0)->getT0();

    // initialize the tpow filter for each sample
    float * filterarray = new float[NSample];
    for (int iarray = 0; iarray < NSample; iarray++)
    { 
	const float TT = std::max(dt, t0 + iarray * dt);
	filterarray[iarray] = pow(TT, tpow);
    }

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,NSample);
	
	float* Data_ptr = (float*) Trace.getTPtr();

	for (int iarray = 0; iarray < NSample; iarray++)
	{
	    Data_ptr[iarray] *= filterarray[iarray];
	}
	
    }

    delete[] filterarray;
}

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
    const int Nt = 1500;
    char * pTraceData = new char[Ntrace * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float))];
    for (int itrace = 0; itrace < Ntrace; itrace++)
    {
	{ // set the header values
	    char* Header_ptr = &pTraceData[itrace  * ( sizeof(int)+7*sizeof(float) + 5*Nt*sizeof(float))];
	    *((int*)(Header_ptr)) = Nt;
	    *((float*)(Header_ptr + sizeof(int))) = 0.004f;
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
		Data[isample] = ((itrace + isample) == Nt/2)?1.f:0.f;
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

//     // ------------------
//     // call to the in-place Bunch-in -- Bunch-out function
//     std::cout << "\nCall the in-place Bunch-in -- Bunch-out function.\n\n";
//     Manipulate(MyBunch);
//     // ------------------

    // ------------------
    // call to the in-place Bunch-in -- Bunch-out function
    std::cout << "\nCall the in-place Bunch-in -- Bunch-out function.\n\n";
    bandpass_impl(MyBunch, 0.f, 20.f, 30.f, 40.f);
    // ------------------

    // ------------------
    // call to the in-place Bunch-in -- Bunch-out function
    std::cout << "\nCall the in-place Bunch-in -- Bunch-out function.\n\n";
    tpow_impl(MyBunch, 5.f);
    // ------------------

    // ------------------
    // call to the in-place Bunch-in -- Bunch-out function
    std::cout << "\nCall the in-place Bunch-in -- Bunch-out function.\n\n";
    frac_impl(MyBunch);
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
