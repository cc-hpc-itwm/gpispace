#include <process.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/split.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include <errno.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

    inline void try_close (int fd)
    {
      try
      {
        fhg::syscall::close (fd);
      }
      catch (boost::system::system_error const& err)
      {
        if (err.code() != boost::system::errc::bad_file_descriptor)
        {
          throw;
        }
        // ignore: fd wasn't open
      }
    }

    /* ********************************************************************* */

    enum {RD = 0, WR = 1};

    /* ********************************************************************* */

    inline void prepare_parent_pipes
      (int out[2], int err[2], int sync_pc[2], int sync_cp[2])
    {
      fhg::syscall::close (out[WR]);
      fhg::syscall::close (err[WR]);
      fhg::syscall::close (sync_pc[RD]);
      fhg::syscall::close (sync_cp[WR]);
    }

    /* ********************************************************************* */

    inline void prepare_child_pipes
      (int in[2], int out[2], int err[2], int sync_pc[2], int sync_cp[2])
    {
      fhg::syscall::close (in[WR]);
      fhg::syscall::close (out[RD]);
      fhg::syscall::close (err[RD]);
      fhg::syscall::close (sync_pc[WR]);
      fhg::syscall::close (sync_cp[RD]);

      if (in[RD] != STDIN_FILENO)
      {
        fhg::syscall::dup (in[RD], STDIN_FILENO);
        fhg::syscall::close (in[RD]);
      }

      if (out[WR] != STDOUT_FILENO)
      {
        fhg::syscall::dup (out[WR], STDOUT_FILENO);
        fhg::syscall::close (out[WR]);
      }

      if (err[WR] != STDERR_FILENO)
      {
        fhg::syscall::dup (err[WR], STDERR_FILENO);
        fhg::syscall::close (err[WR]);
      }

      int maximum_open_files (sysconf (_SC_OPEN_MAX));
      for (int i (3); i < maximum_open_files; ++i)
      {
        if (i != sync_pc[RD] && i != sync_cp[WR])
        {
          try_close (i);
        }
      }
    }
  } // namespace detail

  namespace thread
  {
    /* ********************************************************************* */

    static void circular_reader ( int fd
                                , circular_buffer & circ_buf
                                , std::size_t & bytes_read
                                )
    {
      std::vector<char> buf (PIPE_BUF);

      bytes_read = 0;

      for (;;)
      {
        const ssize_t r (fhg::syscall::read (fd, buf.data(), PIPE_BUF));

        if (r == 0)
        {
          break;
        }
        else
        {
          bytes_read += r;

          std::copy (buf.data(), buf.data() + r, std::back_inserter (circ_buf));
        }
      }
    }

    static void reader ( int fd
                       , void * output
                       , const std::size_t & max_size
                       , std::size_t & bytes_read
                       )
    {
      char * buf (static_cast<char *>(output));

      bytes_read = 0;

      for (;;)
      {
        const std::size_t to_read
          (std::min (std::size_t (PIPE_BUF), max_size - bytes_read));

        const ssize_t r (fhg::syscall::read (fd, buf, to_read));

        if (r == 0)
        {
          break;
        }
        else
        {
          buf += r;
          bytes_read += r;
        }
      }
    }

    /* ********************************************************************* */

    static void writer ( int fd
                       , const void * input
                       , std::size_t bytes_left
                       , std::size_t& written
                       )
    {
      const char * buf (static_cast<const char*> (input));

      written = 0;

      while (bytes_left > 0)
      {
        const std::size_t to_write
          (std::min (std::size_t (PIPE_BUF), bytes_left));

        try
        {
          const ssize_t w (fhg::syscall::write (fd, buf, to_write));

          if (w == 0)
          {
            break;
          }
          else
          {
            buf += w;
            written += w;
            bytes_left -= w;
          }
        }
        catch (boost::system::system_error const& err)
        {
          if (err.code() == boost::system::errc::broken_pipe)
          {
            break;
          }
          throw;
        }

        boost::this_thread::interruption_point();
      }
    }
  } // namespace thread

  /* *********************************************************************** */

  namespace detail
  {
    static std::string tempname()
    {
      static unsigned long i (0);

      char * TMPDIR (getenv ("TMPDIR"));

      std::string dir ((TMPDIR != nullptr) ? TMPDIR : P_tmpdir);

      if (dir.size() == 0)
      {
        throw std::runtime_error ("neither TMPDIR nor P_tmpdir are set");
      }

      std::string fname;
      do
      {
        fname = ( boost::format ("%1%/process.%2%.%3%.%4%")
                % dir % getuid() % getpid() % i++
                ).str();
      }
      while (boost::filesystem::exists (fname));

      return fname;
    }

    struct tempfifo_t : boost::noncopyable
    {
      explicit tempfifo_t (std::string const &p)
        : m_path (p)
      {
        fhg::syscall::mkfifo (m_path.c_str(), S_IWUSR | S_IRUSR);
      }

      ~tempfifo_t()
      {
        fhg::syscall::unlink (m_path.c_str());
      }

    private:
      std::string m_path;
    };
  }

  /* *********************************************************************** */

  namespace
  {
    std::size_t consume_everything (int fd)
    {
      std::size_t consumed_accum (0);

      char buffer[PIPE_BUF];
      for (;;)
      {
        try
        {
          const ssize_t consumed
            (fhg::syscall::read (fd, buffer, sizeof (buffer)));

          if (consumed == 0)
          {
            break;
          }

          consumed_accum += consumed;
        }
        catch (boost::system::system_error const& err)
        {
          //! \note if fd is nonblocking and there is nothing to read
          if (err.code() == boost::system::errc::resource_unavailable_try_again)
          {
            break;
          }
          throw;
        }
      }

      return consumed_accum;
    }

    struct scoped_SIGPIPE_block
    {
      scoped_SIGPIPE_block()
      {
        sigset_t signals_to_block;
        sigemptyset (&signals_to_block);
        sigaddset (&signals_to_block, SIGPIPE);
        pthread_sigmask (SIG_BLOCK, &signals_to_block, &_signals_to_restore);
      }
      ~scoped_SIGPIPE_block()
      {
        pthread_sigmask (SIG_UNBLOCK, &_signals_to_restore, nullptr);
      }
      sigset_t _signals_to_restore;
    };

    namespace sync
    {
      char synchronization_buffer = '0';

      void wait_for_ping (int fd)
      {
        if ( fhg::syscall::read
             (fd, &synchronization_buffer, sizeof (synchronization_buffer))
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: other side did not write");
        }
      }

      void ping (int fd)
      {
        if ( fhg::syscall::write
             (fd, &synchronization_buffer, sizeof (synchronization_buffer))
           != sizeof (synchronization_buffer)
           )
        {
          detail::do_error ("synchronization failed: other side did not read");
        }
      }
    }

    struct scoped_file
    {
      scoped_file (const char* name, int flags)
        : _fd (fhg::syscall::open (name, flags))
      {}
      ~scoped_file()
      {
        fhg::syscall::close (_fd);
      }
      int _fd;
    };
  }

  execute_return_type execute ( std::string const & command
                              , const_buffer const & buf_stdin
                              , buffer const & buf_stdout
                              , circular_buffer & buf_stderr
                              , file_const_buffer_list const & files_input
                              , file_buffer_list const & files_output
                              )
  {
    typedef std::list<std::unique_ptr<detail::tempfifo_t>> tempfifo_list_t;
    execute_return_type ret (files_input.size(), files_output.size());

    int in[2], out[2], err[2];
    fhg::syscall::pipe (in);
    fhg::syscall::pipe (out);
    fhg::syscall::pipe (err);

    {
      std::set<std::string> seen_params;
      for (file_const_buffer const& file : files_input)
      {
        if (!seen_params.insert (file.param()).second)
        {
          detail::put_error ("redefinition of key", file.param());
        }
      }
      for (file_buffer const& file : files_output)
      {
        if (!seen_params.insert (file.param()).second)
        {
          detail::put_error ("redefinition of key", file.param());
        }
      }
    }

    std::unordered_map <std::string, std::string> param_map;
    boost::thread_group writers;
    boost::thread_group readers;
    tempfifo_list_t tempfifos;

    std::size_t writer_i (0);
    for (file_const_buffer const& file_input : files_input)
      {
        std::string filename (detail::tempname());

        tempfifos.emplace_back
          (fhg::util::make_unique<detail::tempfifo_t> (filename));

        writers.add_thread
          ( new boost::thread
            ([filename, file_input, &ret, writer_i]
            {
              scoped_file const file
                (filename.c_str(), O_WRONLY);

              thread::writer ( file._fd
                             , file_input.buf()
                             , file_input.size()
                             , ret.bytes_written_files_input[writer_i]
                             );
            }
            )
          );
        param_map.emplace (file_input.param(), filename);
        ++writer_i;
      }

    std::size_t reader_i (0);
    for (file_buffer const& file_output : files_output)
      {
        std::string filename (detail::tempname());

        tempfifos.emplace_back
          (fhg::util::make_unique<detail::tempfifo_t> (filename));

        readers.add_thread
          ( new boost::thread
            ([filename, file_output, &ret, reader_i]
            {
              scoped_file const file
                (filename.c_str(), O_RDONLY);

              thread::reader ( file._fd
                             , file_output.buf()
                             , file_output.size()
                             , ret.bytes_read_files_output[reader_i]
                             );
            }
            )
          );

        param_map.emplace (file_output.param(), filename);
        ++reader_i;
      }

    scoped_SIGPIPE_block const sigpipe_block;

    int synchronization_fd_parent_child[2] = { -1, -1 };
    int synchronization_fd_child_parent[2] = { -1, -1 };
    fhg::syscall::pipe (synchronization_fd_parent_child);
    fhg::syscall::pipe (synchronization_fd_child_parent);

    pid_t pid (fhg::syscall::fork());

    if (pid == pid_t (0))
    {
      // child: should not produce any output on stdout/stderr
      detail::prepare_child_pipes ( in
                                  , out
                                  , err
                                  , synchronization_fd_parent_child
                                  , synchronization_fd_child_parent
                                  );

      sync::ping (synchronization_fd_child_parent[detail::WR]);

      // wait for parent setting up threads
      sync::wait_for_ping (synchronization_fd_parent_child[detail::RD]);

      fhg::syscall::close (synchronization_fd_parent_child[detail::RD]);
      fhg::syscall::close (synchronization_fd_child_parent[detail::WR]);

      std::vector<char> argv_buffer;
      std::vector<char*> argv;

      {
        std::vector<std::size_t> argv_offsets;

        for ( std::string raw_param
            : fhg::util::split<std::string, std::string> (command, ' ')
            )
        {
          const decltype (param_map)::const_iterator repl
            (param_map.find (raw_param));
          const std::string param ( (repl != param_map.end())
                                  ? std::string (repl->second)
                                  : raw_param
                                  );

          std::size_t pos (argv_buffer.size());
          argv_buffer.resize (argv_buffer.size() + param.size() + 1);
          std::copy (param.begin(), param.end(), argv_buffer.data() + pos);
          argv_buffer[argv_buffer.size() - 1] = '\0';
          argv_offsets.push_back (pos);
        }
        for (std::size_t offset : argv_offsets)
        {
          argv.push_back (argv_buffer.data() + offset);
        }
        argv.push_back (nullptr);
      }

      try
      {
        fhg::syscall::execvp (argv[0], argv.data());
      }
      catch (boost::system::system_error const& err)
      {
        fhg::syscall::close (STDIN_FILENO);
        fhg::syscall::close (STDOUT_FILENO);
        fhg::syscall::close (STDERR_FILENO);

        _exit ( err.code() == boost::system::errc::permission_denied ? 126
              : err.code() == boost::system::errc::no_such_file_or_directory ? 127
              : 254
              );
      }
    }
    else
    {
      // parent
      detail::prepare_parent_pipes ( out
                                   , err
                                   , synchronization_fd_parent_child
                                   , synchronization_fd_child_parent
                                   );

      // wait for child setting up pipes
      sync::wait_for_ping (synchronization_fd_child_parent[detail::RD]);

      //! \note threads get ownership of respective file descriptors
      struct close_on_scope_exit : boost::noncopyable
      {
        close_on_scope_exit (int fd)
          : _fd (fd)
        {}
        ~close_on_scope_exit()
        {
          fhg::syscall::close (_fd);
        }
        int _fd;
      };

      boost::thread thread_buf_stdin
        ( [&buf_stdin, &in, &ret]
        {
          close_on_scope_exit const _ (in[detail::WR]);
          thread::writer ( in[detail::WR]
                         , buf_stdin.buf(), buf_stdin.size()
                         , ret.bytes_written_stdin
                         );
        }
        );

      boost::thread thread_buf_stdout
        ( [&buf_stdout, &out, &ret]
        {
          close_on_scope_exit const _ (out[detail::RD]);
          thread::reader ( out[detail::RD]
                         , buf_stdout.buf(), buf_stdout.size()
                         , ret.bytes_read_stdout
                         );
        }
        );

      boost::thread thread_buf_stderr
        ( [&buf_stderr, &err, &ret]
        {
          close_on_scope_exit const _ (err[detail::RD]);
          thread::circular_reader
            (err[detail::RD], buf_stderr, ret.bytes_read_stderr);
        }
        );

      sync::ping (synchronization_fd_parent_child[detail::WR]);

      fhg::syscall::close (synchronization_fd_parent_child[detail::WR]);
      fhg::syscall::close (synchronization_fd_child_parent[detail::RD]);

      int status (0);

      fhg::syscall::waitpid (pid, &status, 0);

      thread_buf_stdin.interrupt();

      //! \note first consume, then subtract: bytes_written_stdin still
      //! grows while consuming!
      {
        const std::size_t consumed (consume_everything (in[detail::RD]));
        ret.bytes_written_stdin -= consumed;
      }
      fhg::syscall::close (in[detail::RD]);

      thread_buf_stdin.join();
      thread_buf_stdout.join();
      thread_buf_stderr.join();

      writers.interrupt_all();

      {
        std::size_t i (0);
        for (file_const_buffer const& file_input : files_input)
        {
          scoped_file const file
            (param_map.at (file_input.param()).c_str(), O_RDONLY | O_NONBLOCK);

          {
            const std::size_t consumed (consume_everything (file._fd));
            ret.bytes_written_files_input[i] -= consumed;
          }

          ++i;
        }
      }

      writers.join_all();
      readers.join_all();

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

    return ret;
  }
}
