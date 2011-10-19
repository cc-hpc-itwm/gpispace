#include <stdlib.h> // system

#include "sdpactl.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

class ControlImpl : FHG_PLUGIN
                  , public sdpa::Control
{
public:
  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int start ()
  {
    int ec = 0;
    ec += system("sdpa start gpi");
    ec += system("sdpa start orch");
    ec += system("sdpa start agg");
    ec += system("sdpa start nre");
    ec += system("sdpa start drts");
    return ec;
  }

  int restart ()
  {
    int ec = 0;
    ec += stop ();
    sleep(3);
    ec += start ();
    return ec;
  }

  int stop ()
  {
    int ec = 0;
    ec += system("sdpa stop drts");
    ec += system("sdpa stop nre");
    ec += system("sdpa stop agg");
    ec += system("sdpa stop orch");
    return ec;
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
