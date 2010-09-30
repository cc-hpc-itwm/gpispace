#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

#include <boost/function.hpp>

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/signal.h>

static int terminate (pid_t pid)
{
  kill (pid, SIGTERM);
  sleep (1);
  kill (pid, SIGKILL);
  return 0;
}

static long exec_impl ( std::string const & command
		      , const void *input
                      , unsigned int size_in
		      , void *output
                      , unsigned int size_out
		      )
{
  int in_pipe[2];
  int out_pipe[2];
  int err_pipe[2];
  long ec (0);
  
  if (pipe(in_pipe) < 0)
  {
    LOG(ERROR, "opening input pipe failed: " << strerror(errno));
    throw std::runtime_error ("pipe() failed: " + std::string (strerror(errno)));
  }
  fcntl (in_pipe[0], F_SETFD, O_NONBLOCK);
  fcntl (in_pipe[1], F_SETFD, O_NONBLOCK);
  
  if (pipe(out_pipe) < 0)
  {
    LOG(ERROR, "opening output pipe failed: " << strerror(errno));
    throw std::runtime_error ("pipe() failed: " + std::string (strerror(errno)));
  }
  fcntl (out_pipe[0], F_SETFD, O_NONBLOCK);
  fcntl (out_pipe[1], F_SETFD, O_NONBLOCK);
  
  if (pipe(err_pipe) < 0)
  {
    LOG(ERROR, "opening error pipe failed: " << strerror(errno));
    throw std::runtime_error ("pipe() failed: " + std::string (strerror(errno)));
  }
  fcntl (err_pipe[0], F_SETFD, O_NONBLOCK);
  fcntl (err_pipe[1], F_SETFD, O_NONBLOCK);

  pid_t pid_child = fork();
  if (pid_child == 0)
  {
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
    
    std::stringstream sstr_cmd;
    for(size_t k=0; k<idx; k++)
      sstr_cmd << av[idx];

    for (int i = 0; i < 1024; ++i)
    {
      if (i != in_pipe[0] && i != out_pipe[1] && i != err_pipe[1])
	close (i);
    }
    
    close (in_pipe[1]);
    close (out_pipe[0]);
    close (err_pipe[0]);

    dup2(in_pipe[0], 0);
    dup2(out_pipe[1], 1);
    dup2(out_pipe[1], 2);
    
    //    if ( execve( av[0], av, environ) < 0 )
    if ( execvp( av[0], av ) < 0 )
    {
      throw std::runtime_error( std::string("could not exec command line ") + sstr_cmd.str() + std::string(strerror(errno)));
    }
  }
  else if (pid_child > 0)
  {
    LOG(INFO, "child running with pid: " << pid_child);
    close (in_pipe[0]);
    close (out_pipe[1]);
    close (err_pipe[1]);

    int in_to_child = in_pipe[1];
    int out_from_child = out_pipe[0];
    int err_from_child = err_pipe[0];
    
    LOG(INFO, "initiating read/write loop");

    const char *src = (const char*)(input);
    char *dst = (char*)(output);

    std::size_t bytes_wr (0);
    std::size_t bytes_rd (0);

    std::stringstream err_str;
    while (true)
    {
      int nfds = 0, r = 0;
      fd_set rd, wr, er;
      FD_ZERO (&rd);
      FD_ZERO (&wr);
      FD_ZERO (&er);

      if (err_from_child >= 0)
      {
	FD_SET (err_from_child, &rd);
	FD_SET (err_from_child, &er);
	nfds = std::max (err_from_child, nfds);
      }
      if (out_from_child >= 0)
      {
	FD_SET (out_from_child, &rd);
	FD_SET (out_from_child, &er);
	nfds = std::max (out_from_child, nfds);
      }
      if (in_to_child >= 0)
      {
	FD_SET (in_to_child,    &wr);
	FD_SET (in_to_child,    &er);
	nfds = std::max (in_to_child, nfds);
      }

      struct timespec timeout;
      timeout.tv_sec  = 60;
      timeout.tv_nsec = 0;

      if (nfds == 0) break;
      
      int ready = pselect (nfds + 1, &rd, &wr, NULL, &timeout, NULL);
      if (ready < 0 && errno == EINTR)
      {
	DLOG(TRACE, "interrupted");
	continue;
      }
      else if (ready < 0)
      {
	if (errno != 0)
	{
	  LOG(ERROR, "select() failed: " << strerror(errno));
	  terminate (pid_child);
	  throw std::runtime_error ("select() failed: " + std::string(strerror(errno)));
	}
	else
	{
	  break;
	}
      }
      else
      {
	if (out_from_child >= 0 && FD_ISSET(out_from_child, &rd))
	{
	  LOG(TRACE, "output available");

	  r = read (out_from_child, dst, size_out - bytes_rd);
	  if (r < 1)
	  {
	    LOG(TRACE, "closing output stream from child");
	    close (out_from_child);
	    out_from_child = -1;
	  }
	  else
	  {
	    LOG(TRACE, "read " << r << " bytes");
	    bytes_rd += r;
	    dst += r;
	  }
	}

	if (err_from_child >= 0 && FD_ISSET(err_from_child, &rd))
	{
	  LOG(TRACE, "error available");
	  
	  char c;
	  r = read (err_from_child, &c, 1);
	  if (r < 1)
	  {
	    LOG(TRACE, "closing error stream from child");
	    close (err_from_child);
	    err_from_child = -1;
	  }
	  else
	  {
	    err_str << c;
	  }
	}

	if (in_to_child >= 0 && FD_ISSET(in_to_child, &wr))
	{
	  LOG(TRACE, "input possible");

	  r = write (in_to_child, src, size_in - bytes_wr);

	  if (r < 1)
	  {
	    LOG(TRACE, "closing output to child");
	    close (in_to_child);
	    in_to_child = -1;
	  }
	  else
	  {
	    LOG(TRACE, "wrote " << r << " bytes to child");
	    bytes_wr += r;
	    src += r;
	  }
	}
	
	LOG(TRACE, "wr = " << bytes_wr << " rd = " << bytes_rd);

	if (bytes_rd == size_out && bytes_wr == size_in)
	{
	  LOG(INFO, "read/write completed");
	  break;
	}
      }
    }
    
    int status (0);
    waitpid (pid_child, &status, 0);
    if (WIFEXITED(status))
    {
      ec = WEXITSTATUS(status);
      MLOG(INFO, "child exited with exitcode: " << ec);
    }
    else if (WIFSIGNALED(status))
    {
      ec = -WTERMSIG(status);
      MLOG(INFO, "child exited due to signal: " << -ec);
    }
    else
    {
      MLOG(WARN, "strange child status: " << status);
      throw std::runtime_error("STRANGE child status!");
    }
  }
  else
  {
    LOG(ERROR, "could not fork child: " << strerror(errno));
    throw std::runtime_error ("could not fork child: " + std::string (strerror(errno)));
  }
  
  return ec;
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

static void exec ( void * state
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

  exec_impl (cmd, fvmGetShmemPtr(), (std::size_t)size, fvmGetShmemPtr(), (std::size_t)size);

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
  WE_REGISTER_FUN (exec);

}
WE_MOD_INITIALIZE_END (filter_trace);

WE_MOD_FINALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter_trace)");
}
WE_MOD_FINALIZE_END (filter_trace);
