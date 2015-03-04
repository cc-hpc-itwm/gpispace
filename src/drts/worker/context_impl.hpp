#pragma once

#include <drts/worker/context.hpp>

#include <fhglog/Logger.hpp>

#include <list>
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
                          , std::list<std::string> const& worker_list
                          , fhg::log::Logger& logger
                          );
      context::implementation* _;
    };

    class context::implementation
    {
    public:
      implementation ( std::string const& worker_name
                     , std::list<std::string> const& worker_list
                     , fhg::log::Logger& logger
                     );

      std::string const& worker_name() const;

      std::list<std::string> const& worker_list () const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (boost::function<void()> fun);
      void module_call_do_cancel() const;

      void log ( fhg::log::Level const& severity
               , std::string const& file
               , std::string const& function
               , std::size_t const& line
               , std::string const& message
               ) const;

    private:
      std::string _worker_name;
      std::list<std::string> _worker_list;
      boost::function<void()> _module_call_do_cancel;
      fhg::log::Logger& _logger;
    };
  }
}
