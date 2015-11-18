#pragma once

#include <drts/worker/context_fwd.hpp>

#include <set>
#include <stdexcept>
#include <string>

#include <boost/function.hpp>

namespace drts
{
  namespace worker
  {
    class redirect_output;
    class context_constructor;

    class context
    {
    private:
      friend class redirect_output;
      friend class context_constructor;
      class implementation;
      implementation* _;

    public:
      context (context_constructor);
      ~context();

      std::string const& worker_name() const;

      std::set<std::string> const& workers() const;
      std::string worker_to_hostname (std::string const&) const;

      void module_call_do_cancel() const;

      void execute_and_kill_on_cancel
        ( boost::function<void()> fun
        , boost::function<void()> on_cancel
        , boost::function<void (int)> on_signal
        , boost::function<void (int)> on_exit
        );

      void execute_and_kill_on_cancel
        ( boost::function<void()> fun
        , boost::function<void()> on_cancel
        )
      {
        execute_and_kill_on_cancel
          ( fun
          , on_cancel
          , [] (int signal)
            {
              throw std::logic_error
                ("Unexpected on_signal (" + std::to_string (signal) + ")");
            }
          , [] (int exit_code)
            {
              throw std::logic_error
                ("Unexpected on_exit (" + std::to_string (exit_code) + ")");
            }
          );
      }

      struct cancelled : public std::exception {};

      void execute_and_kill_on_cancel (boost::function<void()> fun)
      {
        execute_and_kill_on_cancel
          ( fun
          , [] ()
            {
              throw cancelled();
            }
          );
      }
    };
  }
}
