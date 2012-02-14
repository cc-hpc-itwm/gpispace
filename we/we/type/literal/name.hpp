// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_NAME_HPP
#define _WE_TYPE_LITERAL_NAME_HPP

#include <string>

namespace literal
{
  typedef std::string type_name_t;

#define CONST(name,string)                \
  static inline const type_name_t name () \
  {                                       \
    static const type_name_t x (string);  \
                                          \
    return x;                             \
  }                                       \

  CONST (CONTROL, "control");
  CONST (BOOL, "bool");
  CONST (LONG, "long");
  CONST (DOUBLE, "double");
  CONST (CHAR, "char");
  CONST (STRING, "string");
  CONST (BITSET,"bitset");
  CONST (STACK, "stack");
  CONST (MAP, "map");
  CONST (SET, "set");
  CONST (BYTEARRAY, "bytearray");

#undef CONST
}

#endif
