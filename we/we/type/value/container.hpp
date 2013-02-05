// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_CONTAINER_HPP
#define _WE_TYPE_VALUE_CONTAINER_CONTAINER_HPP 1

#include <we/type/value.hpp>

#include <boost/unordered_map.hpp>

#include <string>
#include <list>
#include <stdexcept>

#include <iosfwd>

namespace value
{
  namespace container
  {
    typedef boost::unordered_map<std::string, value::type> type;
    typedef std::list<std::string> key_vec_t;

    value::type bind (type&, const std::string&, const value::type&);
    const value::type& value (const type&, const std::string&);

    namespace exception
    {
      class missing_binding : public std::runtime_error
      {
      public:
        explicit missing_binding (const std::string& key)
          : std::runtime_error ("missing binding for: ${" + key + "}")
        {};
      };
    }
  }
}

namespace std
{
  std::ostream& operator<< (std::ostream&, const value::container::type&);
}

#endif
