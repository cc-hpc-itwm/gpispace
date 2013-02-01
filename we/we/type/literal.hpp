// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <boost/variant.hpp>

#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/string.hpp>

#include <string>
#include <vector>

#include <map>
#include <set>

namespace literal
{
  typedef std::deque<long> stack_type;
  typedef std::map<long, long> map_type;
  typedef std::set<long> set_type;

  typedef boost::variant< we::type::literal::control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        , stack_type
                        , map_type
                        , set_type
                        , bytearray::type
                        > type;
}

#endif
