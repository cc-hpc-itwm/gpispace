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

// ufbmig types
#include <we/we.hpp>
#include <we/util/token.hpp>
#include <pnetc/type/config.hpp>

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
    int         error;
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

  static std::string type_to_name (int type)
  {
    switch (type)
    {
    case job::type::INITIALIZE:
      return "INITIALIZE";
      break;
    case job::type::CALCULATE:
      return "CALCULATE";
      break;
    case job::type::FINALIZE:
      return "FINALIZE";
      break;
    default:
      return "UNKNOWN";
    }
  }

  std::string status_to_string(int s)
  {
    if (s == sdpa::status::PENDING)   return "Pending";
    if (s == sdpa::status::RUNNING)   return "Running";
    if (s == sdpa::status::FINISHED)  return "Finished";
    if (s == sdpa::status::FAILED)    return "Failed";
    if (s == sdpa::status::CANCELED)  return "Cancelled";
    if (s == sdpa::status::SUSPENDED) return "Suspended";
    else                              return "Unknown";
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

    m_file_with_config =
      fhg_kernel()->get("config", "/fhgfs/HPC/petry/scratch/ufbmig/config.token");

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

    gpi_api = fhg_kernel()->acquire<gpi::GPI>("gpi");
    if (0 == gpi_api)
    {
      MLOG(WARN, "could not acquire gpi::GPI plugin");
    }

    // restore from existing state:
    //   if file exists(config_file)
    //     read it
    //     check handles
    //     if something fails:
    //        submit finalize workflow, to deallocate possible left-overs
    //     else
    //        set state to initialized

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

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (plugin == "gpi")
    {
      gpi_api = fhg_kernel()->acquire<gpi::GPI>("gpi");
      if (0 == gpi_api)
      {
        MLOG(ERROR, "gpi plugin doesn't implement GPI!");
      }
    }
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

    we::activity_t act;

    try
    {
      we::util::codec::decode(wf, act);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "decoding workflow failed: " << ex.what());
      return -EINVAL;
    }

    // place tokens
    try
    {
      we::util::token::put (act, "description", xml);
      we::util::token::put (act, "file_with_config", m_file_with_config);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not put tokens: " << ex.what());
      return -EINVAL;
    }

    return submit_job(we::util::codec::encode(act), job::type::INITIALIZE);
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

    we::activity_t act;

    try
    {
      we::util::codec::decode(wf, act);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "decoding workflow failed: " << ex.what());
      return -EINVAL;
    }

    // place tokens
    try
    {
      we::util::token::put (act, "file_with_config", m_file_with_config);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not put token: " << ex.what());
      return -EINVAL;
    }

    return submit_job(we::util::codec::encode(act), job::type::CALCULATE);
  }

  int finalize()
  {
    MLOG(INFO, "submitting FINALIZE workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_finalize));
    we::activity_t act;

    try
    {
      we::util::codec::decode(wf, act);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "decoding workflow failed: " << ex.what());
      return -EINVAL;
    }

    // place tokens
    try
    {
      we::util::token::put (act, "file_with_config", m_file_with_config);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not put token: " << ex.what());
      return -EINVAL;
    }

    return submit_job(we::util::codec::encode(act), job::type::FINALIZE);
  }

  int cancel()
  {
    MLOG(INFO, "CANCELLING running workflows");
    lock_type lock (m_job_list_mutex);
    BOOST_FOREACH(job::info_t const & j, m_job_list)
    {
      if (! job::is_done(j))
        sdpa_c->cancel(j.id);
    }
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
    ifs >> std::noskipws >> sstr.rdbuf();
    return sstr.str();
  }

  int submit_job (std::string const &wf, int type)
  {
    int ec;
    job::info_t job_info;

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

  void notify_frontend_about_completed_job (job::info_t const &j)
  {
    if (!m_frontend) return;

    switch (j.type)
    {
    case job::type::INITIALIZE:
      m_frontend->initialize_done(j.error);
      break;
    case job::type::CALCULATE:
      m_frontend->calculate_done(j.error);
      break;
    case job::type::FINALIZE:
      m_frontend->finalize_done(j.error);
      break;
    }
  }

  int handle_initialize_result (we::activity_t const &result)
  {
    try
    {
      we::util::token::list_t output (we::util::token::get (result, "config"));
      if (output.empty())
        throw std::runtime_error("empty list");
      config = pnetc::type::config::from_value(output.front());
      MLOG(INFO, "got config: " << config);
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not get config token: " << ex.what());
    }
    return 0;
  }

  int handle_calculate_result (we::activity_t const &result)
  {
    return 0;
  }

  int handle_finalize_result (we::activity_t const &result)
  {
    return 0;
  }

  int handle_job_result (int type, we::activity_t const &result)
  {
    switch (type)
    {
    case job::type::INITIALIZE:
      return handle_initialize_result(result);
      break;
    case job::type::CALCULATE:
      return handle_calculate_result(result);
      break;
    case job::type::FINALIZE:
      return handle_finalize_result(result);
      break;
    default:
      return -EINVAL;
    }
  }

  int handle_completed_job (job::info_t const &j)
  {
    int ec = 0;

    LOG( INFO
       , "job returned: " << job::type_to_name(j.type) << " "
       << job::status_to_string(j.state)
       );

    if (0 == j.error)
    {
      we::activity_t result;
      try
      {
        we::util::codec::decode(j.result, result);
        ec = handle_job_result (j.type, result);
      }
      catch (std::exception const &ex)
      {
        MLOG(ERROR, "could not decode result: " << ex.what());
        ec = -EINVAL;
      }
    }
    else
    {
      ec = j.error;
    }

    notify_frontend_about_completed_job(j);

    return ec;
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
        j->error = job::state_to_result_code(j->state);
        sdpa_c->result(j->id, j->result);
        sdpa_c->remove(j->id);
        handle_completed_job (*j);
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
  gpi::GPI *gpi_api;
  ufbmig::Frontend *m_frontend;
  std::list<job::info_t> m_job_info_list;

  // state
  int migration_state;

  // workflow paths
  std::string m_wf_path_initialize;
  std::string m_wf_path_calculate;
  std::string m_wf_path_finalize;

  // workflow configs
  pnetc::type::config::config config;
  std::string m_file_with_config;
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
