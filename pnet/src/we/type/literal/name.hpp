// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_NAME_HPP
#define _WE_TYPE_LITERAL_NAME_HPP

#include <string>

namespace literal
{
#define CONST(name) const std::string& name()

  CONST (CONTROL);
  CONST (BOOL);
  CONST (LONG);
  CONST (DOUBLE);
  CONST (CHAR);
  CONST (STRING);
  CONST (BITSET);
  CONST (STACK);
  CONST (MAP);
  CONST (SET);
  CONST (BYTEARRAY);

#undef CONST
}

#endif
