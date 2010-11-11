#include <fhglog/fhglog.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>

#include <stdexcept>
#include <limits>

#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/signal.h>

#include "process.hpp"

namespace process
{
  process_t::process_t (std::string const & command_line)
    : command_line_(command_line)
    , child_pid_(-1)
    , exit_code_(0)
  {
    for (int stream (0); stream < 3; ++stream)
    {
      for (int side (0); side < 2; ++side)
      {
        pipe_[stream][side] = -1;
      }
    }
  }

  bool process_t::is_alive () const
  {
    if (child_pid_ < 1)
    {
      return false;
    }

    for (;;)
    {
      int status(0);
      errno = 0;
      int ec = waitpid(child_pid_, &status, WNOHANG);
      if (ec == child_pid_)
      {
        child_pid_ = -1;
        if (WIFEXITED (status))
        {
          exit_code_ = WEXITSTATUS(status);
          MLOG(INFO, "child exited with exitcode: " << exit_code_);
          return false;
        }
        else if (WIFSIGNALED(status))
        {
          exit_code_ = -WTERMSIG(status);
          MLOG(INFO, "child exited due to signal: " << -exit_code_);
          return false;
        }
      }
      else if (ec == 0)
      {
        return true;
      }
      else
      {
        if (errno == EINTR)
          continue;
        else if (errno == ECHILD)
        {
          MLOG(ERROR, "child is already dead!");
          return false;
        }
        else
        {
          MLOG(ERROR, "STRANGE! " << strerror(errno));
          child_pid_ = -1;
          return false;
        }
      }
    }
    return true;
  }

  int process_t::terminate ()
  {
    int ec (0);

    if (is_alive ())
    {
      bool dead (false);

      // TODO: how to terminate and wait without blocking properly?
      kill (child_pid_, SIGTERM);
      for (int trial (0); trial < 3 && !dead; ++trial)
      {
        int status (0);
        usleep (100);
        if (waitpid(child_pid_, &status, WNOHANG) != 0)
        {
          if (WIFEXITED (status))
          {
            ec = WEXITSTATUS(status);
            dead = true;
            MLOG(INFO, "child exited with exitcode: " << ec);
          } else if (WIFSIGNALED(status))
          {
            ec = -WTERMSIG(status);
            dead = true;
            MLOG(INFO, "child exited due to signal: " << -ec);
          }
        }
      }
      if (! dead)
      {
        MLOG(WARN, "killing child: " << child_pid_);

        kill (child_pid_, SIGKILL);
      }
    }
    return ec;
  }

  int process_t::wait (int options)
  {
    LOG(DEBUG, "waiting for child...");

    if (child_pid_ > 0)
    {
      int status (0);
      waitpid(child_pid_, &status, options);
      if (WIFEXITED (status))
      {
        exit_code_ = WEXITSTATUS(status);
        MLOG(INFO, "child exited with exitcode: " << exit_code_);
      } else if (WIFSIGNALED(status))
      {
        exit_code_ = -WTERMSIG(status);
        MLOG(INFO, "child exited due to signal: " << -exit_code_);
      }
      else
      {
        MLOG(ERROR, "STRANGE status after waitpid(): " << status << ": " << strerror(errno));
        exit_code_ = 42;
      }
      child_pid_ = -1;

      for (int stream = 0; stream < 3; ++stream)
      {
        close (pipe_[stream][0]); pipe_[stream][0] = -1;
      }
    }

    return exit_code_;
  }

  void process_t::start ()
  {
    if (is_alive())
    {
      throw std::runtime_error ("already running!");
    }

    initialize_pipes ();
    pid_t self (getpid());

    errno = 0;
    child_pid_ = fork();
    if (child_pid_ == 0)
    {
      // never returns
      child (self);
    }
    else if (child_pid_ > 0)
    {
      LOG(INFO, "child running with pid: " << child_pid_);

      close (pipe_[0][1]);
      close (pipe_[0][1]);
      close (pipe_[0][1]);

      return;
    }
    else
    {
      LOG(ERROR, "could not fork child: " << strerror(errno));
      throw std::runtime_error ("could not fork child: " + std::string (strerror(errno)));
    }
  }

  void process_t::child (const pid_t parent_pid)
  {
    dup2(pipe_[0][1], 0);
    dup2(pipe_[1][1], 1);
    dup2(pipe_[2][1], 2);

    {
      int fd (1024);
      while ( fd --> 3 )
      {
        close (fd);
      }
    }

    std::vector<std::string> cmdline;
    fhg::log::split ( command_line_
                    , " "
                    , std::back_inserter (cmdline)
                    );

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

    if ( execvp( av[0], av ) < 0 )
    {
      std::cerr << "could not execute command line: " << command_line_ << std::endl;
      _exit (127);
    }
  }

