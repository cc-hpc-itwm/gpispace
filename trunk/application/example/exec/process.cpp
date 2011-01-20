#include <fhglog/fhglog.hpp>

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

#include <process.hpp>

#include <boost/thread.hpp>

namespace process
{
  namespace detail
  {
    /* ********************************************************************* */

    inline void put_error (const std::string & msg)
    {
      MLOG (ERROR, msg);

      throw std::runtime_error (msg);
    }

    inline void do_error (const std::string & msg)
    {
      std::ostringstream sstr;

      sstr << msg << ": " << strerror (errno);

      put_error (sstr.str());
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
                       , const std::size_t & max_size
                       , std::size_t & bytes_read
                       , const std::size_t & block_size
                       )
    {
      DLOG (TRACE, "start thread read");

      char * buf (static_cast<char *>(output));

      while (*fd != -1)
        {
          DLOG (TRACE, "try to read " << block_size << " bytes");

          const int r (read ( *fd
                            , buf
                            , std::max (block_size, max_size - bytes_read)
                            )
                      );

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
              if (bytes_read >= max_size)
                {
                  detail::put_error ("buffer full but still data available");
                }

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
                       , std::size_t & bytes_left
                       , const std::size_t & block_size
                       )
    {
      DLOG (TRACE, "start thread write");

      char * buf (static_cast<char *> (const_cast<void *> (input)));

      while (*fd != -1 && bytes_left > 0)
        {
          DLOG (TRACE, "try to write " << bytes_left << " bytes");

          const int w (write (*fd, buf, std::min (block_size, bytes_left)));

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
                      , const void * input
                      , const std::size_t & input_size
                      , void * output
                      , const std::size_t & max_output_size
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

        std::size_t bytes_read (0);
        std::size_t bytes_left (input_size);

        boost::thread thread_writer
          ( thread::writer
          , in + detail::WR
          , input
          , boost::ref (bytes_left)
          , PIPE_BUF
          );

        boost::thread thread_reader
          ( thread::reader
          , out + detail::RD
          , output
          , max_output_size
          , boost::ref (bytes_read)
          , PIPE_BUF
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

    return 0;
  }
}
