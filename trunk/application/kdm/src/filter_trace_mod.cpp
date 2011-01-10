#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <limits>
#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <stdexcept>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <boost/thread.hpp>

namespace process
{
  namespace detail
  {
    /* ********************************************************************* */

    inline void do_error (const std::string & msg)
    {
      std::ostringstream sstr;

      sstr << msg << ": " << strerror (errno);

      MLOG (ERROR, sstr.str());

      throw std::runtime_error (sstr.str());
    }

    template<typename T>
    inline void do_error (const std::string & msg, T x)
    {
      std::ostringstream sstr;

      sstr << msg << ": " << x;

      do_error (sstr.str());
    }

    /* ********************************************************************* */

    inline void do_close (int * fd)
    {
      if (close (*fd) < 0)
        {
          do_error ("close failed");
        }

      *fd = -1;
    }

    /* ********************************************************************* */

    enum {RD = 0, WR = 1};

    /* ********************************************************************* */

    inline void prepare_parent_pipes (int in[2], int out[2])
    {
      do_close (in + RD);
      do_close (out + WR);
    }

    /* ********************************************************************* */

    inline void prepare_child_pipes (int in[2], int out[2])
    {
      do_close (in + WR);
      do_close (out + RD);

      if (in[RD] != STDIN_FILENO)
        {
          if (dup2 (in[RD], STDIN_FILENO) != STDIN_FILENO)
            {
              do_error ("dup to stdin failed");
            }

          do_close (in + RD);
        }

      if (out[WR] != STDOUT_FILENO)
        {
          if (dup2 (out[WR], STDOUT_FILENO) != STDOUT_FILENO)
            {
              do_error ("dup to stdout failed");
            }

          do_close (out + WR);
        }
    }
  } // namespace detail

  /* *********************************************************************** */

  namespace thread
  {
    /* ********************************************************************* */

    inline void reader ( int * fd
                       , void * output
                       , int & bytes_read
                       , const int & block_size = PIPE_BUF
                       )
    {
      DLOG (TRACE, "start thread read");

      char * buf (static_cast<char *>(output));

      while (*fd != -1)
        {
          DLOG (TRACE, "try to read " << block_size << " bytes");

          const int r (read (*fd, buf, block_size));

          if (r < 0)
            {
              detail::do_error ("read failed");
            }
          else if (r == 0)
            {
              DLOG (TRACE, "read pipe closed");

              *fd = -1;
            }
          else
            {
              buf += r;
              bytes_read += r;

              DLOG (TRACE, "read " << r << " bytes, sum " << bytes_read);
            }
        }

      DLOG (TRACE, "done thread read");
    }

    /* ********************************************************************* */

    inline void writer ( int * fd
                       , const void * input
                       , int & bytes_left
                       )
    {
      DLOG (TRACE, "start thread write");

      char * buf (static_cast<char *> (const_cast<void *> (input)));

      while (*fd != -1 && bytes_left > 0)
        {
          DLOG (TRACE, "try to write " << bytes_left << " bytes");

          const int w (write (*fd, buf, bytes_left));

          if (w < 0)
            {
              detail::do_error ("write failed");
            }
          else if (w == 0)
            {
              DLOG (TRACE, "write pipe closed");

              *fd = -1;
            }
          else
            {
              buf += w;
              bytes_left -= w;

              DLOG (TRACE, "written " << w << " bytes, left " << bytes_left);
            }
        }

      detail::do_close (fd);

      DLOG (TRACE, "done thread write");
    }
  } // namespace thread

  /* *********************************************************************** */

  std::size_t execute ( std::string const & command
                      , const void * input, const std::size_t input_size
                      , void * output
                      )
  {
    pid_t pid;

    int in[2], out[2];

    if ((pipe (in) < 0) || (pipe (out) < 0))
      {
        detail::do_error ("pipe failed");
      }

    DLOG (TRACE, "threads running");

    if ((pid = fork()) < 0)
      {
        detail::do_error ("fork failed");
      }
    else if (pid == pid_t (0))
      {
        // child
        DLOG (TRACE, "prepare pipes");

        detail::prepare_child_pipes (in, out);

        DLOG (TRACE, "prepare commandline");

        std::vector<std::string> cmdline;
        fhg::log::split (command, " ", std::back_inserter (cmdline));

        char ** av = new char*[cmdline.size()+1];
        av[cmdline.size()] = (char*)(NULL);

        std::size_t idx (0);
        for ( std::vector<std::string>::const_iterator it (cmdline.begin())
            ; it != cmdline.end()
            ; ++it, ++idx
            )
          {
            av[idx] = new char[it->size()+1];
            memcpy(av[idx], it->c_str(), it->size());
            av[idx][it->size()] = (char)0;
          }

        MLOG (INFO, "run command: " << command);

        if (execvp(av[0], av) < 0)
          {
            detail::do_error ("exec failed");

            _exit (-errno);
          }
      }
    else
      {
        // parent
        DLOG (TRACE, "prepare pipes");

        detail::prepare_parent_pipes (in, out);

        DLOG (TRACE, "start threads");

        int bytes_read (0);
        int bytes_left (input_size);

        boost::thread thread_writer
          ( thread::writer
          , in + detail::WR
          , input
          , boost::ref (bytes_left)
          );

        boost::thread thread_reader
          ( thread::reader
          , out + detail::RD
          , output
          , boost::ref (bytes_read)
          );

        DLOG (TRACE, "await child");

        int status (0);

        waitpid (pid, &status, 0);

        if (WIFEXITED (status))
          {
            const int ec (WEXITSTATUS(status));

            if (ec != 0)
              {
                detail::do_error ("child exited with exitcode", ec);
              }
          }
        else if (WIFSIGNALED (status))
          {
            const int ec (WTERMSIG(status));

            detail::do_error ("child exited due to signal", ec);
          }
        else
          {
            detail::do_error ("strange child status", status);
          }

        DLOG (TRACE, "join threads");

        thread_writer.join();
        thread_reader.join();

        MLOG (INFO, "finished command: " << command);

        return bytes_read;
      }

    return -1;
  }
}

// ************************************************************************* //

static void
generic_filter ( void * state
	       , const we::loader::input_t & input
	       , we::loader::output_t & output
	       , boost::function <void (void *, const long &)> filter_impl
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
  const float & c (get<double> (input, "config", "param.clip.c"));

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
  const float & t (get<long> (input, "config", "param.trap.t"));

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
  const float & frequ1 (get<long> (input, "config", "param.bandpass.frequ1"));
  const float & frequ2 (get<long> (input, "config", "param.bandpass.frequ2"));
  const float & frequ3 (get<long> (input, "config", "param.bandpass.frequ3"));
  const float & frequ4 (get<long> (input, "config", "param.bandpass.frequ4"));

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
  const float & t (get<double> (input, "config", "param.tpow.tpow"));

  generic_filter (state, input, output, boost::bind (tpow_impl, _1, _2, t));
}

// ************************************************************************* //

static void execW ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
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

  // const long & trace_size_in_bytes (get<long> (config, "trace_detect.size_in_bytes"));
  //  const long num (size / trace_size_in_bytes);

  // call exec

  const std::string & cmd (get<std::string> (config, "exec.su"));

  MLOG (INFO, "call exec for '" << cmd << "' with size " << size);

  void * buf (fvmGetShmemPtr());

  std::size_t written (process::execute (cmd, buf, (std::size_t)size, buf));

  if (written != size)
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

  put (output, "out", part_in_store);
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
