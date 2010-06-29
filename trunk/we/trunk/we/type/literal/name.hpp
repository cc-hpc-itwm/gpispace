// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_NAME_HPP
#define _WE_TYPE_LITERAL_NAME_HPP

#include <boost/unordered_set.hpp>

namespace literal
{
  typedef std::string type_name_t;

  static const type_name_t CONTROL ("control");
  static const type_name_t BOOL    ("bool");
  static const type_name_t LONG    ("long");
  static const type_name_t DOUBLE  ("double");
  static const type_name_t CHAR    ("char");
  static const type_name_t STRING  ("string");
  static const type_name_t BITSET  ("bitset");

  struct name
  {
  private:
    typedef boost::unordered_set<type_name_t> set_type;

    set_type set;

  public:
    name (void)
      : set()
    {
      set.insert (CONTROL);
      set.insert (BOOL);
      set.insert (LONG);
      set.insert (DOUBLE);
      set.insert (CHAR);
      set.insert (STRING);
      set.insert (BITSET);
    }

    bool valid (const type_name_t & x)
    {
      return (set.find (x) != set.end());
    }
  };
}

#endif
