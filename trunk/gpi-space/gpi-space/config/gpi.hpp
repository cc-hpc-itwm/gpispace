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
        : network_type (0)
        , mtu (2048)
        , port (10820)
        , processes (0)
      {}

      template <typename Mapping>
      void load (Mapping const & m)
      {
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
      }

      uint64_t       memory_size;
      int            network_type;
      unsigned int   mtu;
      unsigned short port;
      unsigned int   processes;
    };
  }
}

#endif
