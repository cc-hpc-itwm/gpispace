// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_HPP
#define _WE_TYPE_VALUE_HPP

#include <we/type/literal.hpp>
#include <we/type/signature/types.hpp>

#include <string>
#include <map>
#include <list>

#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

namespace value
{
  typedef std::list<signature::field_name_t> path_type;

  struct structured_t;

  typedef boost::variant< literal::type
                        , boost::recursive_wrapper<structured_t>
                        > type;

  struct structured_t
  {
  public:
    // NOTE! sorted container neccessary for operator ==
    typedef std::map<signature::field_name_t, type> map_t;
    typedef map_t::const_iterator const_iterator;
    typedef map_t::const_iterator iterator;

  private:
    map_t map;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(map);
    }

  public:
    structured_t () : map () {}

    type & operator [] (const signature::field_name_t & field_name)
    {
      return map[field_name];
    }

    std::size_t size() const { return map.size(); }

    const_iterator begin (void) const { return map.begin(); }
    const_iterator end (void) const { return map.end(); }
    const_iterator find (const signature::field_name_t & field_name) const
    {
      return map.find (field_name);
    }

    bool has_field (const signature::field_name_t & field_name) const
    {
      return map.find (field_name) != map.end();
    }
  };
}

#endif
