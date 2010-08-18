// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/type/control.hpp>
#include <we/type/bitsetofint.hpp>

#include <boost/variant.hpp>

#include <boost/serialization/vector.hpp>

#include <string>
#include <vector>

namespace literal
{
  typedef std::vector<long> stack_type;

  typedef boost::variant< control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        , stack_type
                        > type;
}

#endif
