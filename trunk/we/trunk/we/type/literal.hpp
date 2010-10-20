// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/type/control.hpp>
#include <we/type/bitsetofint.hpp>

#include <boost/variant.hpp>

#include <boost/serialization/deque.hpp>

#include <boost/unordered_map.hpp>

#include <string>
#include <vector>

namespace literal
{
  typedef std::deque<long> stack_type;
  typedef boost::unordered_map<long, long> map_type;

  typedef boost::variant< control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        , stack_type
                        , map_type
                        > type;
}

#endif
