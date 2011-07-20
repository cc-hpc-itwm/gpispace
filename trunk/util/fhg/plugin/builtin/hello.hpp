#ifndef FHG_PLUGIN_BUILTIN_HELLO_HPP
#define FHG_PLUGIN_BUILTIN_HELLO_HPP 1

namespace hello
{
  class Hello
  {
  public:
    virtual ~Hello () {}

    virtual void say() const = 0;
  };
}

#endif
