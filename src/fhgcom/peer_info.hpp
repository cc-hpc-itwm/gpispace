#ifndef FHG_COM_PEER_INFO_HPP
#define FHG_COM_PEER_INFO_HPP 1

#include <string>

namespace fhg
{
  namespace com
  {
    struct hard_string
    {
      explicit hard_string (std::string const& s)
        : _s (s)
      {}
      operator std::string () const
      {
        return _s;
      }
    private:
      std::string const _s;
    };

    struct host_t : hard_string
    {
      explicit host_t (std::string const& s)
        : hard_string (s)
      {}
    };
    struct port_t : hard_string
    {
      explicit port_t (std::string const& s)
        : hard_string (s)
      {}
    };
  }
}

#endif
