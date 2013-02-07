// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/name.hpp>

namespace literal
{
#define CONST(name,value)                 \
  const std::string& name()               \
  {                                       \
    static const std::string x (value);   \
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
