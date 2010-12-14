#ifndef GPI_SPACE_CONFIG_GPI_HPP
#define GPI_SPACE_CONFIG_GPI_HPP 1

namespace gpi_space
{
  namespace gpi
  {
    struct config
    {
      config ()
        : network_type (0)
        , mtu (2048)
        , port (10820)
        , processes (0)
      {}

      uint64_t       memory_size;
      int            network_type;
      unsigned int   mtu;
      unsigned short port;
      unsigned int   processes;
    };
  }
}

#endif
