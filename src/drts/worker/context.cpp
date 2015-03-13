#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

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
    std::list<std::string> const& context::worker_list () const
    {
      return _->worker_list();
    }
    std::string context::worker_to_hostname (std::string const& worker) const
    {
      return _->worker_to_hostname (worker);
    }
    void context::set_module_call_do_cancel (boost::function<void()> f)
    {
      _->set_module_call_do_cancel (f);
    }
    void context::module_call_do_cancel() const
    {
      _->module_call_do_cancel();
    }
    void context::log ( fhg::log::Level const& severity
                      , std::string const& file
                      , std::string const& function
                      , std::size_t const& line
                      , std::string const& message
                      ) const
    {
      _->log (severity, file, function, line, message);
    }

    context_constructor::context_constructor
      ( std::string const& worker_name
      , std::list<std::string> const& worker_list
      , fhg::log::Logger& logger
      )
        : _ (new context::implementation (worker_name, worker_list, logger))
    {}

    context::implementation::implementation
      ( std::string const &worker_name
      , std::list<std::string> const &worker_list
      , fhg::log::Logger& logger
      )
        : _worker_name (worker_name)
        , _worker_list (worker_list)
        , _module_call_do_cancel ([](){})
        , _logger (logger)
    {}
    std::string const& context::implementation::worker_name() const
    {
      return _worker_name;
    }
    std::list<std::string> const& context::implementation::worker_list() const
    {
      return _worker_list;
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
                                      , std::string const& file
                                      , std::string const& function
                                      , std::size_t const& line
                                      , std::string const& message
                                      ) const
    {
      _logger.log
        (fhg::log::LogEvent (severity, file, function, line, message));
    }
  }
}
