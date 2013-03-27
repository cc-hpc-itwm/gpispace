#include "ufbmig_back.hpp"
#include "ufbmig_front.hpp"

#include <errno.h>

#include <fstream>
#include <sstream>
#include <list>
#include <stack>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/error_codes.hpp>
#include <fhg/util/bool.hpp>
#include <fhg/util/bool_io.hpp>

// plugin interfaces
#include "sdpactl.hpp"
#include "sdpac.hpp"
#include "gpi.hpp"
#include "progress.hpp"

// ufbmig types
#include <we/mgmt/type/activity.hpp>
#include <we/type/net.hpp>
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
        PREPARE,
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
    std::string error_message;
    std::string result;
  };

  static bool is_done(info_t const &info)
  {
    return info.state < sdpa::status::PENDING;
  }

  static std::string type_to_name (int type)
  {
    switch (type)
    {
    case job::type::PREPARE:
      return "PREPARE";
      break;
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

struct gpi_stream
{
  int                     fd;
  gpi::pc::type::handle::descriptor_t handle;
  uint64_t                read_pointer;
  uint64_t                write_pointer;
};

class UfBMigBackImpl : FHG_PLUGIN
                     , public ufbmig::Backend
{
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
  typedef boost::condition_variable_any condition_type;
  typedef std::list<job::info_t> job_list_t;
  typedef boost::shared_ptr<gpi_stream> stream_ptr_t;
  typedef std::map<int, stream_ptr_t> stream_map_t;
  typedef std::stack<int> stream_fd_stack_t;

public:
  FHG_PLUGIN_START()
  {
    m_control_sdpa =
      fhg_kernel()->get<fhg::util::bool_t>("control_sdpa", "true");
    m_check_interval =
      fhg_kernel ()->get<std::size_t>("check_interval", 1800);
    // (10**6) / fhg_kernel ()->tick_time () * 360 // seconds

    m_wf_path_prepare =
      fhg_kernel ()->get("wf_prepare", "ufbmig_prepare.pnet");
    if (! fs::exists(m_wf_path_prepare))
    {
      MLOG(ERROR, "cannot access PREPARE workflow: " << m_wf_path_prepare);
      FHG_PLUGIN_FAILED(ENOENT);
    }

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

    m_chunk_size = fhg_kernel()->get<std::size_t>("chunk_size", "4194304");

    // allocate enough to have double buffering
    m_transfer_segment_size = m_chunk_size;

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
      FHG_PLUGIN_FAILED (ESRCH);
    }

    progress = fhg_kernel()->acquire<progress::Progress>("progress");
    if (0 == progress)
    {
      MLOG (ERROR, "could not acquire progress::Progress plugin");
      FHG_PLUGIN_FAILED (ESRCH);
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

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_frontend = 0;

    {
      lock_type lock (m_stream_mutex);
      while (not m_streams.empty())
      {
        close (m_streams.begin()->first);
      }
      while (not m_stream_fds.empty())
      {
        m_stream_fds.pop();
      }
    }

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (plugin == "gpi")
    {
      gpi_api = fhg_kernel()->acquire<gpi::GPI>(plugin);
      if (0 == gpi_api)
      {
        MLOG(ERROR, "gpi plugin doesn't implement GPI!");
      }
      else
      {
        reinitialize_gpi_state ();
      }
    }
    else if (plugin == "progress")
    {
      progress = fhg_kernel()->acquire<progress::Progress>(plugin);
      if (0 == progress)
      {
        MLOG(ERROR, "progress plugin does not implement Progress");
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

  int prepare ()
  try
  {
    reset_progress ("prepare");

    update_progress (10);

    if (m_control_sdpa && sdpa_ctl)
    {
      MLOG (INFO, "(re)starting SDPA...");

      int rc = 0;

      rc += sdpa_ctl->start ("gpi")  ? 1 : 0;
      update_progress (20);

      rc += sdpa_ctl->start ("orch") ? 2 : 0;
      update_progress (40);

      rc += sdpa_ctl->start ("agg")  ? 4 : 0;
      update_progress (60);

      rc += sdpa_ctl->start ("drts") ? 8 : 0;
      update_progress (80);

      if (rc != 0)
      {
        MLOG (WARN, "start of SDPA failed: " << rc);
        return fhg::error::SDPA_NOT_STARTABLE;
      }

      if (0 != sdpa_ctl->status ("gpi"))
      {
        MLOG (ERROR, "gpi failed to start, giving up");
        return fhg::error::GPI_UNAVAILABLE;
      }
    }

    const std::string wf (read_workflow_from_file(m_wf_path_prepare));

    we::mgmt::type::activity_t act (wf);

    update_progress (95);

    if (m_check_interval)
      fhg_kernel()->schedule ( "check_worker"
                             , boost::bind( &UfBMigBackImpl::check_worker
                                          , this
                                          )
                             , m_check_interval
                             );

    return submit_job (act.to_string(), job::type::PREPARE);
  }
  catch (std::exception const &ex)
  {
    MLOG (ERROR, "decoding PREPARE workflow failed: " << ex.what());
    return -EINVAL;
  }

  int initialize (std::string const &xml)
  try
  {
    if (state::UNINITIALIZED != m_state)
    {
      MLOG(ERROR, "state mismatch: cannot initialize again in state: " << m_state);
      return -EPROTO;
    }

    reset_progress ("initialize");

    MLOG(INFO, "submitting INITIALIZE workflow");

    const std::string wf (read_workflow_from_file(m_wf_path_initialize));

    we::mgmt::type::activity_t act (wf);

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

    return submit_job(act.to_string(), job::type::INITIALIZE);
  }
  catch (std::exception const &ex)
  {
    MLOG(ERROR, "decoding workflow failed: " << ex.what());
    return -EINVAL;
  }

  int update_salt_mask (const char *data, size_t len)
  try
  {
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "cannot update salt mask currently, invalid state: " << m_state);
      return -EPROTO;
    }

    reset_progress ("update");

    MLOG(TRACE, "writing new salt mask");

    {
      fs::ofstream salt_mask_stream (m_file_with_mask);
      salt_mask_stream.write (data, len);
      salt_mask_stream.close();
    }

    MLOG (INFO, "submitting SALTMASK workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_mask));

    we::mgmt::type::activity_t act (wf);

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
    return submit_job(act.to_string(), job::type::UPDATE);
  }
  catch (std::exception const &ex)
  {
    MLOG(ERROR, "decoding workflow failed: " << ex.what());
    return -EINVAL;
  }

  int calculate(std::string const &xml)
  try
  {
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "cannot calculate right now, invalid state: " << m_state);
      return -EPROTO;
    }

    reset_progress ("migrate");

    MLOG(INFO, "submitting CALCULATE workflow");

    const std::string wf (read_workflow_from_file(m_wf_path_calculate));

    we::mgmt::type::activity_t act (wf);

    // place tokens
    try
    {
      we::util::token::put (act, "description", xml);
      we::util::token::put (act, "file_with_config", m_file_with_config);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not put token: " << ex.what());
      return -EINVAL;
    }

    int ec = submit_job(act.to_string(), job::type::CALCULATE);
    if (0 == ec)
    {
      m_state = state::CALCULATING;
    }
    return ec;
  }
  catch (std::exception const &ex)
  {
    MLOG(ERROR, "decoding workflow failed: " << ex.what());
    return -EINVAL;
  }

  int finalize()
  try
  {
    if (state::INITIALIZED != m_state)
    {
      MLOG(WARN, "finalizing from invalid state: " << m_state);
      MLOG(WARN, "this will probably result in failed executions");
    }

    reset_progress ("finalize");

    MLOG(INFO, "submitting FINALIZE workflow");

    const std::string wf(read_workflow_from_file(m_wf_path_finalize));

    we::mgmt::type::activity_t act (wf);

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

    int ec = submit_job(act.to_string(), job::type::FINALIZE);
    if (0 == ec)
    {
      m_state = state::FINALIZING;
    }
    return ec;
  }
  catch (std::exception const &ex)
  {
    MLOG(ERROR, "decoding workflow failed: " << ex.what());
    return -EINVAL;
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
      return 0;
    }
    else
    {
      MLOG(INFO, "nothing to cancel!");
      return -ESRCH;
    }
  }

  int stop ()
  {
    if (m_control_sdpa && sdpa_ctl)
    {
      MLOG(INFO, "stopping SDPA...");
      return sdpa_ctl->stop();
    }
    else
    {
      return 0;
    }
  }

  int open (std::string const & name)
  {
    int ec = 0;

    ec = reinitialize_gpi_state();
    if (0 != ec)
    {
      return ec;
    }

    gpi::pc::type::handle::descriptor_t found;

    ec = lookup_handle_by_name(name, found);
    if (ec < 0)
    {
      return ec;
    }

    if (0 == found.id)
    {
      return -ENOENT;
    }

    int fd = allocate_file_descriptor();
    if (fd >= 0)
    {
      stream_ptr_t s (new gpi_stream);
      s->fd = fd;
      s->handle = found;
      s->read_pointer = 0;
      s->write_pointer = 0;

      lock_type lock (m_stream_mutex);
      m_streams[fd] = s;
    }

    return fd;
  }

  int close (int fd)
  {
    lock_type lock (m_stream_mutex);
    stream_map_t::iterator stream_it (m_streams.find(fd));
    if (stream_it != m_streams.end())
    {
      // any subsequent call to read/write transfers will fail
      stream_it->second->handle.id = 0;

      free_file_descriptor(fd);

      m_streams.erase(stream_it);
      return 0;
    }
    else
    {
      return -EBADF;
    }
  }

  int seek (const int fd, const uint64_t off, const int whence, uint64_t * o)
  {
    lock_type lock (m_stream_mutex);
    stream_map_t::iterator stream_it (m_streams.find(fd));
    if (stream_it == m_streams.end())
      return -EBADF;

    stream_ptr_t s = stream_it->second;
    switch (whence)
    {
    case SEEK_SET:
      {
        if (off > s->handle.size)
          return -EINVAL;
        s->read_pointer = off;
        if (o)
          *o = s->read_pointer;
      }
      break;
    case SEEK_CUR:
      {
        if (s->read_pointer + off < s->read_pointer)
          return -EOVERFLOW;
        s->read_pointer += off;
        if (o)
          *o = s->read_pointer;
      }
      break;
    case SEEK_END:
      {
        s->read_pointer = s->handle.size;
        if (o)
          *o = s->read_pointer;
      }
      break;
    default:
      return -EINVAL;
    }
    return 0;
  }

  int read (int fd, char *buffer, size_t len, size_t & num_read)
  {
    stream_ptr_t s;

    {
      lock_type lock (m_stream_mutex);
      stream_map_t::iterator stream_it (m_streams.find(fd));
      if (stream_it == m_streams.end())
      {
        return -EBADF;
      }
      else
      {
        s = stream_it->second;
      }
    }

    return read_from_stream_to_buffer(s, buffer, len, num_read);
  }

  int write (int fd, const char *buffer, size_t len, size_t & num_written)
  {
    stream_ptr_t s;

    {
      lock_type lock (m_stream_mutex);
      stream_map_t::iterator stream_it (m_streams.find(fd));
      if (stream_it == m_streams.end())
      {
        return -EBADF;
      }
      else
      {
        s = stream_it->second;
      }
    }

    MLOG(ERROR, "write (fd, buffer, len) not yet implemented!");
    return -ENOSYS;
  }
private:
  void reset_progress (std::string const &phase)
  {
    progress->reset ("ufbmig", phase, 100);
    if (m_frontend)
      m_frontend->update_progress (0);
  }

  void update_progress ()
  {
    if (m_frontend)
    {
      size_t value; size_t max;
      if (0 == progress->get ("ufbmig", &value, &max))
      {
        if (value > max)
          value = max;

        int perc = (int)( (float)value / (float)max * 100.);

        m_frontend->update_progress(perc);
      }
    }
  }

  void update_progress (int v)
  {
    progress->set ("ufbmig", v);

    if (m_frontend)
      m_frontend->update_progress(v);
  }

  int allocate_file_descriptor ()
  {
    lock_type lock (m_stream_mutex);
    int fd;
    if (m_stream_fds.size())
    {
      fd = m_stream_fds.top();
      m_stream_fds.pop();
    }
    else
    {
      fd = (int)(m_stream_fds.size());
    }

    return fd;
  }

  void free_file_descriptor (int fd)
  {
    lock_type lock (m_stream_mutex);
    m_stream_fds.push(fd);
  }

  int lookup_handle_by_name( std::string const &name
                           , gpi::pc::type::handle::descriptor_t & found
                           )
  {
    // lookup handle
    gpi::pc::type::handle::list_t
      available_handles;
    try
    {
      available_handles = gpi_api->list_allocations(1);
    }
    catch (std::exception const &ex)
    {
      return -EIO;
    }

    BOOST_FOREACH( gpi::pc::type::handle::descriptor_t const & desc
                 , available_handles
                 )
    {
      if (desc.name == name)
      {
        found = desc;
        break;
      }
    }

    return 0;
  }

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

      if (m_job_list.size() == 1)
      {
        fhg_kernel()->schedule ( "update_job_states"
                               , boost::bind( &UfBMigBackImpl::update_job_states
                                            , this
                                            )
                               );
      }
    }

    return ec;
  }

  int handle_initialize_result (we::mgmt::type::activity_t const &result)
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

  int handle_update_salt_mask_result (we::mgmt::type::activity_t const &result)
  {
    return 0;
  }

  int handle_calculate_result (we::mgmt::type::activity_t const &result)
  {
    return 0;
  }

  int handle_finalize_result (we::mgmt::type::activity_t const &result)
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
       , "job '" << job::type_to_name(j.type) << "' returned: "
       << sdpa::status::show(j.state) << " [" << j.state << "]"
       << ": ec := " << j.error << " msg := " << j.error_message
       );

    update_progress(100);

    int ec (j.error);

    if (0 == ec)
    {
      try
      {
        we::mgmt::type::activity_t result (j.result);

        switch (j.type)
        {
        case job::type::PREPARE:
          break;

        case job::type::INITIALIZE:
          m_state = state::INITIALIZED;
          ec = handle_initialize_result(result);
          break;

        case job::type::UPDATE:
          ec = handle_update_salt_mask_result(result);
          m_state = state::INITIALIZED;
          break;

        case job::type::CALCULATE:
          ec = handle_calculate_result(result);
          break;

        case job::type::FINALIZE:
          ec = handle_finalize_result(result);
          break;

        default:
          return -EINVAL;
        }
      }
      catch (std::exception const &ex)
      {
        ec = -EINVAL;
      }
    }

    switch (j.type)
    {
    case job::type::PREPARE:
      if (m_frontend) m_frontend->prepare_backend_done (ec, j.error_message);
      break;

    case job::type::INITIALIZE:
      if (m_frontend) m_frontend->initialize_done(ec, j.error_message);
      break;

    case job::type::UPDATE:
      m_state = state::INITIALIZED;
      if (m_frontend) m_frontend->salt_mask_done(ec, j.error_message);
      break;

    case job::type::CALCULATE:
      m_state = state::INITIALIZED;
      if (m_frontend) m_frontend->calculate_done(ec, j.error_message);
      break;

    case job::type::FINALIZE:
      m_state = state::UNINITIALIZED;
      if (m_frontend) m_frontend->finalize_done(ec, j.error_message);
      break;

    default:
      return -EINVAL;
    }

    return ec;
  }

  void check_worker ()
  {
    MLOG (INFO, "checking worker status...");
    int ec = 0;
    ec += sdpa_ctl->status ("orch");
    ec += sdpa_ctl->status ("agg");
    ec += sdpa_ctl->status ("drts");
    ec += sdpa_ctl->status ("gpi");

    if (0 != ec)
    {
      MLOG (ERROR, "worker check failed: " << ec);
      if (m_frontend)
        m_frontend->send_logoutput ("worker check failed, terminating...");

      sdpa_ctl->stop();
    }
    else
    {
      fhg_kernel()->schedule ( "check_worker"
                             , boost::bind( &UfBMigBackImpl::check_worker
                                          , this
                                          )
                             , m_check_interval
                             );
    }
  }

  void update_job_states ()
  {
    lock_type lock (m_job_list_mutex);

    update_progress();

    for ( job_list_t::iterator j(m_job_list.begin())
        ; j != m_job_list.end()
        ; ++j
        )
    {
      j->state = sdpa_c->status(j->id, j->error, j->error_message);
      if (job::is_done(*j))
      {
        if (j->state == sdpa::status::CANCELED)
          j->error = -ECANCELED;
        if (j->state == sdpa::status::FINISHED)
          j->error = 0;
        if (j->state == sdpa::status::FAILED && j->error == 0)
          j->error = -EFAULT;
        if (j->state < 0)
        {
          j->error = j->state;
          j->state = sdpa::status::FAILED;
        }

        sdpa_c->result(j->id, j->result);
        sdpa_c->remove(j->id);
        handle_completed_job (*j);
        j = m_job_list.erase(j);
        if (j == m_job_list.end()) break;
      }
    }

    if (not m_job_list.empty())
    {
      fhg_kernel()->schedule ( "update_job_states"
                             , boost::bind( &UfBMigBackImpl::update_job_states
                                          , this
                                          )
                             );
    }
  }

  void clear_my_gpi_state ()
  {
    if (m_transfer_buffer)
    {
      try { gpi_api->free (m_transfer_buffer); } catch (...) {}
      m_transfer_buffer = 0;
    }
    if (m_transfer_segment)
    {
      try { gpi_api->unregister_segment (m_transfer_segment); } catch(...) {}
      m_transfer_segment = 0;
    }
  }

  int setup_my_gpi_state ()
  {
    m_transfer_segment = gpi_api->register_segment ( "ufbmigd"
                                                   , m_transfer_segment_size
                                                   , gpi::pc::F_FORCE_UNLINK
                                                   | gpi::pc::F_EXCLUSIVE
                                                   );
    m_transfer_buffer = gpi_api->alloc ( m_transfer_segment
                                        , m_chunk_size
                                        , "ufbmigd transfer buffer"
                                        , gpi::pc::F_EXCLUSIVE
                                        );
    return 0;
  }

  int reinitialize_gpi_state ()
  {
    lock_type gpi_mutex (m_gpi_state_mutex);

    if (! gpi_api)
    {
      MLOG(ERROR, "cannot initialize gpi connection: gpi plugin not available");
      return -EAGAIN;
    }

    if (! gpi_api->ping())
    {
      clear_my_gpi_state();

      m_state = state::UNINITIALIZED;
      if (m_frontend)
        m_frontend->finalize_done (0, "");

      if (! gpi_api->connect())
      {
        MLOG(ERROR, "could not open connection to gpi");
        return -EAGAIN;
      }
    }

    if (0 == m_transfer_buffer)
    {
      try
      {
        int ec = setup_my_gpi_state ();
        if (0 != ec)
        {
          MLOG(ERROR, "could not setup my gpi state: " << strerror(-ec));
          return ec;
        }
      }
      catch (std::exception const &ex)
      {
        MLOG(ERROR, "could not setup my gpi state: " << ex.what());
        return -EAGAIN;
      }
    }

    return 0;
  }

  int read_from_stream_to_buffer ( stream_ptr_t s
                                 , char * buffer
                                 , size_t len
                                 , size_t & num_read
                                 )
  {
    const gpi::pc::type::queue_id_t queue (0);

    int ec = reinitialize_gpi_state();
    if (0 != ec)
    {
      return ec;
    }

    size_t remaining_bytes = len;

    num_read = 0;
    while (remaining_bytes && (s->read_pointer < s->handle.size))
    {
      size_t transfer_size =
        std::min ( m_chunk_size
                 , std::min ( remaining_bytes
                            , s->handle.size - s->read_pointer
                            )
                 );

      // transfer chunk from handle to scratch
      try
      {
        gpi_api->wait
          (gpi_api->memcpy( gpi::pc::type::memory_location_t( m_transfer_buffer
                                                            , 0
                                                            )
                          , gpi::pc::type::memory_location_t( s->handle.id
                                                            , s->read_pointer
                                                            )
                          , transfer_size
                          , queue
                          )
        );
      }
      catch (std::exception const & ex)
      {
        MLOG(WARN, "could not transfer from " << s->handle << " to scratch: " << ex.what());
        return -EIO;
      }

      void *src = gpi_api->ptr (m_transfer_buffer);
      if (src)
      {
        // memcpy to buffer
        memcpy ( buffer
               , src
               , transfer_size
               );
      }
      else
      {
        MLOG(WARN, "could not memcpy to buffer: shm disappeared");
        return -EIO;
      }

      // update state
      remaining_bytes -= transfer_size;
      s->read_pointer += transfer_size;
      num_read += transfer_size;
      std::advance (buffer, transfer_size);
    }

    return 0;
  }

  mutable mutex_type m_job_list_mutex;
  job_list_t m_job_list;

  mutable mutex_type m_stream_mutex;
  stream_map_t m_streams;
  stream_fd_stack_t m_stream_fds;

  mutable mutex_type m_gpi_state_mutex;

  bool m_control_sdpa;
  sdpa::Control * sdpa_ctl;
  sdpa::Client * sdpa_c;
  progress::Progress *progress;
  gpi::GPI *gpi_api;
  ufbmig::Frontend *m_frontend;
  std::list<job::info_t> m_job_info_list;

  // state
  int m_state;

  std::size_t             m_chunk_size;
  std::size_t             m_check_interval;

  // shared memory
  gpi::pc::type::segment_id_t m_transfer_segment;
  std::size_t                 m_transfer_segment_size;
  gpi::pc::type::handle_t     m_transfer_buffer;

  // workflow paths
  std::string m_wf_path_prepare;
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
                 , ""
                 , "provides the backend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "sdpactl,sdpac,progress,gpi"
                 , ""
                 );
