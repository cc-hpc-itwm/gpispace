#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/map.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/split.hpp>

#include <process.hpp>

#include <functional>

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

    inline void prepare_parent_pipes
      (int in[2], int out[2], int err[2], int sync_pc[2], int sync_cp[2])
    {
      do_close (out + WR);
      do_close (err + WR);
      do_close (sync_pc + RD);
      do_close (sync_cp + WR);
    }

    /* ********************************************************************* */

    inline void prepare_child_pipes
      (int in[2], int out[2], int err[2], int sync_pc[2], int sync_cp[2])
    {
      do_close (in + WR);
      do_close (out + RD);
      do_close (err + RD);
      do_close (sync_pc + WR);
      do_close (sync_cp + RD);

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
        if (i != sync_pc[RD] && i != sync_cp[WR])
        {
          close (i);
        }
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
          const ssize_t r (read (fd, buf, PIPE_BUF));

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

          const ssize_t r (read (fd, buf, to_read));

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
                       , std::size_t* written = nullptr
                       )
    {
      struct maybe_try_close_on_scope_exit_t : boost::noncopyable
      {
        maybe_try_close_on_scope_exit_t (int* fd)
          : _fd (fd)
        {}
        ~maybe_try_close_on_scope_exit_t()
        {
          if (*_fd != -1)
          {
            detail::try_close (_fd);
          }
        }
        int* _fd;
      } const maybe_try_close_on_scope_exit (&fd);

      const char * buf (static_cast<const char*> (input));

      while (fd != -1 && bytes_left > 0)
        {
          const std::size_t to_write
            (std::min (std::size_t (PIPE_BUF), bytes_left));

          const ssize_t w (write (fd, buf, to_write));

          if (w < 0)
            {
              if (errno == EPIPE)
              {
                fd = -1;
                break;
              }

              detail::do_error ("write stdin failed", errno);
            }
          else if (w == 0)
            {
              fd = -1;
            }
          else
            {
              buf += w;
              if (written) *written += w;
              bytes_left -= w;
            }

          boost::this_thread::interruption_point();
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

      std::string dir ((TMPDIR != nullptr) ? TMPDIR : P_tmpdir);

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

    static void fifo (std::string const& filename)
    {
          if (mkfifo (filename.c_str(), S_IWUSR | S_IRUSR) != 0)
          {
            unlink (filename.c_str ());
            detail::do_error ("mkfifo failed", filename);
          }
    }

    struct tempfile_t : boost::noncopyable
    {
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
                                  , std::string const& filename
                                  , std::size_t bytes_left
                                  )
    {
      return new boost::thread ( thread::writer_from_file
                               , std::ref (barrier)
                               , filename
                               , buf
                               , bytes_left
                               );
    }

    static boost::thread * reader ( boost::barrier & barrier
                                  , void * buf
                                  , std::string const& filename
                                  , const std::size_t max_size
                                  , std::size_t & bytes_read
                                  )
    {
      return new boost::thread ( thread::reader_from_file
                               , std::ref (barrier)
                               , filename
                               , buf
                               , max_size
                               , std::ref (bytes_read)
                               );
    }
  }

  /* *********************************************************************** */

  namespace detail
  {
    struct param_map
    {
    private:
      typedef std::unordered_map <std::string, std::string> param_map_t;

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
        for (std::string entry : _map | boost::adaptors::map_values)
          {
            unlink (entry.c_str());
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
    for (file_const_buffer const& file_input : files_input)
      {
        std::string filename (detail::tempname());

        detail::fifo (filename);

        writers.add_thread (start::writer ( writer_barrier
                                          , file_input.buf()
                                          , filename
                                          , file_input.size()
                                          )
                           );
        tempfiles.push_back (tempfile_ptr (new detail::tempfile_t (filename)));
        try
        {
          param_map[file_input.param()] = filename;
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
    for (file_buffer const& file_output : files_output)
      {
        std::string filename (detail::tempname());

        detail::fifo (filename);

        readers.add_thread (start::reader ( reader_barrier
                                          , file_output.buf()
                                          , filename
                                          , file_output.size()
                                          , ret.bytes_read_files_output[i]
                                          )
                           );

        tempfiles.push_back (tempfile_ptr (new detail::tempfile_t (filename)));
        try
        {
          param_map[file_output.param()] = filename;
        }
        catch (...)
        {
          readers.interrupt_all ();
          readers.join_all ();
          throw;
        }
        ++i;
      }

    reader_barrier.wait ();

    std::list<std::string> const cmdline
      (fhg::util::split<std::string, std::string> (command, ' '));

    char ** av = new char*[cmdline.size()+1];
    av[cmdline.size()] = (char*)(nullptr);

    {
      std::size_t idx (0);
      for (std::string raw_param : cmdline)
        {
          const detail::param_map::const_iterator repl (param_map.find (raw_param));

          const std::string param ( (repl != param_map.end())
                                  ? std::string (repl->second)
                                  : raw_param
                                  );

          av[idx] = new char[param.size()+1];
          memcpy(av[idx], param.c_str(), param.size());
          av[idx][param.size()] = '\0';
          ++idx;
        }
    }

    ret.exit_code = 255;
    ret.bytes_written_stdin = 0;
    ret.bytes_read_stdout = 0;
    ret.bytes_read_stderr = 0;

    sigset_t signals_to_block;
    sigset_t signals_to_restore;
    sigemptyset (&signals_to_block);
    sigaddset (&signals_to_block, SIGPIPE);
    sigprocmask (SIG_BLOCK, &signals_to_block, &signals_to_restore);

    int synchronization_fd_parent_child[2] = { -1, -1 };
    int synchronization_fd_child_parent[2] = { -1, -1 };
    char synchronization_buffer = '0';
    fhg::syscall::pipe (synchronization_fd_parent_child);
    fhg::syscall::pipe (synchronization_fd_child_parent);

    pid = fork();

    if (pid < 0)
      {
        detail::do_error ("fork failed", errno);
      }
    else if (pid == pid_t (0))
      {
        // child: should not produce any output on stdout/stderr
        detail::prepare_child_pipes ( in
                                    , out
                                    , err
                                    , synchronization_fd_parent_child
                                    , synchronization_fd_child_parent
                                    );

        if ( fhg::syscall::write
             ( synchronization_fd_child_parent[detail::WR]
             , &synchronization_buffer
             , sizeof (synchronization_buffer)
             )
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: parent did not read");
        }

        // wait for parent setting up threads

        if ( fhg::syscall::read
             ( synchronization_fd_parent_child[detail::RD]
             , &synchronization_buffer
             , sizeof (synchronization_buffer)
             )
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: parent did not write");
        }

        fhg::syscall::close (synchronization_fd_parent_child[detail::RD]);
        fhg::syscall::close (synchronization_fd_child_parent[detail::WR]);

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
        detail::prepare_parent_pipes ( in
                                     , out
                                     , err
                                     , synchronization_fd_parent_child
                                     , synchronization_fd_child_parent
                                     );

        if ( fhg::syscall::read
             ( synchronization_fd_child_parent[detail::RD]
             , &synchronization_buffer
             , sizeof (synchronization_buffer)
             )
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: child did not write");
        }

        boost::thread thread_buf_stdin
          ( [&buf_stdin, &in, &ret]
          {
            thread::writer ( in[detail::WR]
                           , buf_stdin.buf(), buf_stdin.size()
                           , &ret.bytes_written_stdin
                           );
          }
          );

        boost::thread thread_buf_stdout
          ( [&buf_stdout, &out, &ret]
          {
            thread::reader ( out[detail::RD]
                           , buf_stdout.buf(), buf_stdout.size()
                           , ret.bytes_read_stdout
                           );
          }
          );

        boost::thread thread_buf_stderr
          ( [&buf_stderr, &err, &ret]
          {
            thread::circular_reader
              (err[detail::RD], buf_stderr, ret.bytes_read_stderr);
          }
          );

        if ( fhg::syscall::write
             ( synchronization_fd_parent_child[detail::WR]
             , &synchronization_buffer
             , sizeof (synchronization_buffer)
             )
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: child did not read");
        }

        fhg::syscall::close (synchronization_fd_parent_child[detail::WR]);
        fhg::syscall::close (synchronization_fd_child_parent[detail::RD]);

        int status (0);

        waitpid (pid, &status, 0);

        thread_buf_stdin.interrupt();

        {
          ssize_t still_unconsumed (0);

          char buffer[PIPE_BUF];
          bool try_read (true);
          while (try_read)
          {
            const ssize_t consumed
              (fhg::syscall::read (in[detail::RD], buffer, sizeof (buffer)));
            try_read = consumed > 0;
            still_unconsumed += consumed;
          }
          ret.bytes_written_stdin -= still_unconsumed;
        }

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
      for (std::size_t idx (0); idx < cmdline.size(); ++idx)
        {
          delete[] av[idx];
        }

      delete[] av;
    }

    sigprocmask (SIG_UNBLOCK, &signals_to_restore, nullptr);

    return ret;
  }
}
