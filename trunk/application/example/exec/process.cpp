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
#include <boost/unordered_map.hpp>

#include <fhg/util/show.hpp>

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

    static void reader ( int fd
                       , void * output
                       , const std::size_t & max_size
                       , std::size_t & bytes_read
                       , const std::size_t & block_size
                       )
    {
      DLOG (TRACE, "start thread read");

      char * buf (static_cast<char *>(output));

      while (fd != -1)
        {
          DLOG (TRACE, "try to read " << block_size << " bytes");

          const int r (read ( fd
                            , buf
                            , std::max (block_size, max_size - bytes_read)
                            )
                      );

          if (r < 0)
            {
              detail::do_error ("read stdout failed");
            }
          else if (r == 0)
            {
              DLOG (TRACE, "read pipe closed");

              fd = -1;
            }
          else
            {
              if (bytes_read >= max_size)
                {
                  detail::put_error ("stdout buffer overflow");
                }

              buf += r;
              bytes_read += r;

              DLOG (TRACE, "read " << r << " bytes, sum " << bytes_read);
            }
        }

      DLOG (TRACE, "done thread read");
    }

    /* ********************************************************************* */

    static void writer ( int fd
                       , const void * input
                       , std::size_t bytes_left
                       , const std::size_t & block_size
                       )
    {
      DLOG (TRACE, "start thread write");

      char * buf (static_cast<char *> (const_cast<void *> (input)));

      while (fd != -1 && bytes_left > 0)
        {
          DLOG (TRACE, "try to write " << bytes_left << " bytes");

          const int w (write (fd, buf, std::min (block_size, bytes_left)));

          if (w < 0)
            {
              detail::do_error ("write stdin failed");
            }
          else if (w == 0)
            {
              DLOG (TRACE, "write pipe closed");

              fd = -1;
            }
          else
            {
              buf += w;
              bytes_left -= w;

              DLOG (TRACE, "written " << w << " bytes, left " << bytes_left);
            }
        }

      if (fd != -1)
        {
          detail::do_close (&fd);
        }

      DLOG (TRACE, "done thread write");
    }

    static void writer_from_file ( const std::string & filename
                                 , const void * buf
                                 , std::size_t bytes_left
                                 , const std::size_t & block_size
                                 )
    {
      int fd (open (filename.c_str(), O_WRONLY));

      if (fd == -1)
        {
          detail::do_error ("open file for writing failed", filename);
        }

      thread::writer (fd, buf, bytes_left, PIPE_BUF);
    }
  } // namespace thread

  /* *********************************************************************** */

  namespace detail
  {
    static std::string tempname (const std::string & prefix)
    {
      static unsigned long i (0);

      char * TMPDIR (getenv ("TMPDIR"));

      std::string dir ((TMPDIR != NULL) ? TMPDIR : P_tmpdir);

      if (dir.size() == 0)
        {
          throw std::runtime_error ("neither TMPDIR nor P_tmpdir are set");
        }

      return dir + "/" + prefix + fhg::util::show (i++);
    }
  }

  /* *********************************************************************** */

  namespace start
  {
    static boost::thread * writer ( const void * buf
                                  , std::string & filename
                                  , std::size_t bytes_left
                                  )
    {
      filename = detail::tempname ("write.");

      if (mkfifo (filename.c_str(), S_IWUSR | S_IRUSR))
        {
          detail::do_error ("mkfifo failed", filename);
        }

      return new boost::thread ( thread::writer_from_file
                               , filename
                               , buf
                               , bytes_left
                               , PIPE_BUF
                               );
    }
  }

  /* *********************************************************************** */

  namespace detail
  {
    struct param_map
    {
    private:
      typedef boost::unordered_map <std::string, std::string> param_map_t;

      param_map_t _map;

    public:
      param_map_t::mapped_type & operator [] (const param_map_t::key_type & key)
      {
        return _map[key];
      }

      param_map_t::const_iterator find (const param_map_t::key_type & key) const
      {
        return _map.find (key);
      }

      typedef param_map_t::const_iterator const_iterator;

      const_iterator begin (void) const { return _map.begin(); }
      const_iterator end (void) const { return _map.end(); }

      ~param_map ()
      {
        for ( param_map_t::const_iterator param (_map.begin())
            ; param != _map.end()
            ; ++param
            )
          {
            DLOG (TRACE, "unlink " << param->second);

            unlink (param->second.c_str());
          }
      }
    };
  }

  /* *********************************************************************** */

  std::size_t execute ( std::string const & command
                      , const_buffer const & buf_stdin
                      , buffer const & buf_stdout
                      , file_const_buffer_list const & files_input
                      , file_buffer_list const & files_output
                      )
  {
    pid_t pid;

    int in[2], out[2];

    if ((pipe (in) < 0) || (pipe (out) < 0))
      {
        detail::do_error ("pipe failed");
      }

    detail::param_map param_map;
    boost::thread_group writers;

    for ( file_const_buffer_list::const_iterator file_input (files_input.begin())
        ; file_input != files_input.end()
        ; ++file_input
        )
      {
        std::string filename;

        writers.add_thread (start::writer ( file_input->buf()
                                          , filename
                                          , file_input->size()
                                          )
                           );

        param_map[file_input->param()] = filename;
      }

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
            const detail::param_map::const_iterator repl
              (param_map.find (*it));

            const std::string param ((repl != param_map.end())
                                    ? std::string (repl->second)
                                    : *it
                                    );

            av[idx] = new char[param.size()+1];
            memcpy(av[idx], param.c_str(), param.size());
            av[idx][param.size()] = (char)0;
          }

        MLOG (INFO, "run command: " << fhg::util::show (av,av+cmdline.size()));

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

        boost::thread thread_buf_stdin
          ( thread::writer
          , in[detail::WR]
          , buf_stdin.buf()
          , buf_stdin.size()
          , PIPE_BUF
          );

        boost::thread thread_buf_stdout
          ( thread::reader
          , out[detail::RD]
          , buf_stdout.buf()
          , buf_stdout.size()
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

        thread_buf_stdin.join();
        thread_buf_stdout.join();

        writers.join_all();

        MLOG (INFO, "finished command: " << command);

        return bytes_read;
      }

    return 0;
  }
}
