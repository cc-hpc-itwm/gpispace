// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/signature/of_type.hpp>
#include <we/type/value/signature/name.hpp>

#include <boost/unordered_map.hpp>

#include <stdexcept>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        typedef boost::unordered_map<std::string, value_type> by_name_type;

        by_name_type initialize()
        {
          by_name_type by_name;

          by_name[CONTROL()] = we::type::literal::control();
          by_name[BOOL()] = false;
          by_name[INT()] = 0;
          by_name[LONG()] = 0L;
          by_name[UINT()] = 0U;
          by_name[ULONG()] = 0UL;
          by_name[FLOAT()] = 0.0f;
          by_name[DOUBLE()] = 0.0;
          by_name[CHAR()] = '\0';
          by_name[STRING()] = std::string();
          by_name[BITSET()] = bitsetofint::type();
          by_name[BYTEARRAY()] = bytearray::type();
          by_name[LIST()] = std::list<value_type>();
          by_name[VECTOR()] = std::vector<value_type>();
          by_name[SET()] = std::set<value_type>();
          by_name[MAP()] = std::map<value_type,value_type>();

          return by_name;
        }
      }

      value_type of_type (const std::string& name)
      {
        static const by_name_type by_name (initialize());

        const by_name_type::const_iterator value (by_name.find (name));

        if (value == by_name.end())
          {
            throw std::runtime_error
              ("requested default value for unknown type " + name);
          }

        return value->second;
      }
    }
  }
}
