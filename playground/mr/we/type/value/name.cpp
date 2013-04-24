// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/name.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
#define NAME(_name,_value)                      \
      const std::string& _name()                \
      {                                         \
        static const std::string x (_value);    \
                                                \
        return x;                               \
      }

      NAME (CONTROL, "control");
      NAME (BOOL, "bool");
      NAME (INT, "int");
      NAME (LONG, "long");
      NAME (UINT, "unsigned int");
      NAME (ULONG, "unsigned long");
      NAME (FLOAT, "float");
      NAME (DOUBLE, "double");
      NAME (CHAR, "char");
      NAME (STRING, "string");
      NAME (BITSET,"bitset");
      NAME (BYTEARRAY, "bytearray");
      NAME (LIST, "list");
      NAME (SET, "set");
      NAME (MAP, "map");
      NAME (STRUCT, "struct");

#undef NAME
    }
  }
}
