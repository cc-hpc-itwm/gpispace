// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/signature/name.hpp>

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
      NAME (UINT, "uint");
      NAME (ULONG, "ulong");
      NAME (FLOAT, "float");
      NAME (DOUBLE, "double");
      NAME (CHAR, "char");
      NAME (STRING, "string");
      NAME (BITSET,"bitset");
      NAME (BYTEARRAY, "bytearray");
      NAME (LIST, "list");
      NAME (VECTOR, "vector");
      NAME (SET, "set");
      NAME (MAP, "map");
      NAME (STRUCT, "struct");

#undef NAME

#define NAME_OF(_name, _type...)                                  \
      template<> const std::string& name_of<_type> (const _type&) \
      {                                                           \
        return _name();                                           \
      }

      NAME_OF (CONTROL, we::type::literal::control);
      NAME_OF (BOOL, bool);
      NAME_OF (INT, int);
      NAME_OF (LONG, long);
      NAME_OF (UINT, unsigned int);
      NAME_OF (ULONG, unsigned long);
      NAME_OF (FLOAT, float);
      NAME_OF (DOUBLE, double);
      NAME_OF (CHAR, char);
      NAME_OF (STRING, std::string);
      NAME_OF (BITSET, bitsetofint::type);
      NAME_OF (BYTEARRAY, bytearray::type);
      NAME_OF (LIST, std::list<value_type>);
      NAME_OF (VECTOR, std::vector<value_type>);
      NAME_OF (SET, std::set<value_type>);
      NAME_OF (MAP, std::map<value_type,value_type>);
      NAME_OF (STRUCT, structured_type);

#undef NAME_OF
    }
  }
}
