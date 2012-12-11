// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_ID_HPP
#define _TYPE_ID_HPP

#include <boost/cstdint.hpp>

#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/functional/hash.hpp>

namespace petri_net
{
  template<typename POD_TYPE, class client_type>
  struct id_base_type
  {
    typedef POD_TYPE pod_type;

    id_base_type ()
      : _value (0)
    {}

    explicit
    id_base_type (const pod_type& value)
      : _value (value)
    {}

    bool operator== (const id_base_type& other) const
    {
      return _value == other._value;
    }
    bool operator!= (const id_base_type& other) const
    {
      return _value != other._value;
    }
    bool operator< (const id_base_type& other) const
    {
      return _value < other._value;
    }
    bool operator> (const id_base_type& other) const
    {
      return _value > other._value;
    }

    const client_type& operator++ ()
    {
      ++_value;

      return *this;
    }
    client_type operator++ (int)
    {
      client_type old (_value);

      ++_value;

      return old;
    }

    const pod_type& value () const
    {
      return _value;
    }

    template<typename P, typename C>
    friend std::ostream& operator<< (std::ostream&, const id_base_type<P, C>&);
    template<typename P, typename C>
    friend std::istream& operator>> (std::istream&, id_base_type<P, C>&);
    template<typename P, typename C>
    friend std::size_t hash_value (const id_base_type<P, C>&);
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_value);
    }

  private:
    pod_type _value;
  };

  template<typename pod_type, typename client_type>
  std::size_t hash_value (const id_base_type<pod_type, client_type>& id)
  {
    return boost::hash<pod_type>() (id._value);
  }

  template<typename pod_type, typename client_type>
  std::ostream& operator<< ( std::ostream& s
                           , const id_base_type<pod_type, client_type>& id
                           )
  {
    return s << id._value;
  }

  template<typename pod_type, typename client_type>
  std::istream& operator>> ( std::istream& i
                           , id_base_type<pod_type, client_type>& id
                           )
  {
    return i >> id._value;
  }

#define INHERIT_ID_TYPE(_name,_type)                                    \
  struct _name : public ::petri_net::id_base_type<_type, _name>         \
  {                                                                     \
    _name () : id_base_type<_type,_name>() {}                           \
    _name (const _type& value) : id_base_type<_type,_name> (value) {}   \
  }

  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 > 194 years.
  // It follows that an uint64_t is enough for now.

  INHERIT_ID_TYPE (edge_id_type, boost::uint64_t);
  INHERIT_ID_TYPE (place_id_type, boost::uint64_t);
  INHERIT_ID_TYPE (port_id_type, boost::uint64_t);
  INHERIT_ID_TYPE (transition_id_type, boost::uint64_t);
  INHERIT_ID_TYPE (activity_id_type, boost::uint64_t);

  INHERIT_ID_TYPE (priority_type, boost::int16_t);

#undef INHERIT_ID_TYPE

  //! \todo replace with boost::optional!?
#define INVALID(_prefix)                       \
  const _prefix ## _type& _prefix ## _invalid()

  INVALID (edge_id);
  INVALID (place_id);
  INVALID (activity_id);

  INVALID (priority);

#undef INVALID
}

#endif
