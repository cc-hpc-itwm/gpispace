#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

namespace fhg
{
  namespace log
  {
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

    void configure();
    void configure (int ac, char *av[]);
  }
}

#endif
