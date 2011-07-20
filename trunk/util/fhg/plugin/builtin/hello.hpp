#ifndef FHG_PLUGIN_BUILTIN_HELLO_HPP
#define FHG_PLUGIN_BUILTIN_HELLO_HPP 1

#include <string>

namespace example
{
  class Hello
  {
  public:
    virtual ~Hello () {}

    virtual std::string text() const = 0;
  };
}

#endif
