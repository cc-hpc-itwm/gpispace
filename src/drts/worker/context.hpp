#pragma once

#include <drts/worker/context_fwd.hpp>

#include <fhglog/level.hpp>

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

      void log ( fhg::log::Level const& severity
               , std::string const& file
               , std::string const& function
               , std::size_t const& line
               , std::string const& message
               ) const;
    };
  }
}

#include <boost/current_function.hpp>
#include <sstream>

#define GSPC_LOG(_severity, _message)                   \
  do                                                    \
  {                                                     \
    std::ostringstream message;                         \
    message << _message;                                \
    _pnetc_context->log ( fhg::log::_severity           \
                        , __FILE__                      \
                        , BOOST_CURRENT_FUNCTION        \
                        , __LINE__                      \
                        , message.str()                 \
                        );                              \
  } while (0)
