#ifndef GPI_SPACE_CONFIG_LOGGING_HPP
#define GPI_SPACE_CONFIG_LOGGING_HPP 1

namespace gpi_space
{
  namespace logging
  {
    static const std::size_t MAX_HOST_LEN = 256;

    struct server_t
    {
      char host[MAX_HOST_LEN];
      uint16_t port;
    };

    struct sink_t
    {
      uint16_t level; // log level
      uint16_t type;  // 0 - server

      union
      {
        server_t server;
      };
    };

    struct config
    {
      uint8_t num_sink;
      sink_t sink[16];
    };
  }
}

#endif
