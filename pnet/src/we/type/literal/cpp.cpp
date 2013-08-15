// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/cpp.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace literal
{
  namespace cpp
  {
    namespace
    {
      struct info
      {
      private:
        boost::unordered_map<std::string, std::string> _inc;

      public:
        info ()
          : _inc()
        {
          _inc[literal::CONTROL()]   = "we/type/literal/control.hpp";
          _inc[literal::BOOL()]      = "";
          _inc[literal::LONG()]      = "";
          _inc[literal::DOUBLE()]    = "";
          _inc[literal::CHAR()]      = "";
          _inc[literal::STRING()]    = "string";
          _inc[literal::BITSET()]    = "we/type/bitsetofint.hpp";
          _inc[literal::STACK()]     = "deque";
          _inc[literal::MAP()]       = "map";
          _inc[literal::SET()]       = "set";
          _inc[literal::BYTEARRAY()] = "we/type/bytearray.hpp";
        }

        const std::string& include (const std::string& t) const
        {
          return _inc.at (t);
        }
      };
    }

    const std::string& include (const std::string& t)
    {
      static info i;

      return i.include (t);
    }
  }
}
