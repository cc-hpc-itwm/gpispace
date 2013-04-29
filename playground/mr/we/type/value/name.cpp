// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/name.hpp>

#include <iostream>

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

      namespace
      {
        static std::set<std::string> init_typename()
        {
          std::set<std::string> tn;

          tn.insert (CONTROL());
          tn.insert (BOOL());
          tn.insert (INT());
          tn.insert (LONG());
          tn.insert (UINT());
          tn.insert (ULONG());
          tn.insert (FLOAT());
          tn.insert (DOUBLE());
          tn.insert (CHAR());
          tn.insert (STRING());
          tn.insert (LIST());
          tn.insert (SET());
          tn.insert (MAP());

          return tn;
        }

        const std::set<std::string>& typenames()
        {
          static std::set<std::string> tn (init_typename());

          return tn;
        }
      }

      typename_of::typename_of (const std::string& name)
        : _name (name)
      {}
      std::ostream& typename_of::operator() (std::ostream& os) const
      {
        os << _name;
        if (typenames().find (_name) == typenames().end())
        {
          os << std::string ("::type");
        }
        return os;
      }

      std::ostream& operator<< (std::ostream& os, const typename_of& tno)
      {
        return tno (os);
      }
    }
  }
}
