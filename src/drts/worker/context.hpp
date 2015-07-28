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

    typedef boost::function<void ( fhg::log::Level const& severity
                                 , std::string const& file
                                 , std::string const& function
                                 , std::size_t const& line
                                 , std::string const& message
                                 )> logger_type;
  }
}

#include <boost/current_function.hpp>
#include <sstream>

#define GSPC_LLOG(_severity, _message, _logger)         \
  _logger ( fhg::log::_severity                         \
          , __FILE__                                    \
          , BOOST_CURRENT_FUNCTION                      \
          , __LINE__                                    \
          , (static_cast<std::ostringstream&>           \
              (std::ostringstream() << _message)        \
            ).str()                                     \
          )

#include <boost/bind.hpp>

#define GSPC_LOGGER()                           \
  boost::bind ( &drts::worker::context::log     \
              , _pnetc_context                  \
              , _1                              \
              , _2                              \
              , _3                              \
              , _4                              \
              , _5                              \
              )                                 \

#define GSPC_LOG(_severity, _message)                   \
  GSPC_LLOG (_severity, _message, GSPC_LOGGER())
