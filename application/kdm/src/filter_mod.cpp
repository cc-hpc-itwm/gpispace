#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

// ************************************************************************* //

// generic wrapper, no need to modify this
static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

using we::loader::get;

static void get_Job (const value::type & config, MigrationJob & Job)
{
  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
  const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));

  waitComm (fvmGetGlobalData ( handle_Job
                             , fvmGetRank() * sizeofJob()
                             , sizeofJob()
                             , 0
                             , scratch_Job
                             )
           );

  memcpy (&Job, fvmGetShmemPtr(), sizeofJob());
}

static void generic_filter ( void *
			   , const we::loader::input_t & input
			   , we::loader::output_t & output
			   , void (*filter) (TraceBunch &)
			   )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + get<long> (bunch, "volume.offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * migbuf (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(migbuf,oid,1,bid,Job);

  filter (Bunch);

  output.bind ("bunch", bunch);
}

static void
generic_filter_with_float ( void *
			  , const we::loader::input_t & input
			  , we::loader::output_t & output
			  , void (*filter) (TraceBunch &, const float &)
			  , const std::string & field
			  )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + get<long> (bunch, "volume.offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * migbuf (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(migbuf,oid,1,bid,Job);

  const float f (get<double> (input, "config", field));

  filter (Bunch, f);

  output.bind ("bunch", bunch);
}

// ************************************************************************* //

// the implementation of shrink
static void shrink_impl (TraceBunch & Bunch)
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

// the wrapper, uses the generic wrapper
static void shrink ( void * state
		   , const we::loader::input_t & input
		   , we::loader::output_t & output
		   )
{
  generic_filter (state, input, output, shrink_impl);
}

// ************************************************************************* //

static void clip_impl (TraceBunch & Bunch, const float & c)
{
  if (c == -1.f)
    throw std::runtime_error ("clip called but not configured");

    const int NTraces( Bunch.getNTB() );

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,Bunch.getTrace(i)->getNt());

	float* Data_ptr = (float*) Trace.getTPtr();
	const int NSample(  Trace.getNt() );

	for (int it = 0; it < NSample; it++)
	{
	  if (Data_ptr[it] > c)
	    {
	      Data_ptr[it] = c;
	    }
	  else if (Data_ptr[it] < -c)
	    {
	      Data_ptr[it] = -c;
	    }
	}
   }
}

static void clip ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  generic_filter_with_float (state, input, output, clip_impl, "filter.clip");
}

// ************************************************************************* //

static void trap_impl (TraceBunch & Bunch, const float & t)
{
  if (t == -1.f)
    throw std::runtime_error ("trap called but not configured");


    const int NTraces( Bunch.getNTB() );

    for(int i=0; i< NTraces; i++)
    {
	volatile char * pTraceData = Bunch.getTrace(i)->getDataPtr();
	TraceData Trace(pTraceData,Bunch.getTrace(i)->getNt());

	float* Data_ptr = (float*) Trace.getTPtr();
	const int NSample(  Trace.getNt() );

	for (int it = 0; it < NSample; it++)
	{
	  if (Data_ptr[it] > t || Data_ptr[it] < -t)
	    Data_ptr[it] = 0.f;
	}
   }
}

static void trap ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  generic_filter_with_float (state, input, output, trap_impl, "filter.trap");
}

// ************************************************************************* //

static void bandpass_impl ( TraceBunch & Bunch
			  , const float & frequ1
			  , const float & frequ2
			  , const float & frequ3
			  , const float & frequ4
			  )
{
  if ( (frequ1 < 0.f) || (frequ2 < 0.f) || (frequ3 < 0.f) || (frequ4 < 0.f))
    throw std::runtime_error ("bandpass called but not configured");

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
	    const float newre = fftarray[2*iarray]  * filterarray[iarray];
	    const float newim = fftarray[2*iarray+1] * filterarray[iarray];
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

static void
bandpass ( void *
	 , const we::loader::input_t & input
	 , we::loader::output_t & output
	 )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + get<long> (bunch, "volume.offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * migbuf (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(migbuf,oid,1,bid,Job);

  bandpass_impl (Bunch, Job.frequ1, Job.frequ2, Job.frequ3, Job.frequ4);

  output.bind ("bunch", bunch);
}

// ************************************************************************* //

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

static void frac ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  generic_filter (state, input, output, frac_impl);
}

// ************************************************************************* //

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

static void tpow ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  generic_filter_with_float (state, input, output, tpow_impl, "filter.tpow");
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (filter);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter)");

  WE_REGISTER_FUN (shrink);
  WE_REGISTER_FUN (clip);
  WE_REGISTER_FUN (trap);
  WE_REGISTER_FUN (bandpass);
  WE_REGISTER_FUN (frac);
  WE_REGISTER_FUN (tpow);
}
WE_MOD_INITIALIZE_END (filter);

WE_MOD_FINALIZE_START (filter);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter)");
}
WE_MOD_FINALIZE_END (filter);
