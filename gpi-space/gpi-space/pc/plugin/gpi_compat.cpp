// this is a compatibility plugin for the new plugin architecture

#include <fhglog/minimal.hpp>

#include <unistd.h> // usleep
#include <fvm-pc/pc.hpp>
#include "gpi.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/assert.hpp>
#include <fhg/plugin/plugin.hpp>

class GPICompatPluginImpl;
static GPICompatPluginImpl * gpi_compat = 0;

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
      m_scr_size = boost::lexical_cast<fvmSize_t>
        (fhg_kernel()->get<std::size_t>("com_size", 16U * (1<<20)));
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not parse plugin.gpi_compat.scratch_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_initialize_retry_interval =
        fhg_kernel()->get<useconds_t>("initialize_retry_interval", "500");
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
    m_segment_name = "fvm-pc-" + my_pid;
    m_segment_handle_name = "fvm-pc-segment-" + my_pid;
    m_global_handle_name = "fvm-pc-global-" + my_pid;
    m_local_handle_name = "fvm-pc-local-" + my_pid;
    m_scratch_handle_name = "fvm-pc-com-" + my_pid;

    api = fhg_kernel()->acquire<gpi::GPI>("gpi");
    if (reinitialize_gpi_state() < 0)
    {
      LOG(WARN, "gpi plugin is not yet available, state initialization deferred!");
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    try
    {
      if (m_scr_hdl)
        api->free(m_scr_hdl);
    }
    catch (std::exception const &ex)
    {
      LOG(WARN, "gpi_compat plugin could not clean up scratch handle");
    }
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
    lock_type gpi_mutex (m_gpi_state_mutex);

    if (! api->ping())
    {
      clear_my_gpi_state();

      if (! api->connect())
      {
        return -EAGAIN;
      }
    }

    if (0 == m_scr_hdl)
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

  int ensure_gpi_state ()
  {
    int ec;

    do
    {
      ec = reinitialize_gpi_state ();
      if (ec == -EAGAIN)
      {
        usleep (m_initialize_retry_interval);
      }
    }
    while (ec == -EAGAIN);

    return ec;
  }
private:
  int setup_my_gpi_state ()
  {
    api->garbage_collect();

    gpi_info = api->collect_info();

    if (0 == m_shm_size)
    {
      return 0;
    }

    // register segment
    m_shm_id = api->register_segment ( m_segment_name
                                     , m_shm_size
                                     , gpi::pc::type::segment::F_EXCLUSIVE
                                     | gpi::pc::type::segment::F_FORCE_UNLINK
                                     // , gpi::pc::type::segment::F_FORCE_UNLINK
                                     );
    m_scr_hdl = api->alloc ( 1 // GPI
                           , m_scr_size
                           , m_scratch_handle_name
                           , 0
                           );
    m_shm_hdl = api->alloc ( m_shm_id
                           , m_shm_size
                           , m_segment_handle_name
                           , gpi::pc::type::handle::F_EXCLUSIVE
                           );
    m_shm_ptr = api->ptr(m_shm_hdl);


    LOG(INFO, "successfully initialized gpi state");

    return 0;
  }

  void clear_my_gpi_state ()
  {
    m_scr_hdl = 0;
    m_shm_hdl = 0;
    m_shm_ptr = 0;
    m_shm_id  = 0;
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
  fvmSize_t                          m_scr_size;
  gpi::pc::type::handle_t            m_scr_hdl;

private:
  useconds_t                         m_initialize_retry_interval;
};

int fvmConnect()
{
  return 0;
}

int fvmLeave()
{
  return 0;
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size, const char *name)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }

  return gpi_compat->api->alloc ( 1 // GPI
                                , size
                                , name
                                , gpi::pc::type::handle::F_GLOBAL
                                | gpi::pc::type::handle::F_PERSISTENT
                                );
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  return fvmGlobalAlloc(size, gpi_compat->m_global_handle_name.c_str());
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  gpi_compat->api->free(ptr);
  return 0;
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char *name)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
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
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }

  gpi_compat->api->free (ptr);
  return 0;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }

  static const gpi::pc::type::queue_id_t queue = 0;

  fhg_assert (0 != gpi_compat->m_scr_size);
  fhg_assert (0 != gpi_compat->m_scr_hdl);
  fhg_assert (0 != gpi_compat->m_shm_hdl);

  gpi::pc::type::size_t chunk_size (gpi_compat->m_scr_size);
  gpi::pc::type::size_t remaining (size);

  DMLOG_IF( TRACE
          , chunk_size < remaining
          , "internal communication buffer is too small, need to split 'get' up: "
          << "requested := " << size << " "
          << "com-buffer := " << chunk_size
          );

  gpi::pc::type::size_t src_offset(fvmOffset);
  gpi::pc::type::size_t dst_offset(shmemOffset);

  bool in_progress (false);

  while (remaining > 0)
  {
    gpi::pc::type::size_t transfer_size (std::min(remaining, chunk_size));

    if (in_progress)
      gpi_compat->api->wait(queue);

    DLOG(INFO, "transfer from gpi to scratch");

    // 1. transfer memory to scratch
    gpi_compat->api->wait
      (gpi_compat->api->memcpy( gpi::pc::type::memory_location_t( gpi_compat->m_scr_hdl
                                                                , 0
                                                                )
                              , gpi::pc::type::memory_location_t(handle, src_offset)
                              , transfer_size
                              , queue
                              )
      );

    DLOG(INFO, "transfer from scratch to shm");

    // 2. transfer from scratch to shm
    gpi_compat->api->memcpy( gpi::pc::type::memory_location_t( gpi_compat->m_shm_hdl
                                                             , dst_offset
                                                             )
                           , gpi::pc::type::memory_location_t(gpi_compat->m_scr_hdl, 0)
                           , transfer_size
                           , queue
                           );

    in_progress = true;
    remaining -= transfer_size;
    src_offset += transfer_size;
    dst_offset += transfer_size;
  }

  return queue;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  static const gpi::pc::type::queue_id_t queue = 1;

  fhg_assert (0 != gpi_compat->m_scr_size);
  fhg_assert (0 != gpi_compat->m_scr_hdl);
  fhg_assert (0 != gpi_compat->m_shm_hdl);

  gpi::pc::type::size_t chunk_size (gpi_compat->m_scr_size);
  gpi::pc::type::size_t remaining (size);

  DMLOG_IF( TRACE
          , chunk_size < remaining
          , "internal communication buffer is too small, need to split 'get' up: "
          << "requested := " << size << " "
          << "com-buffer := " << chunk_size
          );

  gpi::pc::type::size_t src_offset(shmemOffset);
  gpi::pc::type::size_t dst_offset(fvmOffset);

  bool in_progress (false);

  while (remaining > 0)
  {
    gpi::pc::type::size_t transfer_size (std::min(remaining, chunk_size));

    if (in_progress)
      gpi_compat->api->wait(queue);

    DLOG(INFO, "transfer from shm to scratch");

    // 1. transfer memory from shm to scratch
    gpi_compat->api->wait
      (gpi_compat->api->memcpy( gpi::pc::type::memory_location_t( gpi_compat->m_scr_hdl
                                                                , 0
                                                                )
                              , gpi::pc::type::memory_location_t( gpi_compat->m_shm_hdl
                                                                , src_offset
                                                                )
                              , transfer_size
                              , queue
                              )
      );

    DLOG(INFO, "transfer from scratch to gpi");

    // 2. transfer memory from scratch to global
    gpi_compat->api->memcpy( gpi::pc::type::memory_location_t(handle, dst_offset)
                           , gpi::pc::type::memory_location_t(gpi_compat->m_scr_hdl, 0)
                           , transfer_size
                           , queue
                           );

    in_progress = true;
    remaining  -= transfer_size;
    src_offset += transfer_size;
    dst_offset += transfer_size;
  }

  return queue;
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  static const gpi::pc::type::queue_id_t queue = 2;

  fhg_assert (0 != gpi_compat->m_scr_size);
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
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  static const gpi::pc::type::queue_id_t queue = 3;

  fhg_assert (0 != gpi_compat->m_scr_size);
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
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  try
  {
    gpi::pc::type::size_t num_finished
      (gpi_compat->api->wait(static_cast<gpi::pc::type::queue_id_t>(handle)));
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
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  return gpi_compat->m_shm_ptr;
}

fvmSize_t fvmGetShmemSize()
{
  return gpi_compat->m_shm_size;
}

int fvmGetRank()
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
  return gpi_compat->gpi_info.rank;
}

int fvmGetNodeCount()
{
  int ec = gpi_compat->ensure_gpi_state();
  if (ec < 0)
  {
    throw std::runtime_error(std::string("Could not initialize GPI state: ") + strerror (-ec));
  }
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
