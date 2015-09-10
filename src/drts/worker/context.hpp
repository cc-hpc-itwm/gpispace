#pragma once

#include <drts/worker/context_fwd.hpp>

#include <fhglog/level.hpp>

#include <set>
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

      std::set<std::string> const& workers() const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (boost::function<void()>);
      void module_call_do_cancel() const;

      void log ( fhg::log::Level const& severity
               , std::string const& message
               ) const;
    };

    typedef boost::function<void ( fhg::log::Level const& severity
                                 , std::string const& message
                                 )> logger_type;
  }
}

#include <sstream>

#define GSPC_LLOG(_severity, _message, _logger)         \
  _logger ( fhg::log::_severity                         \
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
              )                                 \

#define GSPC_LOG(_severity, _message)                   \
  GSPC_LLOG (_severity, _message, GSPC_LOGGER())

#include <ostream>
#include <streambuf>

namespace drts
{
  namespace worker
  {
    class line_by_line_streambuf : public std::streambuf
    {
    public:
      line_by_line_streambuf ( context const* const context
                             , fhg::log::Level const& severity
                             )
        : std::streambuf()
        , _buffer()
        , _context (context)
        , _severity (severity)
      {}
      ~line_by_line_streambuf()
      {
        if (!_buffer.empty())
        {
          _context->log (_severity, _buffer);
        }
      }

      int_type overflow (int_type c)
      {
        if ('\n' == traits_type::to_char_type (c))
        {
          _context->log (_severity, _buffer);

          _buffer.clear();
        }
        else
        {
          _buffer += traits_type::to_char_type (c);
        }

        return c;
      }

    private:
      std::string _buffer;
      context const* const _context;
      fhg::log::Level const _severity;
    };

    class ostream : private line_by_line_streambuf
                  , public std::ostream
    {
    public:
      ostream ( context const* const context
              , fhg::log::Level const& severity
              )
        : line_by_line_streambuf (context, severity)
        , std::ostream (this)
      {}
    };
  }
}

#define DECLARE_GSPC_OSTREAM(_severity, _name)          \
  drts::worker::ostream _name ( _pnetc_context          \
                              , fhg::log::_severity     \
                              )
