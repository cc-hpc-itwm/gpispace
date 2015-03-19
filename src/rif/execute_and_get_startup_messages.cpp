// bernd.loerwald@itwm.fraunhofer.de

#include <rif/execute_and_get_startup_messages.hpp>

#include <rif/startup_messages_pipe.hpp>

#include <fhg/assert.hpp>
#include <util-generic/syscall.hpp>
#include <fhg/util/temporary_file.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <system_error>

namespace fhg
{
  namespace rif
  {
    namespace
    {
      void close_if_open (int fd)
      {
        try
        {
          util::syscall::close (fd);
        }
        catch (boost::system::system_error const& err)
        {
          if (err.code() == boost::system::errc::bad_file_descriptor)
          {
            // ignore: fd wasn't open
            return;
          }
          throw;
        }
      }

      size_t append (std::vector<char>& buffer, std::string const& str, size_t pos)
      {
        fhg_assert (buffer.size() >= str.size() + pos);
        std::copy (str.begin(), str.end(), buffer.begin() + pos);
        return pos + str.size();
      }
      size_t append (std::vector<char>& buffer, char c, size_t pos)
      {
        fhg_assert (buffer.size() >= sizeof (char) + pos);
        *(buffer.begin() + pos) = c;
        return pos + 1;
      }

      std::vector<char*> prepare_argv
        ( std::vector<char>& argv_buffer
        , boost::filesystem::path const& command
        , std::string const& startup_messages_pipe_option
        , int pipe_fd
        , std::vector<std::string> const& arguments
        )
      {
        std::string pipe_fd_string (std::to_string (pipe_fd));

        argv_buffer.resize
          ( command.string().size() + 1
          + startup_messages_pipe_option.size() + 1
          + pipe_fd_string.size() + 1
          + std::accumulate
              ( arguments.begin()
              , arguments.end()
              , std::size_t (0)
              , [] (std::size_t s, std::string const& argument)
                {
                  return s + argument.size() + 1;
                }
              )
          );

        std::size_t argv_pos (0);

        std::vector<char*> argv;
        argv.reserve (arguments.size() + 3 + 1);

        argv.push_back (argv_buffer.data() + argv_pos);
        argv_pos = append (argv_buffer, command.string(), argv_pos);
        argv_pos = append (argv_buffer, '\0', argv_pos);

        argv.push_back (argv_buffer.data() + argv_pos);
        argv_pos = append (argv_buffer, startup_messages_pipe_option, argv_pos);
        argv_pos = append (argv_buffer, '\0', argv_pos);

        argv.push_back (argv_buffer.data() + argv_pos);
        argv_pos = append (argv_buffer, pipe_fd_string, argv_pos);
        argv_pos = append (argv_buffer, '\0', argv_pos);

        for (std::string const& argument : arguments)
        {
          argv.push_back (argv_buffer.data() + argv_pos);
          argv_pos = append (argv_buffer, argument, argv_pos);
          argv_pos = append (argv_buffer, '\0', argv_pos);
        }

        argv.push_back (nullptr);

        return argv;
      }

      std::vector<char*> prepare_envp
        ( std::vector<char>& envp_buffer
        , std::unordered_map<std::string, std::string> const& environment
        )
      {
        envp_buffer.resize
          ( std::accumulate
              ( environment.begin()
              , environment.end()
              , std::size_t (0)
              , [] (std::size_t s, std::pair<std::string, std::string> const& entry)
                {
                  return s + entry.first.size() + 1 + entry.second.size() + 1;
                }
              )
          );

        std::size_t envp_pos (0);

        std::vector<char*> envp;
        envp.reserve (environment.size() + 1);

        for (std::pair<std::string, std::string> const& entry : environment)
        {
          envp.push_back (envp_buffer.data() + envp_pos);
          envp_pos = append (envp_buffer, entry.first, envp_pos);
          envp_pos = append (envp_buffer, '=', envp_pos);
          envp_pos = append (envp_buffer, entry.second, envp_pos);
          envp_pos = append (envp_buffer, '\0', envp_pos);
        }

        envp.push_back (nullptr);

        return envp;
      }
    }

