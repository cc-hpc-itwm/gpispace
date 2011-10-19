#include "ufbmig_front.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

class UfBMigFrontImpl : FHG_PLUGIN
                      , public ufbmig::Frontend
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

  int initialize()
  {
    return 0;
  }

  int calculate()
  {
    return 0;
  }

  int finalize()
  {
    return 0;
  }

  int cancel()
  {
    return 0;
  }

  void initialize_done (int)
  {
  }

  void calculate_done (int)
  {
  }

  void finalize_done (int)
  {
  }

  void cancel_done (int)
  {
  }
};

EXPORT_FHG_PLUGIN( ufbmig_front
                 , UfBMigFrontImpl
                 , "provides the frontend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "ufbmig_back"
                 , ""
                 );