  void process_t::communicate ( const void * input, const std::size_t input_size
                            , void * output, const std::size_t output_size
                            , const std::size_t buffer_size
                            )
  {
    const int timeout = -1;

    const std::size_t error_size (4096);
    char error_buffer[error_size + 1];
    error_buffer[error_size] = 0;

    const char *src = (const char*)(input);
    char *dst = (char*)(output);
    char *err = error_buffer;

    std::size_t bytes_wr (0);
    std::size_t bytes_rd (0);
    std::size_t bytes_er (0);

    signal (SIGPIPE, SIG_IGN);

    struct pollfd poll_fd[3];

    // stderr
    poll_fd[0].fd = pipe_[2][0];
    poll_fd[0].events  = POLLIN;
    poll_fd[0].revents = 0;

    // stdout
    poll_fd[1].fd = pipe_[1][0];
    poll_fd[1].events  = POLLIN;
    poll_fd[1].revents = 0;

    // stdin
    poll_fd[2].fd = pipe_[0][0];
    poll_fd[2].events  = POLLOUT;
    poll_fd[2].revents = 0;

    do
    {
      errno = 0;
      DLOG(TRACE, "polling...");
      int ready = poll (poll_fd, 3, timeout);

      for (int fd (0); fd < 3 && ready; ++fd)
      {
        if (poll_fd[fd].revents != 0)
        {
          --ready;
          if (poll_fd[fd].revents & POLLERR)
          {
            LOG(WARN, "error on filedescriptor " << poll_fd[fd].fd);
            if (poll_fd[fd].fd == pipe_[0][0])
            {
              close (pipe_[0][0]);
              pipe_[0][0] = poll_fd[fd].fd = -1;
            }
            LOG(WARN, "stopping to read/write...");
            ready = 0;
          }
          else if (poll_fd[fd].revents & POLLHUP)
          {
            LOG(WARN, "hup on filedescriptor " << poll_fd[fd].fd);
          }
          else if (poll_fd[fd].revents & POLLNVAL)
          {
            LOG(WARN, "nval on filedescriptor " << poll_fd[fd].fd);
          }
          else if (poll_fd[fd].revents & POLLOUT)
          {
            DLOG(TRACE, "can write to " << poll_fd[fd].fd);
            if (poll_fd[fd].fd == pipe_[0][0]) {
              int wr = write ( pipe_[0][0]
                             , src
                             , std::min( buffer_size
                                       , (input_size - bytes_wr)
                                       )
                             );
              if (wr < 1)
              {
                DLOG(TRACE, "closing input stream");
                close (pipe_[0][0]);
                pipe_[0][0] = poll_fd[fd].fd = -1;
              }
              else
              {
                bytes_wr += wr;
                src += wr;
              }
            }
            else
            {
              LOG(ERROR, "STRANGE file descriptor ready to be read from: " << poll_fd[fd].fd);
              throw std::runtime_error("STRANGE file descriptor ready to be read from!");
            }
          }
          else if (poll_fd[fd].revents & POLLIN)
          {
            DLOG(TRACE, "data available on fd: " << poll_fd[fd].fd);
            if (poll_fd[fd].fd == pipe_[1][0])
            {
              DLOG(TRACE, "child sent something on stdout");
              int rd = read (pipe_[1][0], dst, output_size - bytes_rd);
              if (rd < 1)
              {
                DLOG(TRACE, "closing output stream");
                close (pipe_[1][0]);
                pipe_[1][0] = -1;
                poll_fd[fd].fd = -1;
              }
              else
              {
                bytes_rd += rd;
                dst += rd;
              }
            }
            else if (poll_fd[fd].fd == pipe_[2][0])
            {
              DLOG(TRACE, "child sent something on stderr");
              int rd = read (pipe_[2][0], err, error_size - bytes_er);
              if (rd < 1)
              {
                DLOG(TRACE, "closing error stream");
                close (pipe_[2][0]);
                pipe_[2][0] = poll_fd[fd].fd = -1;
              }
              else
              {
                DLOG(TRACE, "read: " << err);
                bytes_er += rd;
                err += rd;
                if (bytes_er > error_size)
                {
                  bytes_er = 0;
                  err = error_buffer;
                }
              }
            }
            else
            {
              LOG(WARN, "STRANGE file descriptor " << poll_fd[fd].fd << " ready for read");
              throw std::runtime_error ("unknown filedescriptor!");
            }
          }
          else
          {
            LOG(ERROR, "STRANGE revents: " << poll_fd[fd].revents << " on fd " << poll_fd[fd].fd);
            throw std::runtime_error ("STRANGE revents!");
          }
        }
      }

      if (! is_alive())
      {
        LOG(WARN, "child is gone");
        break;
      }
    } while (true);

    LOG(INFO, "read " << bytes_rd << "/" << output_size << " written " << bytes_wr << "/" << input_size);
  }

  int process_t::initialize_pipes ()
  {
    for (int stream (0); stream < 3; ++stream)
    {
      errno = 0;
      if (pipe(pipe_[stream]) < 0)
      {
        LOG(ERROR, "opening pipe for stream " << stream << " failed: " << strerror(errno));
        while (stream --> 0)
        {
          close (pipe_[stream][0]);
          close (pipe_[stream][1]);
        }
        return -1;
      }

      fcntl (pipe_[stream][0], F_SETFD, O_NONBLOCK);
      fcntl (pipe_[stream][1], F_SETFD, O_NONBLOCK);
    }

    int tmp = pipe_[0][0];
    pipe_[0][0] = pipe_[0][1];
    pipe_[0][1] = tmp;

    return 0;
  }

  int execute (std::string const & cmd
              , const void * input, const std::size_t input_size
              , void * output, const std::size_t output_size
              , const std::size_t buffer_size
              )
  {
    process_t proc (cmd);
    proc.start ();
    proc.communicate ( input, input_size
                     , output, output_size
                     , buffer_size
                     );
    return proc.wait ();
  }
}
