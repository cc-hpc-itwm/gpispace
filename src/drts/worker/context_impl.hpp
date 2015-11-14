#pragma once

#include <drts/worker/context.hpp>

#include <fhglog/Logger.hpp>

#include <util-generic/ostream/redirect.hpp>

#include <set>
#include <string>

#include <boost/function.hpp>

namespace drts
{
  namespace worker
  {
    class context_constructor
    {
    public:
      context_constructor ( std::string const& worker_name
                          , std::set<std::string> const& workers
                          , fhg::log::Logger& logger
                          );
      context::implementation* _;
    };

    class context::implementation
    {
    public:
      implementation ( std::string const& worker_name
                     , std::set<std::string> const& workers
                     , fhg::log::Logger& logger
                     );

      std::string const& worker_name() const;

      std::set<std::string> const& workers () const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (boost::function<void()> fun);
      void module_call_do_cancel();

      void execute_and_kill_on_cancel
        ( boost::function<void()> fun
        , boost::function<void()> on_cancel
        , boost::function<void (int)> on_signal
        , boost::function<void (int)> on_exit
        );

      void log ( fhg::log::Level const& severity
               , std::string const& message
               ) const;

    private:
      std::string _worker_name;
      std::set<std::string> _workers;
      boost::function<void()> _module_call_do_cancel;
      bool _cancelled;
      fhg::log::Logger& _logger;
    };

    class redirect_output : public fhg::util::ostream::redirect
    {
    public:
      redirect_output ( drts::worker::context const* const context
                      , fhg::log::Level const& severity
                      , std::ostream& os
                      )
        : redirect
          ( os
          , [context, severity] (std::string const& line)
            {
              context->_->log (severity, line);
            }
          )
      {}
    };
  }
}
