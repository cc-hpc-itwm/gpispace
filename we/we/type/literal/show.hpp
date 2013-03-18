// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_SHOW_HPP
#define _WE_TYPE_LITERAL_SHOW_HPP

#include <we/type/literal.hpp>

#include <iosfwd>
#include <string>

namespace literal
{
  //  std::ostream& operator<< (std::ostream&, const type&);
  std::string show (const type&);
}

#endif
