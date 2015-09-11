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
    std::set<std::string> const& context::workers () const
    {
      return _->workers();
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
  }
}
