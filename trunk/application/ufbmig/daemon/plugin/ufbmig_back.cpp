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
#include "gpi.hpp"

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

namespace job
{
  namespace type
  {
    enum code
      {
        INITIALIZE,
        CALCULATE,
        FINALIZE,
      };
  }

  struct info_t
  {
    int         type;     // INIT, CALC, FINAL, ...
    std::string id;
    int         state;
    std::string result;
  };

  static bool is_done(info_t const &info)
  {
    return info.state > sdpa::status::RUNNING;
  }

  static int state_to_result_code (int state)
  {
    if (sdpa::status::FINISHED == state) return 0;
    if (sdpa::status::CANCELED == state) return -ECANCELED;
    if (sdpa::status::FAILED   == state) return -EFAULT;
    else                                 return -EFAULT;
  }
}

class UfBMigBackImpl : FHG_PLUGIN
                     , public ufbmig::Backend
{
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
  typedef boost::condition_variable_any condition_type;
  typedef std::list<job::info_t> job_list_t;

public:
  FHG_PLUGIN_START()
  {
    m_wf_path_initialize =
      fhg_kernel()->get("wf_init", "/u/herc/petry/pnet/ufbmig/init.pnet");
    m_wf_path_calculate =
      fhg_kernel()->get("wf_calc", "/u/herc/petry/pnet/ufbmig/calc.pnet");
    m_wf_path_finalize =
      fhg_kernel()->get("wf_done", "/u/herc/petry/pnet/ufbmig/done.pnet");

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

    // TODO: lazy acquire gpi
    // gpi = fhg_kernel()->acquire<gpi::GPI>("gpi");

    fhg_kernel()->schedule ( boost::bind( &UfBMigBackImpl::update_job_states
                                        , this
                                        )
                           , 1
                           );

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

  int initialize(std::string const &xml)
  {
    MLOG(INFO, "submitting INITIALIZE workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_initialize));

    // place tokens

    return submit_job(wf, job::type::INITIALIZE);
  }

  int update_salt_mask (const char *data, size_t len)
  {
    MLOG(INFO, "updating SALTMASK");
    return 0;
  }

  int calculate(std::string const &xml)
  {
    MLOG(INFO, "submitting CALCULATE workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_calculate));

    // place tokens

    // read workflow from file
    //    use wfe plugin?
    //        Workflow wfe->parse(string);
    //        Workflow.put("port", value);
    //        Workflow.get<>("port");
    // place tokens
    // submit job

    return submit_job(wf, job::type::CALCULATE);
  }

  int finalize()
  {
    MLOG(INFO, "submitting FINALIZE workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_finalize));
    return submit_job(wf, job::type::FINALIZE);
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

  int submit_job (std::string const &wf, int type)
  {
    int ec;
    job::info_t job_info;

    // append job_info_t
    // start kernel task to query job state
    //    std::string res;
    //    sdpa_c->execute(read_workflow_from_file("/u/herc/petry/pnet/fail.pnet"), res);

    job_info.type = type;
    ec = sdpa_c->submit( wf
                       , job_info.id
                       );
    if (0 == ec)
    {
      lock_type lock (m_job_list_mutex);
      m_job_list.push_back(job_info);
    }

    return ec;
  }

  void notify_frontend_about_job (job::info_t const &j)
  {
    if (! m_frontend) return;

    int ec = job::state_to_result_code(j.state);

    switch (j.type)
    {
    case job::type::INITIALIZE:
      m_frontend->initialize_done(ec);
      break;
    case job::type::CALCULATE:
      m_frontend->calculate_done(ec);
      break;
    case job::type::FINALIZE:
      m_frontend->finalize_done(ec);
      break;
    }
  }

  void close_job (job::info_t const &j)
  {
    // parse and handle result

    std::string result;
    sdpa_c->result(j.id, result);

    LOG(INFO, "job finished: " << result);

    sdpa_c->remove(j.id);
    notify_frontend_about_job(j);
  }

  void update_job_states ()
  {
    lock_type lock (m_job_list_mutex);

    for ( job_list_t::iterator j(m_job_list.begin())
        ; j != m_job_list.end()
        ; ++j
        )
    {
      j->state = sdpa_c->status(j->id);
      if (job::is_done(*j))
      {
        close_job (*j);
        j = m_job_list.erase(j);
        if (j == m_job_list.end()) break;
      }
    }

    fhg_kernel()->schedule ( boost::bind( &UfBMigBackImpl::update_job_states
                                        , this
                                        )
                           , 1
                           );
  }

  mutex_type m_job_list_mutex;
  job_list_t m_job_list;
  sdpa::Control * sdpa_ctl;
  sdpa::Client * sdpa_c;
  ufbmig::Frontend *m_frontend;
  std::list<job::info_t> m_job_info_list;

  // workflow paths
  std::string m_wf_path_initialize;
  std::string m_wf_path_calculate;
  std::string m_wf_path_finalize;
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
