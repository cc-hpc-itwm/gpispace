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
    class context
    {
    public:
      context ( std::string const& worker_name
              , std::list<std::string> const& worker_list
              , fhg::log::Logger& logger
              );

      std::string const& worker_name() const;

      std::list<std::string> const& worker_list () const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (boost::function<void()> fun);
      void module_call_do_cancel();

      fhg::log::Logger& logger() const;

    private:
      std::string m_worker_name;
      std::list<std::string> m_worker_list;
      boost::function<void()> _module_call_do_cancel;
      fhg::log::Logger& _logger;
    };
  }
}
