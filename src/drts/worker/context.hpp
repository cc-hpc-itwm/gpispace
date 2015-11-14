#pragma once

#include <drts/worker/context_fwd.hpp>

#include <set>
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
    };
  }
}
