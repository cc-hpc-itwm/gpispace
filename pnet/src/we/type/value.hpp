// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_HPP
#define _WE_TYPE_VALUE_HPP

#include <we/type/literal.hpp>

#include <string>
#include <map>
#include <list>

#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

namespace value
{
  typedef std::list<std::string> path_type;

  struct structured_t;

  typedef boost::variant< literal::type
                        , boost::recursive_wrapper<structured_t>
                        > type;

  typedef std::map<std::string, type> map_type;
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
    type & operator [] (const std::string & field_name)
    {
      return _map[field_name];
    }

    const map_type& map() const { return _map; }
  };

  inline bool operator== (const structured_t& x, const structured_t& y)
  {
    map_type::const_iterator pos_x (x.map().begin());
    map_type::const_iterator pos_y (y.map().begin());
    const map_type::const_iterator end_x (x.map().end());

    bool all_eq (x.map().size() == y.map().size());

    while (all_eq && pos_x != end_x)
    {
      all_eq = pos_x->first == pos_y->first && pos_x->second == pos_y->second;

      ++pos_x;
      ++pos_y;
    }

    return all_eq;
  }
}

#endif
