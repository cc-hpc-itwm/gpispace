// this is a compatibility plugin for the new plugin architecture

#include <fhglog/minimal.hpp>

#include <unistd.h> // usleep
#include <fvm-pc/pc.hpp>
#include "gpi.hpp"
#include <gpi-space/pc/type/flags.hpp>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/assert.hpp>
#include <fhg/plugin/plugin.hpp>

class GPICompatPluginImpl;
static GPICompatPluginImpl * gpi_compat = 0;

enum gpi_state_t
  {
    ST_DISCONNECTED
  , ST_CONNECTED
  };

class GPICompatPluginImpl : FHG_PLUGIN
{
  typedef boost::unordered_map< gpi::pc::type::handle_t
                              , gpi::pc::type::handle::descriptor_t
                              > handle_cache_t;
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
public:
  FHG_PLUGIN_START()
  {
    gpi_compat = this;

    m_gpi_state = ST_DISCONNECTED;
    m_shm_hdl = 0;
    m_shm_ptr = (void*)0;
    m_shm_id = 0;

    try
    {
      m_shm_size = boost::lexical_cast<fvmSize_t>
        (fhg_kernel()->get<std::size_t>("shm_size", 128U * (1<<20)));
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not parse plugin.gpi_compat.shm_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_initialize_retry_interval =
        fhg_kernel()->get<useconds_t>("initialize_retry_interval", "2000000");
    }
    catch (std::exception const &ex)
    {
      LOG( WARN
         , "could not parse plugin.gpi_compat.initialize_retry_interval: "
         << ex.what()
         );
      m_initialize_retry_interval = 500;
    }

    const std::string my_pid(boost::lexical_cast<std::string>(getpid()));
    const std::string name_prefix (fhg_kernel()->get_name () + "-" + my_pid);
    m_segment_name = name_prefix + "-shm";
    m_segment_handle_name = name_prefix + "-shm";
    m_global_handle_name = name_prefix + "-global";
    m_local_handle_name = name_prefix + "-local";
    m_scratch_handle_name = name_prefix + "-com";

    api = fhg_kernel()->acquire<gpi::GPI>("gpi");

    schedule_reinitialize_gpi ();

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    try
    {
      if (m_shm_hdl)
        api->free(m_shm_hdl);
    }
    catch (std::exception const &ex)
    {
      LOG(WARN, "gpi_compat plugin could not clean up shm handle");
    }
    try
    {
      if (m_shm_id)
        api->unregister_segment(m_shm_id);
    }
    catch (std::exception const &ex)
    {
      LOG(WARN, "gpi_compat plugin could not unregister segment");
    }

    m_shm_ptr = 0;
    api = 0;
    gpi_compat = 0;

    FHG_PLUGIN_STOPPED();
  }

  int reinitialize_gpi_state ()
  {
    lock_type lock (m_gpi_state_mutex);

    if (! api->ping())
    {
      clear_my_gpi_state();

      if (! api->connect())
      {
        return -EAGAIN;
      }
    }

    if (ST_DISCONNECTED == m_gpi_state)
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
        return -EFAULT;
      }
    }

    return 0;
  }

  void schedule_reinitialize_gpi ()
  {
    int ec = reinitialize_gpi_state ();
    if (ec == -EAGAIN)
    {
      MLOG ( WARN
                , "gpi plugin is not yet available, state initialization deferred!"
                );

      fhg_kernel()->schedule( "gpi_compat.setup"
                            , boost::bind ( &GPICompatPluginImpl::schedule_reinitialize_gpi
                                          , this
                                          )
                            , 2
                            );
    }
  }

  int ensure_gpi_state ()
  {
    int ec;

    do
    {
      ec = reinitialize_gpi_state ();
      if (ec == -EAGAIN)
      {
        if (m_was_connected)
        {
          ec = -ECONNRESET;
          break;
        }
        else
        {
          usleep (m_initialize_retry_interval);
        }
      }
    }
    while (ec == -EAGAIN);

    if ( (ec == 0) && (m_shm_size > 0))
    {
      if (m_shm_hdl == (gpi::pc::type::handle_t)0)
        MLOG (ERROR, "gpi state setup but shm_hdl == 0");
      if (m_shm_ptr == 0)
        MLOG (ERROR, "gpi state setup but shm_ptr == 0");
    }

    return ec;
  }
private:
  int setup_my_gpi_state ()
  {
    api->garbage_collect();

    gpi_info = api->collect_info();

    if (m_shm_size)
    {
      // register segment
      m_shm_id = api->register_segment ( m_segment_name
                                       , m_shm_size
                                       , gpi::pc::F_EXCLUSIVE
                                       | gpi::pc::F_FORCE_UNLINK
                                       );

      m_shm_hdl = api->alloc ( m_shm_id
                             , m_shm_size
                             , m_segment_handle_name
                             , gpi::pc::F_EXCLUSIVE
                             );
      m_shm_ptr = api->ptr (m_shm_hdl);
    }

    LOG(INFO, "successfully initialized gpi state");

    m_was_connected = true;
    m_gpi_state = ST_CONNECTED;

    return 0;
  }

  void clear_my_gpi_state ()
  {
    if (m_gpi_state == ST_CONNECTED)
    {
      if (api->ping ())
      {
        try { api->free (m_shm_hdl); } catch (...) {}
        try { api->unregister_segment (m_shm_id); } catch (...) {}
      }
    }

    m_shm_hdl = 0;
    m_shm_ptr = 0;
    m_shm_id  = 0;

    m_gpi_state = ST_DISCONNECTED;
  }
