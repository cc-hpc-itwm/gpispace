// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/valid_name.hpp>

#include <we/type/literal/name.hpp>

#include <boost/unordered_set.hpp>

namespace literal
{
  namespace
  {
    struct name
    {
    public:
      name()
        : _set()
      {
        _set.insert (CONTROL());
        _set.insert (BOOL());
        _set.insert (LONG());
        _set.insert (DOUBLE());
        _set.insert (CHAR());
        _set.insert (STRING());
        _set.insert (BITSET());
        _set.insert (STACK());
        _set.insert (MAP());
        _set.insert (SET());
        _set.insert (BYTEARRAY());
      }

      bool valid (const std::string& x) const
      {
        return (_set.find (x) != _set.end());
      }
    private:
      boost::unordered_set<std::string> _set;
    };
  }

  bool valid_name (const std::string& x)
  {
    static name n;

    return n.valid (x);
  }
}
