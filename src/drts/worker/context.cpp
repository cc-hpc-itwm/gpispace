// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/exit_status.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <boost/format.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/system/system_error.hpp>

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
    void context::execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
      ( std::function<void()> fun
      , std::function<void()> on_cancel
      , std::function<void (int)> on_signal
      , std::function<void (int)> on_exit
      )
    {
      _->execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN (fun, on_cancel, on_signal, on_exit);
    }

    context_constructor::context_constructor
      ( std::string const& worker_name
      , std::set<std::string> const& workers
      , fhg::logging::stream_emitter& logger
      )
        : _ (new context::implementation (worker_name, workers, logger))
    {}

    context::implementation::implementation
      ( std::string const& worker_name
      , std::set<std::string> const& workers
      , fhg::logging::stream_emitter& logger
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
      (std::string const& w) const
    {
      const std::string::size_type host_start = w.find ('-') + 1;
      const std::string::size_type host_end = w.find (' ', host_start);

      return w.substr (host_start, host_end-host_start);
    }
    void context::implementation::set_module_call_do_cancel
      (std::function<void()> fun)
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
    void context::implementation::log ( char const* const category
                                      , std::string const& message
                                      ) const
    {
      _logger.emit (message, category);
    }

    //! \todo factor out channel_from_child_to_parent, see
    //! execute_and_get_startup_messages, process::execute
    void context::implementation::execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
      ( std::function<void()> fun
      , std::function<void()> on_cancel
      , std::function<void (int)> on_signal
      , std::function<void (int)> on_exit
      )
    {
      int pipe_fds[2];
      fhg::util::syscall::pipe (pipe_fds);

      if (pid_t child = fhg::util::syscall::fork())
      {
        FHG_UTIL_FINALLY
          ([&pipe_fds]() {fhg::util::syscall::close (pipe_fds[0]); });
        fhg::util::syscall::close (pipe_fds[1]);

        bool cancelled {false};

        auto const module_call_do_cancel (_module_call_do_cancel);

        set_module_call_do_cancel
          ( [&child, &cancelled]
            {
              cancelled = true;

              try
              {
                fhg::util::syscall::kill (child, SIGUSR2);
              }
              catch (boost::system::system_error const& se)
              {
                // ignore: race with normally exiting child (waited below)
                if (se.code() != boost::system::errc::no_such_process)
                {
                  throw;
                }
              }
            }
          );

        FHG_UTIL_FINALLY
          ([&] { set_module_call_do_cancel (module_call_do_cancel); });

        int status;

        if (fhg::util::syscall::waitpid (child, &status, 0) != child)
        {
          throw std::logic_error
            ((boost::format ("wait (%1%) != %1%") % child).str());
        }

        if (fhg::util::wifsignaled (status))
        {
          if (cancelled && fhg::util::wtermsig (status) == SIGUSR2)
          {
            return on_cancel();
          }

          return on_signal (fhg::util::wtermsig (status));
        }
        else if (fhg::util::wifexited (status))
        {
          if (fhg::util::wexitstatus (status) == 1)
          {
            boost::iostreams::stream<boost::iostreams::file_descriptor_source>
              pipe_read (pipe_fds[0], boost::iostreams::never_close_handle);

            pipe_read >> std::noskipws;

            std::string const ex { std::istream_iterator<char> (pipe_read)
                                 , std::istream_iterator<char>()
                                 };

            if (ex.size())
            {
              std::rethrow_exception
                (fhg::util::serialization::exception::deserialize (ex));
            }
          }

          return on_exit (fhg::util::wexitstatus (status));
        }
        else
        {
          throw std::logic_error
            ("Unexpected exit status: " + std::to_string (status));
        }
      }
      else
      {
        FHG_UTIL_FINALLY
          ([&pipe_fds]() {fhg::util::syscall::close (pipe_fds[1]); });
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
            (pipe_fds[1], boost::iostreams::never_close_handle) <<
              fhg::util::serialization::exception::serialize
                (std::current_exception());

          exit (1);
        }
      }
    }

    void throw_cancelled()
    {
      throw context::cancelled();
    }

    void on_signal_unexpected (int signal)
    {
      throw std::logic_error
        ("Unexpected on_signal (" + std::to_string (signal) + ")");
    }
    void on_exit_unexpected (int exit_code)
    {
      throw std::logic_error
        ("Unexpected on_exit (" + std::to_string (exit_code) + ")");
    }
  }
}