public:
  mutable mutex_type                 m_handle_cache_mtx;
  handle_cache_t                     m_handle_cache;

  mutable mutex_type                 m_gpi_state_mutex;

  std::string                        m_segment_name;
  std::string                        m_segment_handle_name;
  std::string                        m_global_handle_name;
  std::string                        m_local_handle_name;
  std::string                        m_scratch_handle_name;

  gpi::GPI                          *api;
  gpi::pc::type::info::descriptor_t  gpi_info;
  gpi::pc::type::segment_id_t        m_shm_id;
  void                              *m_shm_ptr;
  fvmSize_t                          m_shm_size;
  gpi::pc::type::handle_t            m_shm_hdl;
  gpi_state_t                        m_gpi_state;
private:
  useconds_t                         m_initialize_retry_interval;
  bool                               m_was_connected;
};

int fvmConnect()
{
  return 0;
}

int fvmLeave()
{
  return 0;
}

static void require_gpi_state (std::string const & fun_name)
{
  int ec = gpi_compat->ensure_gpi_state ();
  if (ec < 0)
  {
    throw std::runtime_error
      (fun_name + ": could not ensure gpi state: " + strerror (-ec));
  }
}

fvmAllocHandle_t fvmGlobalAllocExact (fvmSize_t size, const char *name)
{
  require_gpi_state ("fvmGlobalAlloc");

  return gpi_compat->api->alloc ( 1 // GPI
                                , size
                                , name
                                , gpi::pc::F_GLOBAL
                                | gpi::pc::F_PERSISTENT
                                );
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size, const char *name)
{
  return fvmGlobalAllocExact ( size * gpi_compat->gpi_info.nodes
                             , name
                             );
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  return fvmGlobalAlloc(size, gpi_compat->m_global_handle_name.c_str());
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
  require_gpi_state ("fvmGlobalFree");

  gpi_compat->api->free(ptr);
  return 0;
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char *name)
{
  require_gpi_state ("fvmLocalAlloc");

  return gpi_compat->api->alloc ( 1 // GPI
                                , size
                                , name
                                , 0
                                );
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
{
  return fvmLocalAlloc(size, gpi_compat->m_local_handle_name.c_str());
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{
  require_gpi_state ("fvmLocalFree");

  gpi_compat->api->free (ptr);
  return 0;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t)
{
  require_gpi_state ("fvmGetGlobalData");

  static const gpi::pc::type::queue_id_t queue = GPI_PC_INVAL;

  fhg_assert (0 != gpi_compat->m_shm_hdl);

  return
    gpi_compat->api->memcpy( gpi::pc::type::memory_location_t ( gpi_compat->m_shm_hdl
                                                              , shmemOffset
                                                              )
                           , gpi::pc::type::memory_location_t (handle, fvmOffset)
                           , size
                           , queue
                           );
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t)
{
  require_gpi_state ("fvmPutGlobalData");

  static const gpi::pc::type::queue_id_t queue = GPI_PC_INVAL;

  fhg_assert (0 != gpi_compat->m_shm_hdl);

  return
    gpi_compat->api->memcpy( gpi::pc::type::memory_location_t (handle, fvmOffset)
                           , gpi::pc::type::memory_location_t ( gpi_compat->m_shm_hdl
                                                              , shmemOffset
                                                              )
                           , size
                           , queue
                           );
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  require_gpi_state ("fvmPutLocalData");

  static const gpi::pc::type::queue_id_t queue = GPI_PC_INVAL;

  fhg_assert (0 != gpi_compat->m_shm_hdl);

  return gpi_compat->api->
    memcpy( gpi::pc::type::memory_location_t(handle, fvmOffset)
          , gpi::pc::type::memory_location_t(gpi_compat->m_shm_hdl, shmemOffset)
          , size
          , queue
          );
}

fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  require_gpi_state ("fvmGetLocalData");

  static const gpi::pc::type::queue_id_t queue = GPI_PC_INVAL;

  fhg_assert (0 != gpi_compat->m_shm_hdl);

  return gpi_compat->api->
    memcpy( gpi::pc::type::memory_location_t(gpi_compat->m_shm_hdl, shmemOffset)
          , gpi::pc::type::memory_location_t(handle, fvmOffset)
          , size
          , queue
          );
}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  require_gpi_state ("waitComm");

  try
  {
    gpi_compat->api->wait(static_cast<gpi::pc::type::queue_id_t>(handle));
    return COMM_HANDLE_OK;
  }
  catch (std::exception const &ex)
  {
    LOG(ERROR, "communication on queue " << handle << " failed: " << ex.what());
    return COMM_HANDLE_ERROR;
  }
}

const char *fvmGetShmemName()
{
  return gpi_compat->m_segment_name.c_str();
}

void *fvmGetShmemPtr()
{
  require_gpi_state ("fvmGetShmemPtr");

  return gpi_compat->m_shm_ptr;
}

fvmSize_t fvmGetShmemSize()
{
  return gpi_compat->m_shm_size;
}

int fvmGetRank()
{
  require_gpi_state ("fvmGetRank");

  return gpi_compat->gpi_info.rank;
}

int fvmGetNodeCount()
{
  require_gpi_state ("fvmGetNodeCount");

  return gpi_compat->gpi_info.nodes;
}

EXPORT_FHG_PLUGIN( gpi_compat
                 , GPICompatPluginImpl
                 , ""
                 , "Plugin to access the gpi-space (compatibility)"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "gpi"
                 , ""
                 );
