#pragma once

#include <iosfwd>
#include <list>
#include <set>
#include <string>

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
      NAME (SET);
      NAME (MAP);
      NAME (STRUCT);

#undef NAME

      std::list<std::string> type_names();
    }
  }
}
