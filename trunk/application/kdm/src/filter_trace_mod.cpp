#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

static void generic_filter ( void * state
                           , const we::loader::input_t & input
                           , we::loader::output_t & output
                           , void (*filter_impl) (void *, const long &)
                           )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & part_in_store (get<value::type> (input, "in"));
  const long & part (get<long> (part_in_store, "id.part"));
  const long & store (get<long> (part_in_store, "id.store"));

  MLOG (INFO, "generic_filter: part " << part << ", store " << store << ", config " << config);

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t & handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t & handle_scratch (get<long> (config, "handle.scratch"));
  const long & sizeofBunchBuffer (get<long> (config, "bunchbuffer.size"));
  const long & data_size (get<long> (config, "data.size"));
  const long size (std::min ( sizeofBunchBuffer
                            , data_size - part * sizeofBunchBuffer
                            )
                  );

  waitComm (fvmGetGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  // call filter_impl

  const long & trace_size_in_bytes (get<long> (config, "trace_detect.size_in_bytes"));
  const long num (size / trace_size_in_bytes);

  filter_impl (fvmGetShmemPtr(), num);

  // communicate back

  waitComm (fvmPutGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  put (output, "out", part_in_store);
}

// ************************************************************************* //

// ptr points to num many basis-items (e.g. traces)
// num is valid also in the case of a smaller part
static void unblank_impl (void * ptr, const long & num)
{
  char * buf ((char *) ptr);

  // in the unblank case, we know, that an item has 300 bytes
  for (long i (0); i < 300 * num; ++i)
    {
      if (buf[i] == ' ')
        {
          buf[i] = '.';
        }
    }
}

static void unblank ( void * state
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  generic_filter (state, input, output, unblank_impl);
}


// ************************************************************************* //


static void frac_impl (void * ptr, const long & num)
{
    const long NTraces( num );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = ((SegYHeader* )ptr)->ns;
    const float dt = ((SegYHeader* )ptr)->dt * 1.e-6;
    const size_t trace_size( sizeof(SegYHeader) + NSample * sizeof(float) );

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
	float* Data_ptr = (float*) ( (char*)ptr + i * trace_size + sizeof(SegYHeader) );

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
WE_MOD_INITIALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter_trace)");

  WE_REGISTER_FUN (unblank);
}
WE_MOD_INITIALIZE_END (filter_trace);

WE_MOD_FINALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter_trace)");
}
WE_MOD_FINALIZE_END (filter_trace);
