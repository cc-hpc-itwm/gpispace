// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_ERROR_HPP
#define _TYPE_ERROR_HPP

#include <stdexcept>

namespace type
{
  class error : public std::runtime_error
  {
  public:
    error (const std::string & what)
      : std::runtime_error ("type error: " + what) {};

    error ( const signature::field_name_t & field
          , const literal::type_name_t & required
          , const literal::type_name_t & given
          )
      : std::runtime_error ( "type error: " + field 
                           + " requires value of type " + required 
                           + ", given value of type " + given
                           ) {};
  };
}

#endif
