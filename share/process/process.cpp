#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <stdexcept>

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

#include <fhg/util/split.hpp>

#include <process.hpp>

namespace process
{
  namespace detail
  {
    /* ********************************************************************* */

    inline void put_error (const std::string & msg)
    {
      throw std::runtime_error (msg);
    }

    template<typename T>
    inline void put_error (const std::string & msg, T x)
    {
      std::ostringstream sstr;

      sstr << msg << ": " << x;

      put_error (sstr.str());
    }

    template<typename T>
    inline void do_error (const std::string & msg, T x)
    {
      std::ostringstream sstr;

      sstr << msg << ": " << x;

      do_error (sstr.str(), errno);
    }

    template<>
    inline void do_error<int> (const std::string & msg, int err)
   {
      std::ostringstream sstr;

      sstr << msg << ": " << strerror (err);

      put_error (sstr.str());
    }

    inline void do_error (const std::string & msg)
    {
      do_error (msg, errno);
    }

    /* ********************************************************************* */

    inline void do_close (int * fd)
    {
      if (close (*fd) < 0)
        {
          do_error("close failed");
        }

      *fd = -1;
    }

    /* ********************************************************************* */

    inline void try_close (int * fd)
    {
      close (*fd);

      *fd = -1;
    }

    /* ********************************************************************* */

    enum {RD = 0, WR = 1};

    /* ********************************************************************* */

    inline void prepare_parent_pipes (int in[2], int out[2], int err[2])
    {
      do_close (in + RD);
      do_close (out + WR);
      do_close (err + WR);
    }

    /* ********************************************************************* */

    inline void prepare_child_pipes (int in[2], int out[2], int err[2])
    {
      do_close (in + WR);
      do_close (out + RD);
      do_close (err + RD);

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

      if (err[WR] != STDERR_FILENO)
        {
          if (dup2 (err[WR], STDERR_FILENO) != STDERR_FILENO)
            {
              do_error ("dup to stderr failed");
            }

          do_close (err + WR);
        }

      for (int i (3); i < 1024; ++i)
        {
          close (i);
        }
    }
  } // namespace detail

  /* *********************************************************************** */

  namespace thread
  {
    /* ********************************************************************* */

    static void circular_reader ( int fd
                                , circular_buffer & circ_buf
                                , std::size_t & bytes_read
                                )
    {
      char * buf (new char[PIPE_BUF]);

      bytes_read = 0;

      while (fd != -1)
        {
          const int r (read (fd, buf, PIPE_BUF));

          if (r < 0)
            {
              detail::do_error ("circ read failed", errno);
            }
          else if (r == 0)
            {
              fd = -1;
            }
          else
            {
              bytes_read += r;

              std::copy (buf, buf + r, std::back_inserter (circ_buf));
            }
        }

      delete[] buf;
    }

    static void reader ( int fd
                       , void * output
                       , const std::size_t & max_size
                       , std::size_t & bytes_read
                       )
    {
      char * buf (static_cast<char *>(output));

      bytes_read = 0;

      while (fd != -1)
        {
          const std::size_t to_read
            (std::min (std::size_t (PIPE_BUF), max_size - bytes_read));

          const int r (read (fd, buf, to_read));

          if (r < 0)
            {
              detail::do_error ("read from stdout failed", errno);
              break;
            }
          else if (r == 0)
            {
              fd = -1;
            }
          else
            {
              buf += r;
              bytes_read += r;
            }
        }
    }

    static void reader_from_file ( boost::barrier & barrier
                                 , const std::string & filename
                                 , void * buf
                                 , const std::size_t max_size
                                 , std::size_t & bytes_read
                                 )
    {
      barrier.wait ();

      int fd (open (filename.c_str(), O_RDONLY));

      if (fd == -1)
        {
          detail::do_error ("open file " + filename + " for reading failed", errno);
        }

      thread::reader (fd, buf, max_size, bytes_read);

      close (fd);
    }

    /* ********************************************************************* */

    static void writer ( int fd
                       , const void * input
                       , std::size_t bytes_left
                       )
    {
      char * buf (static_cast<char *> (const_cast<void *> (input)));

      while (fd != -1 && bytes_left > 0)
        {
          const std::size_t to_write
            (std::min (std::size_t (PIPE_BUF), bytes_left));

          const int w (write (fd, buf, to_write));

          if (w < 0)
            {
              detail::do_error ("write stdin failed", errno);
            }
          else if (w == 0)
            {
              fd = -1;
            }
          else
            {
              buf += w;
              bytes_left -= w;
            }
        }

      if (fd != -1)
      {
        detail::try_close(&fd);
      }
    }

