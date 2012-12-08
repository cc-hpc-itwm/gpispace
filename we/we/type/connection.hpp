// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONNECTION_HPP
#define _WE_TYPE_CONNECTION_HPP

#include <we/type/id.hpp>

#include <boost/serialization/nvp.hpp>

#include <string>

namespace petri_net
{
  namespace edge
  {
    //! \todo eliminate this, instead use subclasses of connection
    enum type {PT,PT_READ,TP};

    bool is_pt_read (const type&);
    bool is_PT (const type&);

    std::string enum_to_string (const type&);
  }

  struct connection_t
  {
  public:
    edge::type type;
    tid_t tid;
    pid_t pid;

    connection_t ();
    connection_t (const edge::type&, const tid_t&, const pid_t&);

    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(type);
      ar & BOOST_SERIALIZATION_NVP(tid);
      ar & BOOST_SERIALIZATION_NVP(pid);
    }
  };
}

#endif
