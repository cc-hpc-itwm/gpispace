#include <iostream>

#include <fhg/plugin/plugin.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class GoodMagicImpl : FHG_PLUGIN
{
public:
  GoodMagicImpl ()
  {}

  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }
private:
};

EXPORT_FHG_PLUGIN( good_magic
                 , GoodMagicImpl
                 , ""
                 , "good magic test plugin"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
