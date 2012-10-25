// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_VALIDNAME_HPP
#define _WE_TYPE_LITERAL_VALIDNAME_HPP

#include <we/type/literal/name.hpp>

#include <boost/unordered/unordered_set.hpp>

namespace literal
{
  struct name
  {
  private:
    typedef boost::unordered_set<type_name_t> set_type;

    set_type set;

  public:
    name (void) : set()
    {
      set.insert (CONTROL());
      set.insert (BOOL());
      set.insert (LONG());
      set.insert (DOUBLE());
      set.insert (CHAR());
      set.insert (STRING());
      set.insert (BITSET());
      set.insert (STACK());
      set.insert (MAP());
      set.insert (SET());
      set.insert (BYTEARRAY());
    }

    bool valid (const type_name_t & x) const
    {
      return (set.find (x) != set.end());
    }
  };

  inline bool valid_name (const type_name_t & x)
  {
    static name n;

    return n.valid (x);
  }
}

#endif
