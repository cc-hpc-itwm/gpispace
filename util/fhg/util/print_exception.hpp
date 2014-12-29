// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_PRINT_EXCEPTION_HPP
#define FHG_UTIL_PRINT_EXCEPTION_HPP

#include <exception>
#include <ostream>
#include <string>

namespace fhg
{
  namespace util
  {
    void print_exception ( std::ostream&
                         , std::string const& prefix
                         , std::exception const&
                         , int indentation = 0
                         );
    void print_exception ( std::ostream&
                         , std::string const& prefix
                         , std::exception_ptr const&
                         , int indentation = 0
                         );
    void print_current_exception ( std::ostream&
                                 , std::string const& prefix
                                 , int indentation = 0
                                 );
  }
}

#endif
