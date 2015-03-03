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
    private:
      static void nop() {}

    public:
      context ( std::string const &worker_name
              , std::list<std::string> const &worker_list
              , fhg::log::Logger& logger
              )
        : m_worker_name (worker_name)
        , m_worker_list (worker_list)
        , _module_call_do_cancel (nop)
        , _logger (logger)
      {}

      const std::string &worker_name () const { return m_worker_name; }

      const std::list<std::string> &worker_list () const { return m_worker_list; }
      std::string worker_to_hostname (std::string const &w) const
      {
        const std::string::size_type host_start = w.find ('-') + 1;
        const std::string::size_type host_end = w.find (' ', host_start);

        return w.substr (host_start, host_end-host_start);
      }

      void set_module_call_do_cancel (boost::function<void()> fun)
      {
        _module_call_do_cancel = fun;
      }
      void module_call_do_cancel()
      {
        _module_call_do_cancel();
      }

      fhg::log::Logger& logger() const
      {
        return _logger;
      }

    private:
      std::string m_worker_name;
      std::list<std::string> m_worker_list;
      boost::function<void()> _module_call_do_cancel;
      fhg::log::Logger& _logger;
    };
  }
}
