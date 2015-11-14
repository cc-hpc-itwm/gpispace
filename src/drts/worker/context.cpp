#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <boost/format.hpp>

#include <stdexcept>
#include <iostream>

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
    }
    void context::implementation::module_call_do_cancel() const
    {
      _module_call_do_cancel();
    }
    void context::implementation::log ( fhg::log::Level const& severity
                                      , std::string const& message
                                      ) const
    {
      _logger.log (fhg::log::LogEvent (severity, message));
    }

    void context::implementation::execute_and_kill_on_cancel
      ( boost::function<void()> fun
      , boost::function<void()> on_cancel
      , boost::function<void (int)> on_signal
      , boost::function<void (int)> on_exit
      )
    {
      if (pid_t child = fhg::util::syscall::fork())
      {
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
          std::cerr << fhg::util::current_exception_printer().string() << '\n';

          exit (1);
        }
      }
    }
  }
}
