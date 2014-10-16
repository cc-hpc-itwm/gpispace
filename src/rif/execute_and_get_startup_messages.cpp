// bernd.loerwald@itwm.fraunhofer.de

#include <rif/execute_and_get_startup_messages.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/temporary_file.hpp>

#include <boost/optional.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <system_error>

#include <signal.h>

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
          fhg::syscall::close (fd);
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
        std::copy (str.begin(), str.end(), buffer.begin() + pos);
        return pos + str.size();
      }
      size_t append (std::vector<char>& buffer, char c, size_t pos)
      {
        *(buffer.begin() + pos) = c;
        return pos + 1;
      }

      std::vector<char*> prepare_argv
        ( std::vector<char>& argv_buffer
        , boost::filesystem::path const& command
        , std::string const& startup_messages_fifo_option
        , boost::filesystem::path const& fifo_path
        , std::vector<std::string> const& arguments
        )
      {
        argv_buffer.resize
          ( command.string().size() + 1
          + startup_messages_fifo_option.size() + 1
          + fifo_path.string().size() + 1
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
        argv_pos = append (argv_buffer, startup_messages_fifo_option, argv_pos);
        argv_pos = append (argv_buffer, '\0', argv_pos);

        argv.push_back (argv_buffer.data() + argv_pos);
        argv_pos = append (argv_buffer, fifo_path.string(), argv_pos);
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

      std::mutex GLOBAL_signal_handler_mutex;
      std::function<void (siginfo_t*)> GLOBAL_signal_handler;
      void GLOBAL_forwarding_signal_handler (int, siginfo_t* info, void*)
      {
        std::unique_lock<std::mutex> const _ (GLOBAL_signal_handler_mutex);
        GLOBAL_signal_handler (info);
      }
    }

    std::pair<pid_t, std::vector<std::string>> execute_and_get_startup_messages
      ( boost::filesystem::path const& fifo_directory
      , std::string const& startup_messages_fifo_option
      , std::string const& end_sentinel_value
      , boost::filesystem::path const& command
      , std::vector<std::string> const& arguments
      , std::unordered_map<std::string, std::string> const& environment
      )
    {
      boost::filesystem::path const fifo_path
        (fifo_directory / boost::filesystem::unique_path());

      boost::optional<std::pair<bool, int>> child_status;
      //! \todo boost 1.56: optional<future>
      std::future<void> fifo_out_opened;

      struct sigaction old_sigact;

      GLOBAL_signal_handler =
        [&child_status, &fifo_path, &fifo_out_opened] (siginfo_t* sig_info)
        {
          if (  sig_info->si_code != CLD_EXITED
             && sig_info->si_code != CLD_KILLED
             && sig_info->si_code != CLD_DUMPED
             )
          {
            return;
          }

          child_status = std::make_pair
          (sig_info->si_code == CLD_EXITED, sig_info->si_status);
          //! \note The signal is handled in same thread as fifo is
          //!       opened for reading, thus a second thread needs to
          //!       open for writing to wake up this thread.
          fifo_out_opened = std::async ( std::launch::async
                                       , [&fifo_path]
                                         {
                                           std::ofstream (fifo_path.string());
                                         }
                                       );
        };
      {
        struct sigaction sigact;
        memset (&sigact, 0, sizeof (sigact));
        sigact.sa_sigaction = GLOBAL_forwarding_signal_handler;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;
        fhg::syscall::sigaction (SIGCHLD, &sigact, &old_sigact);
      }

      fhg::syscall::mkfifo (fifo_path.string().c_str(), S_IRUSR | S_IWUSR);
      fhg::util::temporary_file const fifo_deleter (fifo_path);

      pid_t const pid (fhg::syscall::fork());

      if (pid)
      {
        std::ifstream fifo_in (fifo_path.string());

        //! \note either already hit or hitting can be ignored: if it
        //!       fails before sending all startup messages, the
        //!       sentinel check will fail and detect the bad
        //!       behavior. if it exits after sending all messages,
        //!       everything is fine.
        {
          fhg::syscall::sigaction (SIGCHLD, &old_sigact, nullptr);
          std::unique_lock<std::mutex> const _ (GLOBAL_signal_handler_mutex);
          GLOBAL_signal_handler = nullptr;
        }

        if (fifo_out_opened.valid())
        {
          fifo_out_opened.get();
          if (child_status->first)
          {
            switch (child_status->second)
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
                ("child exited: " + std::to_string (child_status->second));
            }
          }
          else
          {
            throw std::runtime_error
              ("child signalled: " + std::to_string (child_status->second));
          }
        }

        std::vector<std::string> messages;
        std::string line;
        while (std::getline (fifo_in, line) && line != end_sentinel_value)
        {
          messages.emplace_back (std::move (line));
        }

        if (line != end_sentinel_value)
        {
          throw std::runtime_error ("fifo closed before end-sentinel-value read");
        }

        return {pid, std::move (messages)};
      }
      else
      {
        std::vector<char> argv_buffer;
        std::vector<char*> const argv
          ( prepare_argv ( argv_buffer
                         , command
                         , startup_messages_fifo_option
                         , fifo_path
                         , arguments
                         )
          );

        std::vector<char> envp_buffer;
        std::vector<char*> const envp (prepare_envp (envp_buffer, environment));

        long const maximum_open_files (fhg::syscall::sysconf (_SC_OPEN_MAX));
        for (int fd (0); fd < maximum_open_files; ++fd)
        {
          close_if_open (fd);
        }

        try
        {
          fhg::syscall::execve (command.string().c_str(), argv.data(), envp.data());
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
