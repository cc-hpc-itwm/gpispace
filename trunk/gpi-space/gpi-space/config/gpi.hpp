#ifndef GPI_SPACE_CONFIG_GPI_HPP
#define GPI_SPACE_CONFIG_GPI_HPP 1

#include <boost/lexical_cast.hpp>

namespace gpi_space
{
  namespace gpi
  {
    struct config
    {
      config ()
        : memory_size (0)
        , network_type (0)
        , mtu (2048)
        , port (10820)
        , processes (0)
        , timeout_in_sec (120)
        , mode (0700)
      {}

      template <typename Mapping>
      void load (Mapping const & m)
      {
        timeout_in_sec = boost::lexical_cast<unsigned int>(m.get("gpi.timeout", "120"));

        {
          std::size_t multiplier (1);
          std::string mem_size
            (m.get ("gpi.memory_size", "2G"));
          if (mem_size.empty())
          {
            throw std::runtime_error ("invalid value for gpi.memory_size!");
          }
          else
          {
            switch (mem_size[mem_size.size()-1])
            {
            case 'G':
            case 'g':
              multiplier = (1024 * 1024 * 1024);
              mem_size.erase (mem_size.size()-1);
              break;
            case 'M':
            case 'm':
              multiplier = (1024 * 1024);
              mem_size.erase (mem_size.size()-1);
              break;
            case 'K':
            case 'k':
              multiplier = (1024);
              mem_size.erase (mem_size.size()-1);
              break;
            case 'B':
            case 'b':
              mem_size.erase (mem_size.size()-1);
              break;
            default:
              break;
            }
          }
          try
          {
            memory_size = boost::lexical_cast<uint64_t>(mem_size) * multiplier;
          }
          catch (std::exception const & ex)
          {
            throw std::runtime_error ("invalid value for gpi.memory_size: " + mem_size + ": " + ex.what());
          }
        }

        {
          std::string c_port (m.get("gpi.port", boost::lexical_cast<std::string>(port)));
          if (c_port != "default")
          {
            port = boost::lexical_cast<unsigned int>(c_port);
          }
        }

        {
          std::string c_procs (m.get("gpi.processes", boost::lexical_cast<std::string>(processes)));
          if (c_procs != "default")
          {
            processes = boost::lexical_cast<unsigned int>(c_procs);
          }
        }

        mode = boost::lexical_cast<mode_t>(m.get("gpi.socket_mode", "0700"));
        std::string default_path ("/var/tmp/gpi-space");
        snprintf( socket_path
                , gpi_space::MAX_PATH_LEN
                , "%s"
                , m.get("gpi.socket_path", default_path.c_str()).c_str()
                );

        std::string default_name ("control-");
        default_name += boost::lexical_cast<std::string>(getpid());
        snprintf( socket_name
                , gpi_space::MAX_PATH_LEN
                , "%s"
                , m.get("gpi.socket_name", default_name.c_str()).c_str()
                );
      }

      uint64_t       memory_size;
      int            network_type;
      unsigned int   mtu;
      unsigned short port;
      unsigned int   processes;
      unsigned int   timeout_in_sec;
      char socket_path[gpi_space::MAX_PATH_LEN];
      char socket_name[gpi_space::MAX_PATH_LEN];
      mode_t mode;
    };
  }
}

#endif
