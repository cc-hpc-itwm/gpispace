// this is a compatibility plugin for the new plugin architecture

#include <fhglog/minimal.hpp>

#include <fvm-pc/pc.hpp>
#include "gpi.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

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
    try
    {
      m_shm_size = boost::lexical_cast<fvmSize_t>
        (fhg_kernel()->get("shm_size", "536870912"));
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not parse plugin.gpi-compat.shm_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_scr_size = boost::lexical_cast<fvmSize_t>
        (fhg_kernel()->get("com_size", "16777216")); // 16MB
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not parse plugin.gpi-compat.scratch_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    const std::string my_pid(boost::lexical_cast<std::string>(getpid()));
    m_segment_name = "fvm-pc-" + my_pid;
    m_segment_handle_name = "fvm-pc-segment-" + my_pid;
    m_global_handle_name = "fvm-pc-global-" + my_pid;
    m_local_handle_name = "fvm-pc-local-" + my_pid;
    m_scratch_handle_name = "fvm-pc-com-" + my_pid;

    if (! try_start())
    {
      LOG(WARN, "gpi-compat plugin could not be started, gpi plugin is not (yet) available");
      LOG(WARN, "There be dragons! (Segfaults are imminent if you execute gpi modules!)");
      FHG_PLUGIN_INCOMPLETE();
    }
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    if (api)
    {
      api->free(m_scr_hdl);
      api->free(m_shm_hdl);
      api->unregister_segment(m_shm_id);
      m_shm_ptr = 0;
      api = 0;
      gpi_compat = 0;
    }
    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if ("gpi" == plugin)
    {
      if (try_start())
      {
        fhg_kernel()->start_completed(0);
      }
      else
      {
        fhg_kernel()->start_completed(EINVAL);
      }
    }
  }

  gpi::pc::type::handle::descriptor_t
  get_handle_info (gpi::pc::type::handle_t h)
  {
    lock_type lock(m_handle_cache_mtx);

    handle_cache_t::iterator info (m_handle_cache.find(h));
    if (info == m_handle_cache.end())
    {
      gpi::pc::type::handle::list_t handles (api->list_allocations (1));
      for ( gpi::pc::type::handle::list_t::const_iterator it(handles.begin())
          ; it != handles.end()
          ; ++it
          )
      {
        if (it->id == h)
        {
          while (m_handle_cache.size() >= 1024)
          {
            m_handle_cache.erase(m_handle_cache.begin());
          }

          info = m_handle_cache.insert(std::make_pair(h, *it)).first;
          break;
        }
      }
    }

    if (info == m_handle_cache.end())
    {
      LOG(ERROR, "could not get handle information for handle " << h << ": no such handle");
      return gpi::pc::type::handle::descriptor_t();
    }
    else
    {
      return info->second;
    }
  }

private:
  bool try_start ()
  {
    try
    {
      api = fhg_kernel()->acquire<gpi::GPI>("gpi");
      if (0 == api)
      {
        return false;
      }

      gpi_info = api->collect_info();

      // register segment
      m_shm_id = api->register_segment ( m_segment_name
                                       , m_shm_size
                                       // , gpi::pc::type::segment::F_EXCLUSIVE
                                       // | gpi::pc::type::segment::F_FORCE_UNLINK
                                       , gpi::pc::type::segment::F_FORCE_UNLINK
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

      gpi_compat = this;

      return true;
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not initialize gpi structures: " << ex.what());
      return false;
    }
  }

public:
  mutable mutex_type                 m_handle_cache_mtx;
  handle_cache_t                     m_handle_cache;
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
  gpi_compat->api->free(ptr);
  return 0;
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char *name)
{
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
  gpi_compat->api->free (ptr);
  return 0;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t)
{
  static const gpi::pc::type::queue_id_t queue = 0;

  gpi::pc::type::size_t chunk_size (gpi_compat->m_scr_size);
  gpi::pc::type::size_t remaining (size);

  LOG_IF( INFO
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
  static const gpi::pc::type::queue_id_t queue = 1;

  gpi::pc::type::size_t chunk_size (gpi_compat->m_scr_size);
  gpi::pc::type::size_t remaining (size);

  LOG_IF( INFO
        , chunk_size < remaining
        , "internal communication buffer is too small, need to split 'put' up: "
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
  static const gpi::pc::type::queue_id_t queue = 2;
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
  static const gpi::pc::type::queue_id_t queue = 3;
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
  return gpi_compat->m_shm_ptr;
}

fvmSize_t fvmGetShmemSize()
{
  return gpi_compat->m_shm_size;
}

int fvmGetRank()
{
  return gpi_compat->gpi_info.rank;
}

int fvmGetNodeCount()
{
  return gpi_compat->gpi_info.nodes;
}

EXPORT_FHG_PLUGIN( gpi-compat
                 , GPICompatPluginImpl
                 , "Plugin to access the gpi-space (compatibility)"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
