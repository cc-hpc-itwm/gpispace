// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_CONNECTION_HPP
#define _TYPE_CONNECTION_HPP

#include <boost/serialization/nvp.hpp>

namespace petri_net
{
  template<typename TYPE, typename TID, typename PID>
  struct connection
  {
  public:
    TYPE type;
    TID tid;
    PID pid;

    connection () : type(), tid(), pid() {}

    connection (const TYPE & _type, const TID & _tid, const PID & _pid)
      : type (_type)
      , tid (_tid)
      , pid (_pid)
    {}

    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & type;
      ar & tid;
      ar & pid;
    }
  };
}

#endif
