#include <process.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/environment.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <unordered_map>

#include <fcntl.h>

namespace process
{
  namespace
  {
    union pipe_fds
    {
      int both[2];
      struct
      {
        int read;
        int write;
      };
    };

    void prepare_parent_pipes
      (pipe_fds out, pipe_fds err, pipe_fds sync_pc, pipe_fds sync_cp)
    {
      fhg::syscall::close (out.write);
      fhg::syscall::close (err.write);
      fhg::syscall::close (sync_pc.read);
      fhg::syscall::close (sync_cp.write);
    }

    void prepare_child_pipes
      (pipe_fds in, pipe_fds out, pipe_fds err, pipe_fds sync_pc, pipe_fds sync_cp)
    {
      fhg::syscall::close (in.write);
      fhg::syscall::close (out.read);
      fhg::syscall::close (err.read);
      fhg::syscall::close (sync_pc.write);
      fhg::syscall::close (sync_cp.read);

      if (in.read != STDIN_FILENO)
      {
        fhg::syscall::dup (in.read, STDIN_FILENO);
        fhg::syscall::close (in.read);
      }

      if (out.write != STDOUT_FILENO)
      {
        fhg::syscall::dup (out.write, STDOUT_FILENO);
        fhg::syscall::close (out.write);
      }

      if (err.write != STDERR_FILENO)
      {
        fhg::syscall::dup (err.write, STDERR_FILENO);
        fhg::syscall::close (err.write);
      }

      const int maximum_open_files (sysconf (_SC_OPEN_MAX));
      for (int i (0); i < maximum_open_files; ++i)
      {
        if ( i != sync_pc.read && i != sync_cp.write
          && i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO
           )
        {
          try
          {
            fhg::syscall::close (i);
          }
          catch (boost::system::system_error const& err)
          {
            if (err.code() == boost::system::errc::bad_file_descriptor)
            {
              // ignore: fd wasn't open
              continue;
            }
            throw;
          }
        }
      }
    }

    namespace thread
    {
      void circular_reader
        (int fd, circular_buffer& circ_buf, std::size_t& bytes_read)
      {
        std::array<char, PIPE_BUF> buf;

        bytes_read = 0;

        ssize_t r;
        do
        {
          r = fhg::syscall::read (fd, buf.data(), PIPE_BUF);
          bytes_read += r;
          std::copy (buf.data(), buf.data() + r, std::back_inserter (circ_buf));
        }
        while (r != 0);
      }

      void reader (int fd, buffer output, std::size_t& bytes_read)
      {
        char* buf (static_cast<char*> (output.buf()));

        bytes_read = 0;

        ssize_t r;
        do
        {
          const std::size_t to_read
            (std::min (std::size_t (PIPE_BUF), output.size() - bytes_read));

          r = fhg::syscall::read (fd, buf, to_read);
          buf += r;
          bytes_read += r;
        }
        while (r != 0);
      }

      void writer (int fd, const_buffer input, std::size_t& written)
      {
        const char* buf (static_cast<const char*> (input.buf()));

        written = 0;

        std::size_t bytes_left (input.size());
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
    }

    std::string make_unique_temporary_filename()
    {
      static struct
      {
        unsigned long operator++ (int)
        {
          boost::mutex::scoped_lock const _ (_mutex);
          return _counter++;
        }
        boost::mutex _mutex;
        unsigned long _counter = 0;
      } count;

      char* TMPDIR (fhg::syscall::getenv ("TMPDIR"));

      std::string dir (TMPDIR ? TMPDIR : P_tmpdir);

      if (dir.empty())
      {
        throw std::runtime_error ("neither TMPDIR nor P_tmpdir are set");
      }

      std::string fname;
      do
      {
        fname = ( boost::format ("%1%/process.%2%.%3%.%4%")
                % dir % fhg::syscall::getuid() % fhg::syscall::getpid() % count++
                ).str();
      }
      while (boost::filesystem::exists (fname));

      return fname;
    }

    struct tempfifo_t : boost::noncopyable
    {
      explicit tempfifo_t (std::string const& path)
        : _path (path)
      {
        fhg::syscall::mkfifo (_path.c_str(), S_IWUSR | S_IRUSR);
      }

      ~tempfifo_t()
      {
        fhg::syscall::unlink (_path.c_str());
      }

    private:
      std::string _path;
    };

