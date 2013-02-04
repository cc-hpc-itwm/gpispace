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

  typedef std::map<signature::field_name_t, type> map_type;
  typedef std::pair<std::string, type> key_node_type;

  struct structured_t
  {
  private:
    map_type _map;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_map);
    }

  public:
    type & operator [] (const signature::field_name_t & field_name)
    {
      return _map[field_name];
    }

    const map_type& map() const { return _map; }
  };
}

#endif
