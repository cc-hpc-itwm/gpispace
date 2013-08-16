// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/complete.hpp>

#include <we/type/value/name.hpp>

#include <boost/unordered_map.hpp>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        typedef boost::unordered_map<std::string,std::string> map_type;

        map_type init_typenames_complete()
        {
          map_type tn;

          tn[value::CONTROL()] = "we::type::literal::control";
          tn[value::BOOL()] = "bool";
          tn[value::INT()] = "int";
          tn[value::LONG()] = "long";
          tn[value::UINT()] = "unsigned int";
          tn[value::ULONG()] = "unsigned long";
          tn[value::FLOAT()] = "float";
          tn[value::DOUBLE()] = "double";
          tn[value::CHAR()] = "char";
          tn[value::STRING()] = "std::string";
          tn[value::BITSET()] = "bitsetofint::type";
          tn[value::BYTEARRAY()] = "bytearray::type";
          tn[value::LIST()] = "std::list<pnet::type::value::value_type>";
          tn[value::SET()] = "std::set<pnet::type::value::value_type>";
          tn[value::MAP()] = "std::map<pnet::type::value::value_type,pnet::type::value::value_type>";

          //\! Remove when old value type has been removed
          tn["stack"] = "std::list<pnet::type::value::value_type>";

          return tn;
        }

        const std::string typename_complete (const std::string& tname)
        {
          static map_type tn (init_typenames_complete());

          const map_type::const_iterator pos (tn.find (tname));

          if (pos == tn.end())
          {
            return (tname + "::type");
          }

          return pos->second;
        }
      }

      complete::complete (const std::string& tname)
        : _tname (tname)
      {}
      const std::string& complete::tname() const
      {
        return _tname;
      }
      std::ostream& operator<< (std::ostream& os, const complete& c)
      {
        return os << typename_complete (c.tname());
      }
    }
  }
}
