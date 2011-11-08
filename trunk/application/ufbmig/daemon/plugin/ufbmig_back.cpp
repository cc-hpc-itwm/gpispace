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
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

namespace job
{
  namespace type
  {
    enum code
      {
        INITIALIZE,
        UPDATE,
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
    case job::type::UPDATE:
      return "UPDATE";
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

namespace state
{
  /*
                                    CALCULATING
                                      ^   |
                                calc  |   | done
                                      |   v
    UNINITIALIZED -- initialize --> INITIALIZED -- finalize ---.
           ^                          |    ^                   |
           |                   update |    | done              |
           |                          v    |                   |
           |                         UPDATING                  |
           |                                                   |
           |                                                  /
           `-----------------------  FINALIZING <------------'
   */
  enum code
    {
      UNINITIALIZED = 0,
      INITIALIZED,
      CALCULATING,
      UPDATING,
      FINALIZING,
    };
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
    m_control_sdpa = fhg_kernel()->get<bool>("control_sdpa", "0");

    m_wf_path_initialize =
      fhg_kernel()->get("wf_init", "ufbmig_init.pnet");
    if (! fs::exists(m_wf_path_initialize))
    {
      MLOG(ERROR, "cannot access INITIALIZE workflow: " << m_wf_path_initialize);
      FHG_PLUGIN_FAILED(ENOENT);
    }

    m_wf_path_mask =
      fhg_kernel()->get("wf_mask", "ufbmig_mask.pnet");
    if (! fs::exists(m_wf_path_mask))
    {
      MLOG(ERROR, "cannot access UPDATE workflow: " << m_wf_path_mask);
      FHG_PLUGIN_FAILED(ENOENT);
    }

    m_wf_path_calculate =
      fhg_kernel()->get("wf_calc", "ufbmig_calc.pnet");
    if (! fs::exists(m_wf_path_calculate))
    {
      MLOG(ERROR, "cannot access CALCULATE workflow: " << m_wf_path_calculate);
      FHG_PLUGIN_FAILED(ENOENT);
    }

    m_wf_path_finalize =
      fhg_kernel()->get("wf_done", "ufbmig_done.pnet");
    if (! fs::exists(m_wf_path_finalize))
    {
      MLOG(ERROR, "cannot access FINALIZE workflow: " << m_wf_path_finalize);
      FHG_PLUGIN_FAILED(ENOENT);
    }

    m_file_with_config =
      fhg_kernel()->get("config", "config.token");
    m_file_with_mask =
      fhg_kernel()->get("saltmask", "saltmask");

    if (fs::exists(m_file_with_config))
    {
      MLOG(INFO, "trying to recover from existing config...");
      // TODO: not implemented, since gpid kills gpi-space anyways...
      //    i.e. recovery is currently not possible
    }

    if (fs::exists(m_file_with_mask))
    {
      try
      {
        fs::ofstream of(m_file_with_mask);
        if (! of)
        {
          throw std::runtime_error ("could not open file");
        }
      }
      catch (std::exception const & ex)
      {
        MLOG(ERROR, "could not create saltmask state in: " << m_file_with_mask << ": " << ex.what());
        FHG_PLUGIN_FAILED(EIO);
      }
    }

    sdpa_c = fhg_kernel()->acquire<sdpa::Client>("sdpac");
    if (0 == sdpa_c)
    {
      MLOG(ERROR, "could not acquire sdpa::Client plugin");
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    gpi_api = fhg_kernel()->acquire<gpi::GPI>("gpi");
    if (0 == gpi_api)
    {
      MLOG(WARN, "could not acquire gpi::GPI plugin");
    }

    if (m_control_sdpa)
    {
      sdpa_ctl = fhg_kernel()->acquire<sdpa::Control>("sdpactl");
      if (0 == sdpa_ctl)
      {
        MLOG(WARN, "could not acquire sdpa::Control plugin, cannot control SDPA!");
        m_control_sdpa = false;
      }
    }
    else
    {
      sdpa_ctl = 0;
    }

    m_frontend = 0;
    m_state = state::UNINITIALIZED;

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
    if (state::UNINITIALIZED != m_state)
    {
      MLOG(ERROR, "state mismatch: cannot initialize again in state: " << m_state);
      return -EINVAL;
    }

    if (m_control_sdpa && sdpa_ctl)
    {
      MLOG(INFO, "restarting SDPA...");
      sdpa_ctl->restart();
    }

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
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "cannot update salt mask currently, invalid state: " << m_state);
      return -EAGAIN;
    }

