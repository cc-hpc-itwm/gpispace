#pragma once

#include <drts/worker/context.hpp>

#include <fhglog/Logger.hpp>

#include <set>
#include <string>
#include <streambuf>

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
      void module_call_do_cancel() const;

      void log ( fhg::log::Level const& severity
               , std::string const& message
               ) const;

    private:
      std::string _worker_name;
      std::set<std::string> _workers;
      boost::function<void()> _module_call_do_cancel;
      fhg::log::Logger& _logger;
    };

    class redirect_output : public std::streambuf
    {
    public:
      redirect_output ( drts::worker::context const* const context
                      , fhg::log::Level const& severity
                      , std::ostream& os
                      )
        : std::streambuf()
        , _buffer()
        , _context (context)
        , _severity (severity)
        , _os (os)
        , _streambuf (_os.rdbuf (this))
      {}
      ~redirect_output()
      {
        if (_streambuf)
        {
          _os.rdbuf (_streambuf);
        }

        if (!_buffer.empty())
        {
          _context->_->log (_severity, _buffer);
        }
      }

      int_type overflow (int_type c)
      {
        if ('\n' == traits_type::to_char_type (c))
        {
          _os.rdbuf (_streambuf);
          _streambuf = nullptr;
          _context->_->log (_severity, _buffer);
          _streambuf = _os.rdbuf (this);

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
      drts::worker::context const* const _context;
      fhg::log::Level const _severity;
      std::ostream& _os;
      std::streambuf* _streambuf;
    };
  }
}
