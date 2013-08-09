#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <limits>
#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#include <we2/type/value/peek.hpp>
#include <we2/type/value/show.hpp>

namespace
{
  template<typename R>
    R peek (const pnet::type::value::value_type& x, const std::string& key)
  {
    return boost::get<R> (*pnet::type::value::peek (key, x));
  }
}

// ************************************************************************* //

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <stdexcept>

//! \todo Is this really supposed to be included? Its in example/exec/
#include <process/process.hpp>

// ************************************************************************* //

static void
generic_filter ( void * state
	       , const we::loader::input_t & input
	       , we::loader::output_t & output
	       , boost::function <void (void *, const long &)> filter_impl
	       )
{
  const pnet::type::value::value_type& config (input.value ("config"));
  const pnet::type::value::value_type& part_in_store (input.value ("in"));
  const long & part (peek<const long&> (part_in_store, "id.part"));
  const long & store (peek<const long&> (part_in_store, "id.store"));

  MLOG (INFO, "generic_filter: part " << part << ", store " << store << ", config " << pnet::type::value::show (config));

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t handle_data (peek<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (peek<long> (config, "handle.scratch"));
  const long& sizeofBunchBuffer (peek<const long&> (config, "bunchbuffer.size"));
  const long& data_size (peek<const long&> (config, "data.size"));
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

  const long& trace_size_in_bytes (peek<const long&> (config, "trace_detect.size_in_bytes"));
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

  output.bind ("out", part_in_store);
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

static void clip_impl (void * ptr, const long & num
		       , const float & c)
{
  if (c == -1.f)
    throw std::runtime_error ("clip called but not configured");

  const long NTraces( num );
  if (NTraces <= 0)
      return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
  const int NSample = ((SegYHeader* )ptr)->ns;
  const size_t trace_size( sizeof(SegYHeader) + NSample * sizeof(float) );

    for(int i=0; i< NTraces; i++)
    {
	float* Data_ptr = (float*) ( (char*)ptr + i * trace_size + sizeof(SegYHeader) );

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
  const float c (peek<double> (input.value ("config"), "param.clip.c"));

  generic_filter (state, input, output, boost::bind (clip_impl, _1, _2, c));
}

// ************************************************************************* //

static void trap_impl (void * ptr, const long & num
		       , const float & t)
{
  if (t == -1.f)
    throw std::runtime_error ("trap called but not configured");


    const long NTraces( num );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = ((SegYHeader* )ptr)->ns;
    const size_t trace_size( sizeof(SegYHeader) + NSample * sizeof(float) );

    for(int i=0; i< NTraces; i++)
    {
	float* Data_ptr = (float*) ( (char*)ptr + i * trace_size + sizeof(SegYHeader) );

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
  const float t (peek<long> (input.value ("config"), "param.trap.t"));

  generic_filter (state, input, output, boost::bind (trap_impl, _1, _2, t));
}

// ************************************************************************* //

static void bandpass_impl ( void * ptr, const long & num
			  , const float & frequ1
			  , const float & frequ2
			  , const float & frequ3
			  , const float & frequ4
			  )
{
  if ( (frequ1 < 0.f) || (frequ2 < 0.f) || (frequ3 < 0.f) || (frequ4 < 0.f))
    throw std::runtime_error ("bandpass called but not configured");

    const long NTraces( num );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = ((SegYHeader* )ptr)->ns;
    const float dt = ((SegYHeader* )ptr)->dt * 1.e-6;
    const size_t trace_size( sizeof(SegYHeader) + NSample * sizeof(float) );

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
	float* Data_ptr = (float*) ( (char*)ptr + i * trace_size + sizeof(SegYHeader) );

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

static void bandpass ( void * state
		     , const we::loader::input_t & input
		     , we::loader::output_t & output
		     )
{
  const float frequ1 (peek<long> (input.value ("config"), "param.bandpass.frequ1"));
  const float frequ2 (peek<long> (input.value ("config"), "param.bandpass.frequ2"));
  const float frequ3 (peek<long> (input.value ("config"), "param.bandpass.frequ3"));
  const float frequ4 (peek<long> (input.value ("config"), "param.bandpass.frequ4"));

  generic_filter (state, input, output
		 , boost::bind ( bandpass_impl
			       , _1
			       , _2
			       , frequ1
			       , frequ2
			       , frequ3
			       , frequ4
			       )
		 );
}

// ************************************************************************* //


static void frac_impl (void * ptr, const long & num)
{
  MLOG(INFO, "frac called");

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

static void tpow_impl (void * ptr, const long & num,
		       const float & tpow)
{
//     if (tpow == 0.f)
// 	throw std::runtime_error ("tpow called but not configured");

    const long NTraces( num );
    if (NTraces <= 0)
	return;

    // get the time sampling from the first trace
    // assuming that all traces have the same sampling
    const int NSample = ((SegYHeader* )ptr)->ns;
    const float dt = ((SegYHeader* )ptr)->dt * 1.e-6;
    const float t0 = ((SegYHeader* )ptr)->delrt;
    const size_t trace_size( sizeof(SegYHeader) + NSample * sizeof(float) );

    // initialize the tpow filter for each sample
    float * filterarray = new float[NSample];
    for (int iarray = 0; iarray < NSample; iarray++)
    {
	const float TT = std::max(dt, t0 + iarray * dt);
	filterarray[iarray] = pow(TT, tpow);
    }

    for(int i=0; i< NTraces; i++)
    {
	float* Data_ptr = (float*) ( (char*)ptr + i * trace_size + sizeof(SegYHeader) );

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
  const float t (peek<double> (input.value ("config"), "param.tpow.tpow"));

  generic_filter (state, input, output, boost::bind (tpow_impl, _1, _2, t));
}

// ************************************************************************* //

static void execW ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const pnet::type::value::value_type& config (input.value ("config"));
  const pnet::type::value::value_type& part_in_store (input.value ("in"));
  const long& part (peek<const long&> (part_in_store, "id.part"));
  const long& store (peek<const long&> (part_in_store, "id.store"));

  MLOG (INFO, "generic_filter: part " << part << ", store " << store << ", config " << pnet::type::value::show (config));

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t handle_data (peek<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (peek<long> (config, "handle.scratch"));
  const long& sizeofBunchBuffer (peek<const long&> (config, "bunchbuffer.size"));
  const long& data_size (peek<const long&> (config, "data.size"));
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

  // const long & trace_size_in_bytes (get<long> (config, "trace_detect.size_in_bytes"));
  //  const long num (size / trace_size_in_bytes);

  // call exec

  const std::string& cmd (peek<const std::string&> (config, "exec.su"));

  MLOG (INFO, "call exec for '" << cmd << "' with size " << size);

  void * buf (fvmGetShmemPtr());

  std::size_t written (process::execute ( cmd
                                        , buf
                                        , (std::size_t)size
                                        , buf
                                        , (std::size_t)size
                                        )
                      );

  if (written != (std::size_t)size)
    {
      throw std::runtime_error ("written != size");
    }

  // communicate back

  waitComm (fvmPutGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  output.bind ("out", part_in_store);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter_trace)");

  WE_REGISTER_FUN (unblank);
  WE_REGISTER_FUN (frac);
  WE_REGISTER_FUN (clip);
  WE_REGISTER_FUN (trap);
  WE_REGISTER_FUN (tpow);
  WE_REGISTER_FUN (bandpass);
  WE_REGISTER_FUN (execW);
}
WE_MOD_INITIALIZE_END (filter_trace);

WE_MOD_FINALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter_trace)");
}
WE_MOD_FINALIZE_END (filter_trace);
