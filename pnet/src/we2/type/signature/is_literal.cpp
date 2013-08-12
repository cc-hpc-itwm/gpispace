// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/is_literal.hpp>
#include <we2/type/value/name.hpp>

#include <set>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        std::set<std::string> init_literal_names()
        {
          std::set<std::string> ln;

          ln.insert (value::CONTROL());
          ln.insert (value::BOOL());
          ln.insert (value::INT());
          ln.insert (value::LONG());
          ln.insert (value::UINT());
          ln.insert (value::ULONG());
          ln.insert (value::FLOAT());
          ln.insert (value::DOUBLE());
          ln.insert (value::CHAR());
          ln.insert (value::STRING());
          ln.insert (value::BITSET());
          ln.insert (value::BYTEARRAY());
          ln.insert (value::LIST());
          ln.insert (value::SET());
          ln.insert (value::MAP());

          ln.insert ("stack");

          return ln;
        }
      }

      bool is_literal (const std::string& tname)
      {
        static std::set<std::string> ln (init_literal_names());

        return ln.find (tname) != ln.end();
      }
    }
  }
}
