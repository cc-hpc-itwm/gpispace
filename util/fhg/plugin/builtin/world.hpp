#ifndef FHG_PLUGIN_BUILTIN_WORLD_HPP
#define FHG_PLUGIN_BUILTIN_WORLD_HPP 1

#include <string>

namespace example
{
  class World
  {
  public:
    virtual ~World () {}

    virtual std::string text() const = 0;
  };
}

#endif
