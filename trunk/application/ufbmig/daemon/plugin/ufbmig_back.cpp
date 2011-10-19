#include "ufbmig_back.hpp"
#include "ufbmig_front.hpp"

#include <errno.h>

#include <fstream>
#include <sstream>
#include <list>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

// plugin interfaces
#include "sdpactl.hpp"
#include "sdpac.hpp"

namespace job_type
{
  enum code
    {
      INITIALIZE,
      CALCULATE,
      FINALIZE,
    };
}

struct job_info_t
{
  int         type;     // INIT, CALC, FINAL, ...
  std::string id;
  int         state;
  std::string result;
};

class UfBMigBackImpl : FHG_PLUGIN
                     , public ufbmig::Backend
{
public:
  FHG_PLUGIN_START()
  {
    m_wf_path_initialize = fhg_kernel()->get("path.initialize", "/u/herc/petry/pnet/fail.pnet");
    m_frontend = 0;
    sdpa_ctl = fhg_kernel()->acquire<sdpa::Control>("sdpactl");
    if (0 == sdpa_ctl)
    {
      MLOG(ERROR, "could not acquire sdpa::Control plugin");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    sdpa_c = fhg_kernel()->acquire<sdpa::Client>("sdpac");
    if (0 == sdpa_c)
    {
      MLOG(ERROR, "could not acquire sdpa::Client plugin");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_frontend = 0;
    FHG_PLUGIN_STOPPED();
  }

  void registerFrontend(ufbmig::Frontend* f)
  {
    // insert a listener that is called when:
    //     initialize finihed
    //          initialize_done(error)
    //     calculate finished
    //          calculate_done(error)
    //     finalize finished
    //          finalize_done(error)
    m_frontend = f;
  }

  int initialize(std::string const &desc)
  {
    MLOG(INFO, "submitting INITIALIZE workflow");

    job_info_t job_info;
    int ec;

    job_info.type = job_type::INITIALIZE;

    // write xml desc to file

    // read workflow from file
    //    use wfe plugin?
    //        Workflow wfe->parse(string);
    //        Workflow.put("port", value);
    //        Workflow.get<>("port");
    // place tokens
    // submit job
    ec = sdpa_c->submit( read_workflow_from_file(m_wf_path_initialize)
                       , job_info.id
                       );

    // append job_info_t
    // start kernel task to query job state
    //    std::string res;
    //    sdpa_c->execute(read_workflow_from_file("/u/herc/petry/pnet/fail.pnet"), res);

    return ec;
  }

  int update_salt_mask (const char *data, size_t len)
  {
    MLOG(INFO, "updating SALTMASK");
    return 0;
  }

  int calculate()
  {
    MLOG(INFO, "submitting CALCULATE workflow");

    job_info_t job_info;
    int ec;

    job_info.type = job_type::CALCULATE;

    // write xml desc to file

    // read workflow from file
    //    use wfe plugin?
    //        Workflow wfe->parse(string);
    //        Workflow.put("port", value);
    //        Workflow.get<>("port");
    // place tokens
    // submit job
    ec = sdpa_c->submit( read_workflow_from_file(m_wf_path_initialize)
                       , job_info.id
                       );

    // append job_info_t
    // start kernel task to query job state
    //    std::string res;
    //    sdpa_c->execute(read_workflow_from_file("/u/herc/petry/pnet/fail.pnet"), res);

    return ec;
  }

  int finalize()
  {
    MLOG(INFO, "submitting FINALIZE workflow");

    job_info_t job_info;
    int ec;

    job_info.type = job_type::FINALIZE;

    // write xml desc to file

    // read workflow from file
    //    use wfe plugin?
    //        Workflow wfe->parse(string);
    //        Workflow.put("port", value);
    //        Workflow.get<>("port");
    // place tokens
    // submit job
    ec = sdpa_c->submit( read_workflow_from_file(m_wf_path_initialize)
                       , job_info.id
                       );

    // append job_info_t
    // start kernel task to query job state
    //    std::string res;
    //    sdpa_c->execute(read_workflow_from_file("/u/herc/petry/pnet/fail.pnet"), res);

    return ec;
  }

  int cancel()
  {
    MLOG(INFO, "CANCELLING running workflows");
    return 0;
  }

  size_t output_volume_size ()
  {
    return 0;
  }

  int read_output_volume (char *buffer, size_t buffer_size, size_t offset)
  {
    // remaining = sizeof(output_volume) - offset;
    // memcpy(scratch, output_volume+offset, 0, min(scratch_size, remaining));
    // memcpy(shared_mem, scratch, min(remaining, scratch_size));
    // memcpy to buffer
    return 0;
  }

  size_t salt_mask_size ()
  {
    return 0;
  }
private:
  std::string read_workflow_from_file (std::string const & path)
  {
    std::ifstream ifs(path.c_str());
    if (! ifs.good())
    {
      MLOG(ERROR, "could not open: " << path);
      throw std::runtime_error("could not open: " + path);
    }

    std::stringstream sstr;
    ifs >> sstr.rdbuf();
    return sstr.str();
  }

  sdpa::Control * sdpa_ctl;
  sdpa::Client * sdpa_c;
  ufbmig::Frontend *m_frontend;
  std::list<job_info_t> m_job_info_list;

  // workflow paths
  std::string m_wf_path_initialize;
};

EXPORT_FHG_PLUGIN( ufbmig_back
                 , UfBMigBackImpl
                 , "provides the backend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "sdpactl,sdpac"
                 , ""
                 );
