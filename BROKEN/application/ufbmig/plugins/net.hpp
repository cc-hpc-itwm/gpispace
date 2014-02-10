#ifndef SDPA_PLUGIN_NET_HPP
#define SDPA_PLUGIN_NET_HPP 1

#include <string>

namespace net
{
  class Peer
  {
  public:
    virtual ~Peer () {}

    virtual std::string const & name () const = 0;

    virtual int send ( std::string const & to
                     , std::string const & data
                     ) = 0;
    virtual int recv ( std::string & from
                     , std::string & data
                     ) = 0;
  };
}

#endif