    static void writer_from_file ( boost::barrier & barrier
                                 , const std::string & filename
                                 , const void * buf
                                 , std::size_t bytes_left
                                 )
    {
      barrier.wait ();

      int fd (open (filename.c_str(), O_WRONLY));

      if (fd == -1)
        {
          detail::do_error ("open file " + filename + "for writing failed", errno);
        }

      thread::writer (fd, buf, bytes_left);

      close (fd);
    }
  } // namespace thread

  /* *********************************************************************** */

  namespace detail
  {
    static std::string tempname ()
    {
      static unsigned long i (0);

      char * TMPDIR (getenv ("TMPDIR"));

      std::string dir ((TMPDIR != NULL) ? TMPDIR : P_tmpdir);

      if (dir.size() == 0)
        {
          throw std::runtime_error ("neither TMPDIR nor P_tmpdir are set");
        }

      bool file_already_exists = true;
      std::string fname;
      while (file_already_exists)
      {
        namespace fs = boost::filesystem;

        std::ostringstream sstr;
        sstr << dir
             << "/"
             << "process." << getuid () << "." << getpid() << "." << i++;
        fname = sstr.str ();

        if (not fs::exists (fname))
        {
          file_already_exists = false;
        }
      }

      return fname;
    }

    static void fifo (std::string & filename)
    {
      int ec = 0;

      do
        {
          filename = detail::tempname ();

          ec = mkfifo (filename.c_str(), S_IWUSR | S_IRUSR);

          if (ec != 0)
          {
            unlink (filename.c_str ());
            detail::do_error ("mkfifo failed", filename);
          }
        }
      while (ec != 0);
    }

    struct tempfile_t
    {
      tempfile_t ()
        : m_path ()
      {}

      explicit
      tempfile_t (std::string const &p)
        : m_path (p)
      {}

      ~tempfile_t ()
      {
        if (not m_path.empty ())
          unlink (m_path.c_str ());
      }

    private:
      std::string m_path;
    };
  }

  /* *********************************************************************** */

  namespace start
  {
    static boost::thread * writer ( boost::barrier & barrier
                                  , const void * buf
                                  , std::string & filename
                                  , std::size_t bytes_left
                                  )
    {
      detail::fifo (filename);

      return new boost::thread ( thread::writer_from_file
                               , boost::ref (barrier)
                               , filename
                               , buf
                               , bytes_left
                               );
    }

