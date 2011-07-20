#ifndef FHG_PLUGIN_BUILTIN_WORLD_HPP
#define FHG_PLUGIN_BUILTIN_WORLD_HPP 1

namespace world
{
  class World
  {
  public:
    virtual ~World () {}

    virtual void say() const = 0;
  };
}

#endif