    MLOG(TRACE, "writing new salt mask");

    {
      fs::ofstream salt_mask_stream (m_file_with_mask);
      salt_mask_stream.write (data, len);
      salt_mask_stream.close();
    }

    MLOG(INFO, "submitting SALTMASK workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_mask));

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
      we::util::token::put (act, "file_with_mask", m_file_with_mask);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not put tokens: " << ex.what());
      return -EINVAL;
    }

    m_state = state::UPDATING;
    return submit_job(we::util::codec::encode(act), job::type::UPDATE);
  }

  int calculate(std::string const &xml)
  {
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "cannot calculate right now, invalid state: " << m_state);
      return -EAGAIN;
    }

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

    int ec = submit_job(we::util::codec::encode(act), job::type::CALCULATE);
    if (0 == ec)
    {
      m_state = state::CALCULATING;
    }
    return ec;
  }

  int finalize()
  {
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "finalizing from invalid state: " << m_state);
      MLOG(WARN, "this will probably result in failed executions");
    }

    if (! fs::exists(m_file_with_config))
    {
      MLOG(INFO, "no config file found, finalize not possible!");
      return -EINVAL;
    }

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

    int ec = submit_job(we::util::codec::encode(act), job::type::FINALIZE);
    if (0 == ec)
    {
      m_state = state::FINALIZING;
    }
    return ec;
  }

  int cancel()
  {
    lock_type lock (m_job_list_mutex);
    if (m_job_list.size())
    {
      MLOG(INFO, "CANCELLING running workflows");
      BOOST_FOREACH(job::info_t const & j, m_job_list)
      {
        if (! job::is_done(j))
          sdpa_c->cancel(j.id);
      }
    }
    else
    {
      MLOG(INFO, "nothing to cancel!");
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

  int handle_update_salt_mask_result (we::activity_t const &result)
  {
    return 0;
  }

  int handle_calculate_result (we::activity_t const &result)
  {
    return 0;
  }

  int handle_finalize_result (we::activity_t const &result)
  {
    if (fs::exists(m_file_with_config))
    {
      fs::remove(m_file_with_config);
    }
    if (fs::exists(m_file_with_mask))
    {
      fs::remove(m_file_with_mask);
    }

    return 0;
  }

  int handle_completed_job (job::info_t const &j)
  {
    LOG( INFO
       , "job returned: " << job::type_to_name(j.type) << " "
       << job::status_to_string(j.state)
       );

    int ec = j.error;

    we::activity_t result;
    if (0 == ec)
    {
      try
      {
        we::util::codec::decode(j.result, result);
      }
      catch (std::exception const &ex)
      {
        ec = -EINVAL;
      }
    }

    switch (j.type)
    {
    case job::type::INITIALIZE:
      if (0 == ec)
      {
        m_state = state::INITIALIZED;
        ec = handle_initialize_result(result);
      }

      if (m_frontend) m_frontend->initialize_done(ec);
      break;
    case job::type::UPDATE:
      if (0 == ec)
      {
        ec = handle_update_salt_mask_result(result);
      }
      m_state = state::INITIALIZED;

      if (m_frontend) m_frontend->salt_mask_done(ec);
      break;
    case job::type::CALCULATE:
      if (0 == ec)
      {
        ec = handle_calculate_result(result);
      }
      m_state = state::INITIALIZED;

      if (m_frontend) m_frontend->calculate_done(ec);
      break;
    case job::type::FINALIZE:
      if (0 == ec)
      {
        ec = handle_finalize_result(result);
      }
      m_state = state::UNINITIALIZED;

      if (m_frontend) m_frontend->finalize_done(ec);
      break;
    default:
      return -EINVAL;
    }

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

  bool m_control_sdpa;
  sdpa::Control * sdpa_ctl;
  sdpa::Client * sdpa_c;
  gpi::GPI *gpi_api;
  ufbmig::Frontend *m_frontend;
  std::list<job::info_t> m_job_info_list;

  // state
  int m_state;

  // workflow paths
  std::string m_wf_path_initialize;
  std::string m_wf_path_mask;
  std::string m_wf_path_calculate;
  std::string m_wf_path_finalize;

  // workflow configs
  pnetc::type::config::config config;
  std::string m_file_with_config;
  std::string m_file_with_mask;
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
