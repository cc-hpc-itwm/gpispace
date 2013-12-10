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
    connection_t ();
    connection_t ( const edge::type&
                 , const transition_id_type&
                 , const place_id_type&
                 );

    edge::type const& type() const
    {
      return _type;
    }
    transition_id_type const& transition_id() const
    {
      return _tid;
    }
    place_id_type const& place_id() const
    {
      return _pid;
    }

    friend std::size_t hash_value (const connection_t&);
    friend bool operator== (const connection_t&, const connection_t&);
    friend bool operator!= (const connection_t&, const connection_t&);

    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_type);
      ar & BOOST_SERIALIZATION_NVP(_tid);
      ar & BOOST_SERIALIZATION_NVP(_pid);
    }

  private:
    edge::type _type;
    transition_id_type _tid;
    place_id_type _pid;
  };
}

#endif
