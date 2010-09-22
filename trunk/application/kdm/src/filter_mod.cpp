#include <we/loader/macros.hpp>
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
using we::loader::put;

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

  put (output, "bunch", bunch);
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

  put (output, "bunch", bunch);
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
	    Data_ptr[it] = c;
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
	  if (Data_ptr[it] > t)
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

WE_MOD_INITIALIZE_START (filter);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter)");

  WE_REGISTER_FUN (shrink);
  WE_REGISTER_FUN (clip);
  WE_REGISTER_FUN (trap);
}
WE_MOD_INITIALIZE_END (filter);

WE_MOD_FINALIZE_START (filter);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter)");
}
WE_MOD_FINALIZE_END (filter);
