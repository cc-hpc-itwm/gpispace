#include "wfe_test.hpp"

#include <errno.h>

#include <list>

//#include <boost/thread.hpp>
//#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/getenv.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>


class WFEImpl_test : FHG_PLUGIN, public wfe::WFE
{

public:
  virtual ~WFEImpl_test() {}

  FHG_PLUGIN_START()
  {
	  FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
	  FHG_PLUGIN_STOPPED();
  }

  // ec > 0 -> FAILED
  // ec < 0 -> CANCELED
  // ec = 0 -> FINISHED

  int execute ( std::string const &job_id
              , std::string const &job_description
              , wfe::capabilities_t const & capabilities
              , std::string & result
              , wfe::meta_data_t const & meta_data
              )
  {
    int ec = 0; //FINISHED

    result = std::string("****************abcdefghijklmnopqrst************");

    return ec;
  }

  int cancel (std::string const &job_id)
  {
	  return 0;
  }
private:

};

EXPORT_FHG_PLUGIN( wfe
                 , WFEImpl_test
                 , "provides access to a workflow-engine"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
