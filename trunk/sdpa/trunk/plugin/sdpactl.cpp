#include <stdlib.h> // system

#include "sdpactl.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

class ControlImpl : FHG_PLUGIN
                  , public sdpactl::Control
{
public:
  FHG_PLUGIN_START()
  {
    restart ();
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int start ()
  {
    return system("sdpa start");
  }

  int restart ()
  {
    return system("sdpa restart");
  }

  int stop ()
  {
    return system("sdpa stop");
  }
};

EXPORT_FHG_PLUGIN( sdpactl
                 , ControlImpl
                 , "provides control functions for SDPA"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
