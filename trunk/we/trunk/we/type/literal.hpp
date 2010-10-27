// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/type/control.hpp>
#include <we/type/bitsetofint.hpp>

#include <boost/variant.hpp>

#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>

#include <string>
#include <vector>

#include <map>
#include <set>

namespace literal
{
  typedef std::deque<long> stack_type;
  typedef std::map<long, long> map_type;
  typedef std::set<long> set_type;

  typedef boost::variant< control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        , stack_type
                        , map_type
                        , set_type
                        > type;
}

#endif
