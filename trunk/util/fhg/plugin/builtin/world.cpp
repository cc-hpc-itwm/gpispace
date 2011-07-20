#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/world.hpp>

class WorldImpl : IS_A_FHG_PLUGIN
                , public world::World
{
public:
  WorldImpl () {}
  ~WorldImpl () {}

  FHG_PLUGIN_START(config)
  {
    return 0;
  }

  FHG_PLUGIN_STOP()
  {
    return 0;
  }

  void say () const
  {
    std::cout << "world!" << std::endl;
  }
};

FHG_PLUGIN( world
          , WorldImpl
          , "say world"
          , "Alexander Petry <petry@itwm.fhg.de>"
          , "v0.0.1"
          , "GPL"
          , "hello"
          , ""
          );
