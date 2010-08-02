// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/type/control.hpp>
#include <we/type/bitsetofint.hpp>

#include <boost/variant.hpp>

#include <string>

namespace literal
{
  typedef boost::variant< control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        > type;
}

#endif
