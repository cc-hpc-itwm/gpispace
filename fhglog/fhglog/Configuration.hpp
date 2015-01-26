#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

namespace boost
{
  namespace asio
  {
    class io_service;
  }
}

namespace fhg
{
  namespace log
  {
    /*
     * This configuration takes the following environment variables into account (case sensitive):
     *
     *  FHGLOG_level                     log everything with at least this level, defaults to TRACE
     *  FHGLOG_color={on,off}            colorized output
     *  FHGLOG_format=log-format         defaults to the default format in Formatter
     *  FHGLOG_to_console={stdout,stderr,stdlog} log to stdout, stderr, clog
     *  FHGLOG_to_file=path to logfile   log to specified file
     *  FHGLOG_to_server=ip:port         log to the specified server
     *  FHGLOG_disabled={anything}       disable logging if defined
     */

    void configure (boost::asio::io_service& remote_log_io_service);
  }
}

#endif
