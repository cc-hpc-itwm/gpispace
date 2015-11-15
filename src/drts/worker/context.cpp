#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/serialization/exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <boost/format.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

namespace drts
{
  namespace worker
  {
    context::context (context_constructor ctor)
      : _ (ctor._)
    {}
    context::~context()
    {
      delete _;
      _ = nullptr;
    }
    std::string const& context::worker_name() const
    {
      return _->worker_name();
    }
    std::set<std::string> const& context::workers () const
    {
      return _->workers();
    }
    std::string context::worker_to_hostname (std::string const& worker) const
    {
      return _->worker_to_hostname (worker);
    }
    void context::module_call_do_cancel() const
    {
      _->module_call_do_cancel();
    }
    void context::execute_and_kill_on_cancel
      ( boost::function<void()> fun
      , boost::function<void()> on_cancel
      , boost::function<void (int)> on_signal
      , boost::function<void (int)> on_exit
      )
    {
      _->execute_and_kill_on_cancel (fun, on_cancel, on_signal, on_exit);
    }

    context_constructor::context_constructor
      ( std::string const& worker_name
      , std::set<std::string> const& workers
      , fhg::log::Logger& logger
      )
        : _ (new context::implementation (worker_name, workers, logger))
    {}

    context::implementation::implementation
      ( std::string const &worker_name
      , std::set<std::string> const& workers
      , fhg::log::Logger& logger
      )
        : _worker_name (worker_name)
        , _workers (workers)
        , _module_call_do_cancel ([](){})
        , _cancelled (false)
        , _logger (logger)
    {}
    std::string const& context::implementation::worker_name() const
    {
      return _worker_name;
    }
    std::set<std::string> const& context::implementation::workers() const
    {
      return _workers;
    }
    std::string context::implementation::worker_to_hostname
      (std::string const & w) const
    {
      const std::string::size_type host_start = w.find ('-') + 1;
      const std::string::size_type host_end = w.find (' ', host_start);

      return w.substr (host_start, host_end-host_start);
    }
    void context::implementation::set_module_call_do_cancel
      (boost::function<void()> fun)
    {
      _module_call_do_cancel = fun;
      if (_cancelled)
      {
        module_call_do_cancel();
      }
    }
    void context::implementation::module_call_do_cancel()
    {
      _cancelled = true;
      _module_call_do_cancel();
    }
    void context::implementation::log ( fhg::log::Level const& severity
                                      , std::string const& message
                                      ) const
    {
      _logger.log (fhg::log::LogEvent (severity, message));
    }

    //! \todo factor out channel_from_child_to_parent, see
    //! execute_and_get_startup_messages, process::execute
    void context::implementation::execute_and_kill_on_cancel
      ( boost::function<void()> fun
      , boost::function<void()> on_cancel
      , boost::function<void (int)> on_signal
      , boost::function<void (int)> on_exit
      )
    {
      int pipe_fds[2];
      fhg::util::syscall::pipe (pipe_fds);

      if (pid_t child = fhg::util::syscall::fork())
      {
        fhg::util::syscall::close (pipe_fds[1]);

        bool cancelled {false};

        set_module_call_do_cancel
          ( [&child, &cancelled]
            {
              cancelled = true;

              fhg::util::syscall::kill (child, SIGUSR2);
            }
          );

        int status;

        if (fhg::util::syscall::waitpid (child, &status, 0) != child)
        {
          throw std::logic_error
            ((boost::format ("wait (%1%) != %1%") % child).str());
        }

        if (WIFSIGNALED (status))
        {
          if (cancelled && WTERMSIG (status) == SIGUSR2)
          {
            return on_cancel();
          }

          return on_signal (WTERMSIG (status));
        }
        else if (WIFEXITED (status))
        {
          if (WEXITSTATUS (status) == 1)
          {
            boost::iostreams::stream<boost::iostreams::file_descriptor_source>
              pipe_read (pipe_fds[0], boost::iostreams::close_handle);

            pipe_read >> std::noskipws;

            std::string const ex { std::istream_iterator<char> (pipe_read)
                                 , std::istream_iterator<char>()
                                 };

            std::rethrow_exception
              ( fhg::util::serialization::exception::deserialize
                ( ex
                , fhg::util::serialization::exception::serialization_functions()
                , fhg::util::serialization::exception::aggregated_serialization_functions()
                )
              );
          }

          return on_exit (WEXITSTATUS (status));
        }
        else
        {
          throw std::logic_error
            ("Unexpected exit status" + std::to_string (status));
        }
      }
      else
      {
        fhg::util::syscall::close (pipe_fds[0]);

        //! \note block to avoid "normal" exit due to external signal
        fhg::util::syscall::process_signal_block const process_signal_block
          {fhg::util::syscall::signal_set ({SIGINT, SIGTERM})};

        try
        {
          fun();

          exit (0);
        }
        catch (...)
        {
          boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
            (pipe_fds[1], boost::iostreams::close_handle) <<
              fhg::util::serialization::exception::serialize
                ( std::current_exception()
                , fhg::util::serialization::exception::serialization_functions()
                , fhg::util::serialization::exception::aggregated_serialization_functions()
                );

          exit (1);
        }
      }
    }
  }
}