    std::pair<pid_t, std::vector<std::string>> execute_and_get_startup_messages
      ( boost::filesystem::path command
      , std::vector<std::string> arguments
      , std::unordered_map<std::string, std::string> environment
      )
    {
      int pipe_fds[2];
      util::syscall::pipe (pipe_fds);

      pid_t const pid (util::syscall::fork());

      if (pid)
      {
        util::syscall::close (pipe_fds[1]);

        boost::iostreams::stream<boost::iostreams::file_descriptor_source>
          pipe_read (pipe_fds[0], boost::iostreams::close_handle);

        std::vector<std::string> messages;
        std::string line;
        while ( std::getline (pipe_read, line)
              && line != startup_messages_pipe::end_sentinel_value()
              )
        {
          messages.emplace_back (std::move (line));
        }

        if (line != startup_messages_pipe::end_sentinel_value())
        {
          int child_status (0);
          if (util::syscall::waitpid (pid, &child_status, WNOHANG) == pid)
          {
            if (WIFSIGNALED (child_status))
            {
              throw std::runtime_error
                ("child signalled: " + std::to_string (WTERMSIG (child_status)));
            }
            else if (WIFEXITED (child_status))
            {
              switch (WEXITSTATUS (child_status))
              {
              case 241:
                throw std::system_error
                  (std::make_error_code (std::errc::argument_list_too_long));
              case 242:
                throw std::system_error
                  (std::make_error_code (std::errc::filename_too_long));
              case 243:
                throw std::system_error
                  (std::make_error_code (std::errc::invalid_argument));
              case 244:
                throw std::system_error
                  (std::make_error_code (std::errc::no_such_file_or_directory));
              case 245:
                throw std::system_error
                  (std::make_error_code (std::errc::not_a_directory));
              case 246:
                throw std::system_error
                  (std::make_error_code (std::errc::permission_denied));
              case 247:
                throw std::system_error
                  (std::make_error_code (std::errc::too_many_symbolic_link_levels));
              case 240:
                throw std::runtime_error
                  ("execve failed: unknown error");
              default:
                throw std::runtime_error
                  ("child exited: " + std::to_string (WEXITSTATUS (child_status)));
              }
            }
            //! \note can't really happen, but just fall through to
            //! generic exception
          }

          throw std::runtime_error ("pipe closed before end-sentinel-value read");
        }

        return {pid, std::move (messages)};
      }
      else
      {
        util::syscall::close (pipe_fds[0]);

        std::vector<char> argv_buffer;
        std::vector<char*> const argv
          ( prepare_argv ( argv_buffer
                         , command
                         , std::string ("--") + startup_messages_pipe::option_name()
                         , pipe_fds[1]
                         , arguments
                         )
          );

        std::vector<char> envp_buffer;
        std::vector<char*> const envp (prepare_envp (envp_buffer, environment));

        long const maximum_open_files (util::syscall::sysconf (_SC_OPEN_MAX));
        for (int fd (0); fd < maximum_open_files; ++fd)
        {
          if (fd == pipe_fds[1])
          {
            continue;
          }
          close_if_open (fd);
        }

        try
        {
          util::syscall::execve (command.string().c_str(), argv.data(), envp.data());
        }
        catch (boost::system::system_error const& err)
        {
          _exit
            ( err.code() == boost::system::errc::argument_list_too_long ? 241
            : err.code() == boost::system::errc::filename_too_long ? 242
            : err.code() == boost::system::errc::invalid_argument ? 243
            : err.code() == boost::system::errc::no_such_file_or_directory ? 244
            : err.code() == boost::system::errc::not_a_directory ? 245
            : err.code() == boost::system::errc::permission_denied ? 246
            : err.code() == boost::system::errc::too_many_symbolic_link_levels ? 247
            : 240
            );
        }
      }
    }
  }
}
