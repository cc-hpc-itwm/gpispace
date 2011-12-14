#ifndef GPI_SPACE_CONFIG_LOGGING_HPP
#define GPI_SPACE_CONFIG_LOGGING_HPP 1

#include <gpi-space/config/config-data.hpp>
#include <fhglog/minimal.hpp>
#include <fhg/util/getenv.hpp>

#include <stdio.h> // snprintf
#include <boost/lexical_cast.hpp>

/*
  ;; future

  [logger ""]
  level = MIN
  sink = net

  [sink "net"]
  type = udp
  host = localhost
  port = 2438

  [sink "console"]
  level = MIN
  type = stream
  stream = stderr

  [sink "file"]
  type = file
  path = /tmp/foo/bar.log
  mode = 0600

  ;; now
  [logging]
  ; 0 - MIN (more output)
  ; ...
  ; 5 - MAX (less output)
  level = 0
  server = ip[:port]
*/

namespace gpi_space
{
  namespace logging
  {
    enum type_code_t
      {
        TC_INVALID = 0   // i.e. disabled
      , TC_REMOTE = 1
      , TC_FILE = 2
      , TC_STREAM = 3
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

    struct stream_t
    {
      enum stream_type
        {
          CONSOLE_OUT
        , CONSOLE_ERR
        , CONSOLE_LOG
        };
      stream_type which;
    };

    struct sink_t
    {
      sink_t ()
        : type (TC_INVALID)
      {}

      char    level[16]; // log level
      uint8_t type;  // see type_code_t

      union
      {
        server_t server;
        file_t file;
        stream_t stream;
      };
    };

    struct config
    {
      enum { max_sinks = 3 };
      sink_t sink[max_sinks];

      template <typename Mapping>
      void load (Mapping const &cfg)
      {
        const std::string fallback_server
          (fhg::util::getenv("FHGLOG_to_server", "localhost"));
        const std::string fallback_level
          (fhg::util::getenv("FHGLOG_level", "INFO"));
        std::string server(cfg.get ("log.server.url", fallback_server));
        std::string level (cfg.get ("log.server.level", fallback_level));

        sink[0].type = TC_REMOTE;
        snprintf (sink[0].server.host, gpi_space::MAX_HOST_LEN, "%s", server.c_str());
        snprintf (sink[0].level      , 16                     , "%s", level.c_str());
      }
    };

    // WORK HERE: this is just a quick hack to get things working...
    void configure (config const & c)
    {
      for (std::size_t s (0); s < config::max_sinks; ++s)
      {
        switch (c.sink[s].type)
        {
        case TC_REMOTE:
          {
            setenv ("FHGLOG_to_server", c.sink[s].server.host, true);
            setenv ("FHGLOG_level", c.sink[s].level, true);
          }
          break;
        default:
          break;
        }
      }

      FHGLOG_SETUP();
    }
  }
}

#endif
