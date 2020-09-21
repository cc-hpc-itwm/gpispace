#include <iml/rif/execute_and_get_startup_messages.hpp>

#include <iml/rif/started_process_promise.hpp>

#include <util-generic/serialization/exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/temporary_file.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace fhg
{
  namespace iml
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
          if (buffer.size() < str.size() + pos)
          {
            throw std::logic_error ("append assumes preallocated buffer");
          }
          std::copy (str.begin(), str.end(), buffer.begin() + pos);
          return pos + str.size();
        }
        size_t append (std::vector<char>& buffer, char c, size_t pos)
        {
          if (buffer.size() < sizeof (char) + 1)
          {
            throw std::logic_error ("append assumes preallocated buffer");
          }
          *(buffer.begin() + pos) = c;
          return pos + 1;
        }

        std::vector<char*> prepare_argv
          ( std::vector<char>& argv_buffer
          , boost::filesystem::path const& command
          , int pipe_fd
          , std::vector<std::string> const& arguments
          )
        {
          std::string pipe_fd_string (std::to_string (pipe_fd));

          argv_buffer.resize
            ( command.string().size() + 1
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
          argv.reserve (arguments.size() + 2 + 1);

          argv.push_back (argv_buffer.data() + argv_pos);
          argv_pos = append (argv_buffer, command.string(), argv_pos);
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

          for (auto const& entry : environment)
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

          pipe_read >> std::noskipws;

          std::string const message_data
            { std::istream_iterator<char> (pipe_read)
            , std::istream_iterator<char>()
            };

          std::string const expected_end
            (started_process_promise::end_sentinel_value());

          bool const is_complete
            ( message_data.size() >= expected_end.size()
            && std::equal ( expected_end.rbegin()
                          , expected_end.rend()
                          , message_data.rbegin()
                          )
            );

          if (!is_complete)
          {
            int child_status (0);
            if (util::syscall::waitpid (pid, &child_status, 0) != pid)
            {
              throw std::logic_error ("waitpid (pid) != pid");
            }

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

            throw std::runtime_error ("child exited: unknown status");
          }

          std::istringstream stream (message_data);
          boost::archive::text_iarchive archive (stream);
          bool result;
          archive & result;

          if (!result)
          {
            std::string data;
            archive & data;

            std::rethrow_exception
              (fhg::util::serialization::exception::deserialize (data));
          }

          //! \todo types from startup_data
          std::vector<std::string> messages;
          archive & messages;
          return {pid, std::move (messages)};
        }
        else
        {
          util::syscall::close (pipe_fds[0]);

          std::vector<char> argv_buffer;
          std::vector<char*> const argv
            ( prepare_argv ( argv_buffer
                           , command
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
}
