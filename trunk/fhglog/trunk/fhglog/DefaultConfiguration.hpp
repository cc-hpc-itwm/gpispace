#ifndef FHGLOG_DEFAULT_CONFIGURATION_HPP
#define FHGLOG_DEFAULT_CONFIGURATION_HPP 1

#include <fhglog/fhglog-config.hpp>
#include <fhglog/util.hpp> // environment parser
#include <fhglog/LogLevel.hpp>

#include <iostream>
namespace fhg { namespace log {
    class DefaultConfiguration {
    public:
      static const std::string & STDOUT() { static std::string s("stdout"); return s; }
      static const std::string & STDERR() { static std::string s("stderr"); return s; }
      static const std::string & STDLOG() { static std::string s("stdlog"); return s; }

      DefaultConfiguration();

      void operator() () throw()
      {
#if FHGLOG_DISABLE_LOGGING != 1

#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "I: performing default logging configuration" << std::endl;
#  endif
        default_configuration ();
#endif
    }

    private:
      /*
       * This configuration takes the following environment variables into account (case sensitive):
       *
       *  FHGLOG_level                     log everything with at least this level, defaults to TRACE
       *  FHGLOG_color={auto,on,off}       colorized output
       *  FHGLOG_format=log-format         defaults to the default format in Formatter
       *  FHGLOG_to_console={stdout,stderr,stdlog} log to stdout, stderr, clog
       *  FHGLOG_to_file=path to logfile   log to specified file
       *  FHGLOG_to_server=ip:port         log to the specified server
       *  FHGLOG_threaded={yes,true,1}     make logging appear from a separate thread (unsafe)
       *  FHGLOG_disabled={anything}       disable logging if defined
       *  FHGLOG_synch={anything}          make logging synchronized (very expensive!)
       */
      void parse_environment();
      bool check_config();
      void configure();
      void default_configuration();
      void fallback_configuration();
      void parse_key_value(const std::string &key, const std::string &val);
    private:
      // internal config variables
      fhg::log::LogLevel level_;
      std::string to_console_;
      std::string to_file_;
      std::string to_server_;
      std::string fmt_string_;
      bool threaded_;
      std::string color_;
      bool disabled_;
      bool synchronize_;
    };
  }
}

#endif
