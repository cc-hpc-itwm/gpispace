#pragma once

#include <fhglog/Logger.hpp>

#include <boost/asio/io_service.hpp>

namespace fhg
{
  namespace log
  {
    /*
     * This configuration takes the following environment variables into account (case sensitive):
     *
     *  FHGLOG_level                     log everything with at least this level, defaults to TRACE
     *  FHGLOG_format=log-format         defaults to the default format in Formatter
     *  FHGLOG_to_console={stdout,stderr,stdlog} log to stdout, stderr, clog
     *  FHGLOG_to_file=path to logfile   log to specified file
     *  FHGLOG_to_server=ip:port         log to the specified server
     *  FHGLOG_disabled={anything}       disable logging if defined
     */

    void configure (boost::asio::io_service& remote_log_io_service, Logger&);

    // - level = trace
    // - to_console = stderr
    // - remainder unset
    void configure_to_stderr (Logger&);
  }
}
