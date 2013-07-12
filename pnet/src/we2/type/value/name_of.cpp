// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/value/name_of.hpp>
#include <we2/type/value/name.hpp>

#include <we2/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
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
      NAME_OF (SET, std::set<value_type>);
      NAME_OF (MAP, std::map<value_type,value_type>);
      NAME_OF (STRUCT, structured_type);

#undef NAME_OF
    }
  }
}
