#include "wfe.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>

class WFEImpl : FHG_PLUGIN
              , public wfe::WFE
{
public:
  virtual ~WFEImpl() {}

  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int execute ( std::string const &job_id
              , std::string const &job_description
              , wfe::capabilities_t const & capabilities
              , std::string & result
              , boost::posix_time::time_duration = boost::posix_time::seconds(0)
              )
  {
    MLOG(INFO, "executing...");

    // parse description
    //   delegate (activity, id, callback) to thread + callback
    //      run execute loop
    // wait for finish/fail whatever using walltime
    //      cancel task-loop if there is any
    return -EINVAL;
  }

  int cancel (std::string const &job_id)
  {
    return -ESRCH;
  }
private:
};

EXPORT_FHG_PLUGIN( wfe
                 , WFEImpl
                 , "provides access to a workflow-engine"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
