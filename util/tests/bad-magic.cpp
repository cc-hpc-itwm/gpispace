#include <iostream>
#include <fhg/plugin/magic.hpp>
#undef FHG_PLUGIN_API_VERSION
#define FHG_PLUGIN_API_VERSION "bad-version-magic"
#include <fhg/plugin/plugin.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class BadMagicImpl : FHG_PLUGIN
{
public:
  BadMagicImpl ()
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

EXPORT_FHG_PLUGIN( bad_magic
                 , BadMagicImpl
                 , ""
                 , "bad magic test plugin"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
