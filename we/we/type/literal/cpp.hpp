// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_CPP_HPP
#define _WE_TYPE_LITERAL_CPP_HPP

#include <we/type/literal/name.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace literal
{
  namespace cpp
  {
    struct info
    {
    public:
      typedef boost::unordered_map<type_name_t, std::string> map_type;

    private:
      map_type trans;
      map_type inc;

    public:
      info (void) : trans (), inc ()
      {
        trans[literal::CONTROL()]   = "we::type::literal::control";
        trans[literal::BOOL()]      = "bool";
        trans[literal::LONG()]      = "long";
        trans[literal::DOUBLE()]    = "double";
        trans[literal::CHAR()]      = "char";
        trans[literal::STRING()]    = "std::string";
        trans[literal::BITSET()]    = "bitsetofint::type";
        trans[literal::STACK()]     = "std::deque<long>";
        trans[literal::MAP()]       = "std::map<long, long>";
        trans[literal::SET()]       = "std::set<long>";
        trans[literal::BYTEARRAY()] = "bytearray::type";

        inc[literal::CONTROL()]   = "we/type/literal/control.hpp";
        inc[literal::BOOL()]      = "";
        inc[literal::LONG()]      = "";
        inc[literal::DOUBLE()]    = "";
        inc[literal::CHAR()]      = "";
        inc[literal::STRING()]    = "string";
        inc[literal::BITSET()]    = "we/type/bitsetofint.hpp";
        inc[literal::STACK()]     = "deque";
        inc[literal::MAP()]       = "map";
        inc[literal::SET()]       = "set";
        inc[literal::BYTEARRAY()] = "we/type/bytearray.hpp";
      }

      const map_type::mapped_type & translate (const type_name_t & t) const
      {
        return trans.at (t);
      }

      const map_type::mapped_type & include (const type_name_t & t) const
      {
        return inc.at (t);
      }

      bool known (const type_name_t & t) const
      {
        return trans.find (t) != trans.end();
      }
    };

    inline const info::map_type::mapped_type & translate (const type_name_t & t)
    {
      static info i;

      return i.translate (t);
    }

    inline const info::map_type::mapped_type & include (const type_name_t & t)
    {
      static info i;

      return i.include (t);
    }

    inline bool known (const type_name_t & t)
    {
      static info n;

      return n.known (t);
    }

    struct names_reserved
    {
    private:
      typedef boost::unordered_set<std::string> set_type;

      set_type s;

    public:
      names_reserved (void) : s ()
      {
        s.insert ("control");
        s.insert ("bool");
        s.insert ("long");
        s.insert ("double");
        s.insert ("char");
      }

      bool reserved (const set_type::value_type & x) const
      {
        return (s.find (x) != s.end());
      }
    };

    inline bool reserved (const std::string & x)
    {
      static names_reserved n;

      return n.reserved (x);
    }
  }
}

#endif
