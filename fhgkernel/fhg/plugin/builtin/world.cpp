#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/world.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class WorldImpl : FHG_PLUGIN
                , public example::World
{
public:
  FHG_PLUGIN_START()
  {
    m_text = fhg_kernel()->get("text", "World");
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  std::string text () const
  {
    return m_text;
  }
private:
  std::string m_text;
};

EXPORT_FHG_PLUGIN( world
                 , WorldImpl
                 , ""
                 , "say world"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