    static boost::thread * reader ( boost::barrier & barrier
                                  , void * buf
                                  , std::string & filename
                                  , const std::size_t max_size
                                  , std::size_t & bytes_read
                                  )
    {
      detail::fifo (filename);

      return new boost::thread ( thread::reader_from_file
                               , boost::ref (barrier)
                               , filename
                               , buf
                               , max_size
                               , boost::ref (bytes_read)
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
        if (this->find (key) != this->end())
          {
            put_error ("redefinition of key", key);
          }

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
            unlink (param->second.c_str());
          }
      }
    };
  }

  /* *********************************************************************** */

  execute_return_type execute ( std::string const & command
                              , const_buffer const & buf_stdin
                              , buffer const & buf_stdout
                              , circular_buffer & buf_stderr
                              , file_const_buffer_list const & files_input
                              , file_buffer_list const & files_output
                              )
  {
    typedef boost::shared_ptr<detail::tempfile_t> tempfile_ptr;
    typedef std::list<tempfile_ptr> tempfile_list_t;
    execute_return_type ret;

    pid_t pid;

    int in[2], out[2], err[2];

    if ((pipe (in) < 0) || (pipe (out) < 0) || (pipe (err) < 0))
      {
        detail::do_error ("pipe failed");
      }

    detail::param_map param_map;
    boost::thread_group writers;
    boost::thread_group readers;
    tempfile_list_t tempfiles;

    boost::barrier writer_barrier (1 + files_input.size ());
    for ( file_const_buffer_list::const_iterator file_input (files_input.begin())
        ; file_input != files_input.end()
        ; ++file_input
        )
      {
        std::string filename;

        writers.add_thread (start::writer ( writer_barrier
                                          , file_input->buf()
                                          , filename
                                          , file_input->size()
                                          )
                           );
        tempfiles.push_back (tempfile_ptr (new detail::tempfile_t (filename)));
        try
        {
          param_map[file_input->param()] = filename;
        }
        catch (...)
        {
          writers.interrupt_all ();
          writers.join_all ();
          throw;
        }
      }

    writer_barrier.wait ();

    ret.bytes_read_files_output.resize (files_output.size());

    std::size_t i (0);

    boost::barrier reader_barrier (1 + files_output.size ());
    for ( file_buffer_list::const_iterator file_output (files_output.begin())
        ; file_output != files_output.end()
        ; ++file_output, ++i
        )
      {
        std::string filename;

        readers.add_thread (start::reader ( reader_barrier
                                          , file_output->buf()
                                          , filename
                                          , file_output->size()
                                          , ret.bytes_read_files_output[i]
                                          )
                           );

        tempfiles.push_back (tempfile_ptr (new detail::tempfile_t (filename)));
        try
        {
          param_map[file_output->param()] = filename;
        }
        catch (...)
        {
          readers.interrupt_all ();
          readers.join_all ();
          throw;
        }
      }

    reader_barrier.wait ();

    std::list<std::string> const cmdline
      (fhg::util::split<std::string, std::string> (command, ' '));

    char ** av = new char*[cmdline.size()+1];
    av[cmdline.size()] = (char*)(NULL);

    {
      std::size_t idx (0);
      for ( std::list<std::string>::const_iterator it (cmdline.begin())
          ; it != cmdline.end()
          ; ++it, ++idx
          )
        {
          const detail::param_map::const_iterator repl (param_map.find (*it));

          const std::string param ( (repl != param_map.end())
                                  ? std::string (repl->second)
                                  : *it
                                  );

          av[idx] = new char[param.size()+1];
          memcpy(av[idx], param.c_str(), param.size());
          av[idx][param.size()] = (char)0;
        }
    }

    ret.exit_code = 255;
    ret.bytes_read_stdout = 0;
    ret.bytes_read_stderr = 0;

    sigset_t signals_to_block;
    sigset_t signals_to_restore;
    sigemptyset (&signals_to_block);
    sigaddset (&signals_to_block, SIGPIPE);
    sigprocmask (SIG_BLOCK, &signals_to_block, &signals_to_restore);

    pid = fork();

    if (pid < 0)
      {
        detail::do_error ("fork failed", errno);
      }
    else if (pid == pid_t (0))
      {
        // child: should not produce any output on stdout/stderr
        detail::prepare_child_pipes (in, out, err);

        if (execvp(av[0], av) < 0)
          {
            int ec = errno;

            close (0);
            close (1);
            close (2);

            if (ec == EACCES)
              _exit (126);
            if (ec == ENOENT)
              _exit (127);
            else
              _exit (254);
          }
      }
    else
      {
        // parent
        detail::prepare_parent_pipes (in, out, err);

        boost::thread thread_buf_stdin
          ( thread::writer
          , in[detail::WR]
          , buf_stdin.buf()
          , buf_stdin.size()
          );

        boost::thread thread_buf_stdout
          ( thread::reader
          , out[detail::RD]
          , buf_stdout.buf()
          , buf_stdout.size()
          , boost::ref (ret.bytes_read_stdout)
          );

        boost::thread thread_buf_stderr
          ( thread::circular_reader
          , err[detail::RD]
          , boost::ref (buf_stderr)
          , boost::ref (ret.bytes_read_stderr)
          );

        int status (0);

        waitpid (pid, &status, 0);

        thread_buf_stdin.join();
        thread_buf_stdout.join();
        thread_buf_stderr.join();

        writers.join_all();
        readers.join_all();

        detail::try_close (in + detail::WR);
        detail::try_close (out + detail::RD);
        detail::try_close (err + detail::RD);

        if (WIFEXITED (status))
          {
            ret.exit_code = WEXITSTATUS (status);
          }
        else if (WIFSIGNALED (status))
          {
            ret.exit_code = 128 + WTERMSIG (status);
          }
        else
          {
            detail::do_error ("strange child status: ", status);
          }
      }

    {
      std::size_t idx (0);

      for ( std::list<std::string>::const_iterator it (cmdline.begin())
          ; it != cmdline.end()
          ; ++it, ++idx
          )
        {
          delete[] av[idx];
        }

      delete[] av;
    }

    sigprocmask (SIG_UNBLOCK, &signals_to_restore, NULL);

    return ret;
  }
}
