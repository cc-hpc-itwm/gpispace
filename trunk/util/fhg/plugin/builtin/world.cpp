#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/world.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class WorldImpl : FHG_PLUGIN
                , public example::World
{
public:
  WorldImpl () {}
  ~WorldImpl () {}

  FHG_PLUGIN_START(kernel)
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    FHG_PLUGIN_STOPPED();
  }

  std::string text () const
  {
    return "World!";
  }
};

EXPORT_FHG_PLUGIN( world
                 , WorldImpl
                 , "say world"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
