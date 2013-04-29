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

    error ( const std::string & field
          , const std::string & required
          , const std::string & given
          )
      : std::runtime_error ( "type error: " + field 
                           + " requires value of type " + required 
                           + ", given value of type " + given
                           ) {};
  };
}

#endif