    std::size_t consume_everything (int fd)
    {
      std::size_t consumed_accum (0);

      char buffer[PIPE_BUF];
      ssize_t consumed;
      do
      {
        consumed = fhg::syscall::read (fd, buffer, sizeof (buffer));
        consumed_accum += consumed;
      }
      while (consumed != 0);

      return consumed_accum;
    }

    struct scoped_SIGPIPE_block
    {
      scoped_SIGPIPE_block()
      {
        sigset_t signals_to_block;
        fhg::syscall::sigemptyset (&signals_to_block);
        fhg::syscall::sigaddset (&signals_to_block, SIGPIPE);
        fhg::syscall::pthread_sigmask
          (SIG_BLOCK, &signals_to_block, &_signals_to_restore);
      }
      ~scoped_SIGPIPE_block()
      {
        fhg::syscall::pthread_sigmask
          (SIG_UNBLOCK, &_signals_to_restore, nullptr);
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
          throw std::runtime_error
            ("synchronization failed: other side did not write");
        }
      }

      void ping (int fd)
      {
        if ( fhg::syscall::write
             (fd, &synchronization_buffer, sizeof (synchronization_buffer))
           != sizeof (synchronization_buffer)
           )
        {
          throw std::runtime_error
            ("synchronization failed: other side did not read");
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

  extern execute_return_type execute ( std::string const& command
                                     , const_buffer const& buf_stdin
                                     , buffer const& buf_stdout
                                     , circular_buffer& buf_stderr
                                     , file_const_buffer_list const& files_input
                                     , file_buffer_list const& files_output
                                     )
  {
    return execute ( command
                   , buf_stdin, buf_stdout, buf_stderr
                   , files_input, files_output
                   , fhg::util::environment ()
                   );
  }

  execute_return_type execute ( std::string const& command
                              , const_buffer const& buf_stdin
                              , buffer const& buf_stdout
                              , circular_buffer& buf_stderr
                              , file_const_buffer_list const& files_input
                              , file_buffer_list const& files_output
                              , std::map<std::string, std::string> const& environment
                              )
  {
    execute_return_type ret (files_output.size());

    pipe_fds in;
    pipe_fds out;
    pipe_fds err;
    fhg::syscall::pipe (in.both);
    fhg::syscall::pipe (out.both);
    fhg::syscall::pipe (err.both);

    {
      std::set<std::string> seen_params;
      for (file_const_buffer const& file : files_input)
      {
        if (!seen_params.insert (file.param()).second)
        {
          throw std::runtime_error ("redefinition of key: " + file.param());
        }
      }
      for (file_buffer const& file : files_output)
      {
        if (!seen_params.insert (file.param()).second)
        {
          throw std::runtime_error ("redefinition of key: " + file.param());
        }
      }
    }

    std::unordered_map <std::string, std::string> param_map;
    boost::thread_group writers;
    boost::thread_group readers;
    std::list<std::unique_ptr<tempfifo_t>> tempfifos;
    std::vector<boost::optional<std::string>> output_file_unopened
      (files_output.size(), boost::none);
    std::vector<boost::optional<std::string>> input_file_unopened
      (files_input.size(), boost::none);

    {
      std::size_t writer_i (0);
      for (file_const_buffer const& file_input : files_input)
      {
        const std::string filename (make_unique_temporary_filename());

        tempfifos.emplace_back (fhg::util::make_unique<tempfifo_t> (filename));

        writers.add_thread
          ( new boost::thread
            ( [filename, file_input, &ret, writer_i, &input_file_unopened]
            {
              scoped_SIGPIPE_block const sigpipe_block;

              input_file_unopened[writer_i] = filename;

              scoped_file const file (filename.c_str(), O_WRONLY);

              input_file_unopened[writer_i] = boost::none;

              //! \todo Maybe somehow also consume_everything from fifos.
              std::size_t dummy_written;
              thread::writer (file._fd, file_input.to_buffer(), dummy_written);
            }
            )
          );

        param_map.emplace (file_input.param(), filename);
        ++writer_i;
      }
    }

    {
      std::size_t reader_i (0);
      for (file_buffer const& file_output : files_output)
      {
        const std::string filename (make_unique_temporary_filename());

        tempfifos.emplace_back (fhg::util::make_unique<tempfifo_t> (filename));

        readers.add_thread
          ( new boost::thread
            ( [filename, file_output, &ret, reader_i, &output_file_unopened]
            {
              output_file_unopened[reader_i] = filename;

              scoped_file const file
                (filename.c_str(), O_RDONLY);

              output_file_unopened[reader_i] = boost::none;

              thread::reader ( file._fd
                             , file_output.to_buffer()
                             , ret.bytes_read_files_output[reader_i]
                             );
            }
            )
          );

        param_map.emplace (file_output.param(), filename);
        ++reader_i;
      }
    }


    pipe_fds synchronization_fd_parent_child;
    pipe_fds synchronization_fd_child_parent;
    fhg::syscall::pipe (synchronization_fd_parent_child.both);
    fhg::syscall::pipe (synchronization_fd_child_parent.both);

    const pid_t pid (fhg::syscall::fork());

    if (pid == pid_t (0))
    {
      // child: should not produce any output on stdout/stderr
      prepare_child_pipes ( in
                          , out
                          , err
                          , synchronization_fd_parent_child
                          , synchronization_fd_child_parent
                          );

      sync::ping (synchronization_fd_child_parent.write);

      // wait for parent setting up threads
      sync::wait_for_ping (synchronization_fd_parent_child.read);

      fhg::syscall::close (synchronization_fd_parent_child.read);
      fhg::syscall::close (synchronization_fd_child_parent.write);


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
          const std::string param
            ((repl != param_map.end()) ? std::string (repl->second) : raw_param);

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

      std::vector<char> envp_buffer;
      std::vector<char*> envp;

      {
        std::vector<std::size_t> envp_offsets;

        for (const std::pair<std::string, std::string> entry : environment)
        {
          std::size_t pos (envp_buffer.size ());
          const std::string raw_entry (entry.first + "=" + entry.second);
          envp_buffer.resize (envp_buffer.size () + raw_entry.size () + 1);
          std::copy (raw_entry.begin (), raw_entry.end (), envp_buffer.data () + pos);
          envp_buffer [envp_buffer.size () - 1] = '\0';
          envp_offsets.push_back (pos);
        }
        for (std::size_t offset : envp_offsets)
        {
          envp.push_back (envp_buffer.data () + offset);
        }
        envp.push_back (nullptr);
      }

      try
      {
        fhg::syscall::execvpe (argv[0], argv.data(), envp.data());
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
      prepare_parent_pipes ( out
                           , err
                           , synchronization_fd_parent_child
                           , synchronization_fd_child_parent
                           );

      // wait for child setting up pipes
      sync::wait_for_ping (synchronization_fd_child_parent.read);

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
          scoped_SIGPIPE_block const sigpipe_block;

          close_on_scope_exit const _ (in.write);
          thread::writer (in.write, buf_stdin, ret.bytes_written_stdin);
        }
        );

      boost::thread thread_buf_stdout
        ( [&buf_stdout, &out, &ret]
        {
          close_on_scope_exit const _ (out.read);
          thread::reader (out.read, buf_stdout, ret.bytes_read_stdout);
        }
        );

      boost::thread thread_buf_stderr
        ( [&buf_stderr, &err, &ret]
        {
          close_on_scope_exit const _ (err.read);
          thread::circular_reader (err.read, buf_stderr, ret.bytes_read_stderr);
        }
        );

      sync::ping (synchronization_fd_parent_child.write);

      fhg::syscall::close (synchronization_fd_parent_child.write);
      fhg::syscall::close (synchronization_fd_child_parent.read);


      int status (0);

      fhg::syscall::waitpid (pid, &status, 0);

      thread_buf_stdin.interrupt();

      //! \note first consume, then subtract: bytes_written_stdin is
      //! also written to in the writer thread. the thread will stop
      //! modifying it as soon as comsume_everything returned. thus, a
      //! sequence point is required.
      {
        const std::size_t consumed (consume_everything (in.read));
        ret.bytes_written_stdin -= consumed;
      }
      fhg::syscall::close (in.read);

      thread_buf_stdin.join();
      thread_buf_stdout.join();
      thread_buf_stderr.join();

      for (boost::optional<std::string> const& e : input_file_unopened)
      {
        if (e)
        {
          scoped_file const file (e->c_str(), O_RDONLY);
        }
      }
      for (boost::optional<std::string> const& e : output_file_unopened)
      {
        if (e)
        {
          scoped_file const file (e->c_str(), O_WRONLY);
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
        throw std::runtime_error
          ("strange child status: " + std::to_string (status));
      }
    }

    return ret;
  }
}
