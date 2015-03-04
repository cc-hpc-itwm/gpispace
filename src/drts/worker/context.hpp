#pragma once

#include <drts/worker/context_fwd.hpp>

#include <fhglog/Logger.hpp>

#include <list>
#include <string>

#include <boost/function.hpp>

namespace drts
{
  namespace worker
  {
    class context_constructor;

    class context
    {
    private:
      friend class context_constructor;
      class implementation;
      implementation* _;

    public:
      context (context_constructor);
      ~context();

      std::string const& worker_name() const;

      std::list<std::string> const& worker_list() const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (boost::function<void()>);
      void module_call_do_cancel() const;

      fhg::log::Logger& logger() const;
    };
  }
}
