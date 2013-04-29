// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SIGNATURE_NAME_HPP
#define PNET_SRC_WE_TYPE_VALUE_SIGNATURE_NAME_HPP

#include <string>
#include <set>
#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
#define NAME(_name) const std::string& _name()

      NAME (CONTROL);
      NAME (BOOL);
      NAME (INT);
      NAME (LONG);
      NAME (UINT);
      NAME (ULONG);
      NAME (FLOAT);
      NAME (DOUBLE);
      NAME (CHAR);
      NAME (STRING);
      NAME (BITSET);
      NAME (BYTEARRAY);
      NAME (LIST);
      NAME (VECTOR);
      NAME (SET);
      NAME (MAP);
      NAME (STRUCT);

#undef NAME

      class typename_of
      {
      public:
        typename_of (const std::string&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const std::string& _name;
      };
      std::ostream& operator<< (std::ostream&, const typename_of&);
    }
  }
}

#endif
