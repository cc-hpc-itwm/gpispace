#ifndef GPI_SPACE_CONFIG_LOGGING_HPP
#define GPI_SPACE_CONFIG_LOGGING_HPP 1

/*
  [logger ""]
  level = MIN
  appender = logserver logfile

  [appender "logserver"]
  type = server
  level = DEF

  host = localhost
  port = 2438

  [appender "logfile"]
  type = file
  level = MIN

  path = /tmp/foo/bar.log
  mode = 0600
*/

namespace gpi_space
{
  namespace logging
  {
    enum type_code_t
      {
        TC_INVALID = 0   // i.e. disabled
      , TC_SERVER = 1
      , TC_FILE = 2
      , TC_CONSOLE = 3
      // more to come
      };

    struct server_t
    {
      char host[gpi_space::MAX_HOST_LEN];
      uint16_t port;
    };

    struct file_t
    {
      char path[gpi_space::MAX_PATH_LEN];
      uint16_t mode;
    };

    struct console_t
    {
      enum console_type
        {
          CONSOLE_OUT
        , CONSOLE_ERR
        , CONSOLE_LOG
        };
      console_type which;
    };

    struct sink_t
    {
      sink_t ()
        : type (TC_INVALID)
      {}

      uint16_t level; // log level
      uint16_t type;  // see type_code_t

      union
      {
        server_t server;
      };
    };

    struct config
    {
      enum { max_sinks = 3 };
      sink_t sink[max_sinks];
    };
  }
}

#endif
