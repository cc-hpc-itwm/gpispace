#include <drts/worker/context.hpp>

namespace drts
{
  namespace worker
  {
    context::context ( std::string const &worker_name
                     , std::list<std::string> const &worker_list
                     , fhg::log::Logger& logger
                     )
      : m_worker_name (worker_name)
      , m_worker_list (worker_list)
      , _module_call_do_cancel ([](){})
      , _logger (logger)
    {}

    std::string const& context::worker_name() const
    {
      return m_worker_name;
    }

    std::list<std::string> const& context::worker_list() const
    {
      return m_worker_list;
    }

    std::string context::worker_to_hostname (std::string const & w) const
    {
      const std::string::size_type host_start = w.find ('-') + 1;
      const std::string::size_type host_end = w.find (' ', host_start);

      return w.substr (host_start, host_end-host_start);
    }

    void context::set_module_call_do_cancel (boost::function<void()> fun)
    {
      _module_call_do_cancel = fun;
    }
    void context::module_call_do_cancel()
    {
      _module_call_do_cancel();
    }

    fhg::log::Logger& context::logger() const
    {
      return _logger;
    }
  }
}
