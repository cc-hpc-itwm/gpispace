#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>

#include <stdexcept>

#include <boost/random.hpp>
#include <limits>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/signal.h>

using we::loader::put;
using we::loader::get;

// ************************************************************************* //

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
      LOG(DEBUG, "av[" << idx << "]: " << *it);
      av[idx] = new char[it->size()+1];
      memcpy(av[idx], it->c_str(), it->size());
      av[idx][it->size()] = (char)0;
    }
    
    std::stringstream sstr_cmd;
    for(size_t k=0; k<idx; k++)
      sstr_cmd << av[idx];
  
    close (0);
    close (1);
    close (2);
    
    close (in_pipe[1]);
    close (out_pipe[0]);
    close (err_pipe[0]);

    dup2(in_pipe[0], 0);
    dup2(out_pipe[1], 1);
    dup2(out_pipe[1], 2);
    
    //    if ( execve( av[0], av, environ) < 0 )
    if ( execvp( av[0], av ) < 0 )
    {
      LOG(ERROR, "could not execv(" << command << "): " << strerror(errno));
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
      
      int ready = pselect (nfds + 1, &rd, &wr, NULL, NULL, NULL);
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
	  DLOG(TRACE, "output available");

	  r = read (out_from_child, dst, size_out - bytes_rd);
	  if (r < 1)
	  {
	    DLOG(TRACE, "closing output stream from child");
	    close (out_from_child);
	    out_from_child = -1;
	  }
	  else
	  {
	    DLOG(TRACE, "read " << r << " bytes");
	    bytes_rd += r;
	    dst += r;
	  }
	}

	if (err_from_child >= 0 && FD_ISSET(err_from_child, &rd))
	{
	  DLOG(TRACE, "error available");
	  
	  char c;
	  r = read (err_from_child, &c, 1);
	  if (r < 1)
	  {
	    DLOG(TRACE, "closing error stream from child");
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
	  DLOG(TRACE, "input possible");

	  r = write (in_to_child, src, size_in - bytes_wr);

	  if (r < 1)
	  {
	    DLOG(TRACE, "closing output to child");
	    close (in_to_child);
	    in_to_child = -1;
	  }
	  else
	  {
	    DLOG(TRACE, "wrote " << r << " bytes to child");
	    bytes_wr += r;
	    src += r;
	  }
	}
	
	DLOG(TRACE, "wr = " << bytes_wr << " rd = " << bytes_rd);

	if (bytes_rd == size_out && bytes_wr == size_in)
	{
	  DLOG(INFO, "read/write completed");
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

// ************************************************************************* //

static void exec_wrapper ( void *
			 , const we::loader::input_t & input
			 , we::loader::output_t & output
			 )
{
  const std::string & command (get<std::string> (input, "command"));

  MLOG (INFO, "exec:  = \"" << command << "\"");

  void * input_traces (0);
  void * output_traces (0);
  
  long ec (exec_impl (command, input_traces, 0, output_traces, 0));

  MLOG (INFO, "exec: returned with: " << ec);
  put (output, "ec", ec);
}

// ************************************************************************* //

static void selftest ( void *
		     , const we::loader::input_t &
		     , we::loader::output_t &
		     )
{
  char out_buf[1024];
  memset (out_buf, 0, sizeof(out_buf));

  const std::string inp ("Hallo Welt!");
  
  long ec (exec_impl ( "/bin/cat"
		     , inp.c_str(), inp.size()
		     , out_buf, sizeof(out_buf)
		     )
	  );

  MLOG(INFO, "selftest: ec=" << ec << " o=" << out_buf);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (exec);
{
  WE_REGISTER_FUN_AS (exec_wrapper, "exec");
  WE_REGISTER_FUN (selftest);
  
  char out_buf[10240];
  memset (out_buf, 0, sizeof(out_buf));

  const std::string inp ("Hallo Welt!");
  
  long ec (exec_impl ( "cat /etc/passwd"
		     , inp.c_str(), inp.size()
		     , out_buf, sizeof(out_buf)
		     )
	  );
  MLOG(INFO, "output = " << out_buf);
}
WE_MOD_INITIALIZE_END (exec);

WE_MOD_FINALIZE_START (exec);
{
}
WE_MOD_FINALIZE_END (exec);
