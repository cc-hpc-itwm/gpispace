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
        boost::unordered_map<std::string, std::string> _trans;
        boost::unordered_map<std::string, std::string> _inc;

      public:
        info ()
          : _trans()
          , _inc()
        {
          _trans[literal::CONTROL()]   = "we::type::literal::control";
          _trans[literal::BOOL()]      = "bool";
          _trans[literal::LONG()]      = "long";
          _trans[literal::DOUBLE()]    = "double";
          _trans[literal::CHAR()]      = "char";
          _trans[literal::STRING()]    = "std::string";
          _trans[literal::BITSET()]    = "bitsetofint::type";
          _trans[literal::STACK()]     = "std::deque<long>";
          _trans[literal::MAP()]       = "std::map<long, long>";
          _trans[literal::SET()]       = "std::set<long>";
          _trans[literal::BYTEARRAY()] = "bytearray::type";

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

        const std::string& translate (const std::string& t) const
        {
          return _trans.at (t);
        }

        const std::string& include (const std::string& t) const
        {
          return _inc.at (t);
        }
      };
    }

    const std::string& translate (const std::string& t)
    {
      static info i;

      return i.translate (t);
    }
    const std::string& include (const std::string& t)
    {
      static info i;

      return i.include (t);
    }

    namespace
    {
      struct names_reserved
      {
      public:
        names_reserved ()
          : _s()
        {
          _s.insert ("control");
          _s.insert ("bool");
          _s.insert ("long");
          _s.insert ("double");
          _s.insert ("char");
        }

        bool reserved (const std::string& x) const
        {
          return _s.find (x) != _s.end();
        }

      private:
        boost::unordered_set<std::string> _s;
      };
    }

    bool reserved (const std::string& x)
    {
      static names_reserved n;

      return n.reserved (x);
    }
  }
}
