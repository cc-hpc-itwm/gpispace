#include "drts.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

//#include

class DRTSImpl : FHG_PLUGIN
               , public drts::DRTS
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

  int exec (drts::job_desc_t const &, drts::job_id_t &, drts::JobListener *)
  {
    return -EPERM;
  }
  drts::status::status_t query (drts::job_id_t const & jobid)
  {
    return drts::status::FAILED;
  }
  int cancel (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }
  int results (drts::job_id_t const & jobid, std::string &)
  {
    return -ESRCH;
  }
  int remove (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }
private:
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
