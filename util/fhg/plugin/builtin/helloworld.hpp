#ifndef FHG_PLUGIN_BUILTIN_HELLOWORLD_HPP
#define FHG_PLUGIN_BUILTIN_HELLOWORLD_HPP 1

#include <ostream>

namespace example
{
  class HelloWorld
  {
  public:
    virtual ~HelloWorld () {}

    virtual void say(std::ostream & os) const = 0;
  };
}

#endif
