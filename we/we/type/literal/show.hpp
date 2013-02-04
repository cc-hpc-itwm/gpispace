// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_SHOW_HPP
#define _WE_TYPE_LITERAL_SHOW_HPP

#include <we/type/literal.hpp>

#include <iosfwd>
#include <string>

namespace literal
{
  std::ostream& operator<< (std::ostream&, const type&);
}

namespace std
{
  std::ostream& operator<< (std::ostream&, const literal::stack_type&);
  std::ostream& operator<< (std::ostream&, const literal::map_type&);
  std::ostream& operator<< (std::ostream&, const literal::set_type&);
}

#endif
