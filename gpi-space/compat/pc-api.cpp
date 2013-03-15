#include <fvm-pc/pc.hpp>

#define LOG_COMPONENT "fvm-pc-compat"
#include <fhglog/minimal.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <we/loader/macros.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/config/parser.hpp>
#include <gpi-space/pc/client/api.hpp>

#include <boost/thread/mutex.hpp>

typedef boost::mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;

static gpi::pc::client::api_t & gpi_api ()
{
  static gpi::pc::client::api_t a;
  return a;
}

static gpi::pc::type::info::descriptor_t gpi_info;
static void *shm_ptr = 0;
static fvmSize_t shm_size = 0;
static gpi::pc::type::segment_id_t shm_id = 0;
static gpi::pc::type::handle_t     shm_hdl = 0;

int fvmConnect()
{
  throw std::runtime_error ("fvmConnect not implemented anymore");
}

int fvmLeave()
{
  shm_ptr = 0;
  shm_size = 0;
  shm_id = 0;
  shm_hdl = 0;
  gpi_api().stop();
  return 0;
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  return gpi_api().alloc ( 1 // GPI
                         , size
                         , "fvm-pc-compat-global-no-name"
                         , gpi::pc::F_GLOBAL
                         | gpi::pc::F_PERSISTENT
                         );
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
  gpi_api().free (ptr);
  return 0;
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
{
  return gpi_api().alloc ( 1 // GPI
                         , size
                         , "fvm-pc-compat-local-no-name"
                         , 0
                         );
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{
  gpi_api().free (ptr);
  return 0;
}

static
gpi::pc::type::handle::descriptor_t
get_handle_info (gpi::pc::type::handle_t h)
{
  typedef boost::unordered_map< gpi::pc::type::handle_t
                              , gpi::pc::type::handle::descriptor_t
                              > handle_cache_t;
  static handle_cache_t handle_cache;
  static mutex_type mutex;

  lock_type lock(mutex);

  handle_cache_t::iterator info (handle_cache.find(h));
  if (info == handle_cache.end())
  {
    gpi::pc::type::handle::list_t handles (gpi_api().list_allocations (1));
    for ( gpi::pc::type::handle::list_t::const_iterator it(handles.begin())
        ; it != handles.end()
        ; ++it
        )
    {
      if (it->id == h)
      {
        while (handle_cache.size() >= 1024)
        {
          handle_cache.erase(handle_cache.begin());
        }

        info = handle_cache.insert(std::make_pair(h, *it)).first;
        break;
      }
    }
  }

  if (info == handle_cache.end())
  {
    LOG(ERROR, "cannot get handle information for handle " << h << ": no such handle");
    throw std::runtime_error("scratch handle is invalid!");
  }

  return info->second;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t scratch)
{
  gpi::pc::type::handle::descriptor_t
    hdl_info (get_handle_info(scratch));
  if (hdl_info.segment != 1)
  {
    throw std::runtime_error ("STRANGE!!! scratch handle is not a gpi allocation");
  }

  gpi::pc::type::size_t base (0);
  if (hdl_info.flags & gpi::pc::F_GLOBAL)
  {
    base = hdl_info.size * gpi_info.rank;
  }

  gpi::pc::type::size_t chunk_size (hdl_info.size);
  gpi::pc::type::size_t remaining (size);

  gpi::pc::type::size_t src_offset(fvmOffset);
  gpi::pc::type::size_t dst_offset(shmemOffset);

  bool in_progress (false);
  const gpi::pc::type::queue_id_t queue (0);

  while (remaining > 0)
  {
    gpi::pc::type::size_t transfer_size (std::min(remaining, chunk_size));

    if (in_progress)
      gpi_api().wait(queue);

    DLOG(INFO, "transfer from gpi to scratch");

    // 1. transfer memory to scratch
    gpi_api().wait
      (gpi_api().memcpy( gpi::pc::type::memory_location_t(scratch, base)
                       , gpi::pc::type::memory_location_t(handle, src_offset)
                       , transfer_size
                       , queue
                       )
      );

    DLOG(INFO, "transfer from scratch to shm");

    // 2. transfer from scratch to shm
    gpi_api().memcpy( gpi::pc::type::memory_location_t(shm_hdl, dst_offset)
                    , gpi::pc::type::memory_location_t(scratch, base)
                    , transfer_size
                    , queue
                    );

    in_progress = true;
    remaining -= transfer_size;
    src_offset += transfer_size;
    dst_offset += transfer_size;
  }

  /*
  if (in_progress)
    gpi_api().wait(queue);
  */

  return queue;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t scratch)
{
  gpi::pc::type::handle::descriptor_t
    hdl_info (get_handle_info(scratch));
  if (hdl_info.segment != 1)
  {
    throw std::runtime_error ("STRANGE!!! scratch handle is not a gpi allocation");
  }

  gpi::pc::type::size_t base (0);
  if (hdl_info.flags & gpi::pc::F_GLOBAL)
  {
    base = hdl_info.size * gpi_info.rank;
  }

  gpi::pc::type::size_t chunk_size (hdl_info.size);
  gpi::pc::type::size_t remaining (size);

  gpi::pc::type::size_t src_offset(shmemOffset);
  gpi::pc::type::size_t dst_offset(fvmOffset);


  bool in_progress (false);
  const gpi::pc::type::queue_id_t queue (0);

  while (remaining > 0)
  {
    gpi::pc::type::size_t transfer_size (std::min(remaining, chunk_size));

    if (in_progress) gpi_api().wait(queue);

    DLOG(INFO, "transfer from shm to scratch");

    // 1. transfer memory from shm to scratch
    gpi_api().wait
      (gpi_api().memcpy( gpi::pc::type::memory_location_t(scratch, base)
                       , gpi::pc::type::memory_location_t(shm_hdl, src_offset)
                       , transfer_size
                       , queue
                       )
      );

    DLOG(INFO, "transfer from scratch to gpi");

    // 2. transfer memory from scratch to global
    gpi_api().memcpy( gpi::pc::type::memory_location_t(handle, dst_offset)
                    , gpi::pc::type::memory_location_t(scratch, base)
                    , transfer_size
                    , queue
                    );

    in_progress = true;
    remaining  -= transfer_size;
    src_offset += transfer_size;
    dst_offset += transfer_size;
  }

  /*
  if (in_progress) gpi_api().wait(queue);
  */

  return queue;
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  return gpi_api().memcpy( gpi::pc::type::memory_location_t(handle, fvmOffset)
                         , gpi::pc::type::memory_location_t(shm_hdl, shmemOffset)
                         , size
                         , 0
                         );
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  return gpi_api().memcpy( gpi::pc::type::memory_location_t(shm_hdl, shmemOffset)
                         , gpi::pc::type::memory_location_t(handle, fvmOffset)
                         , size
                         , 0
                         );
}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  gpi_api().wait (static_cast<gpi::pc::type::queue_id_t>(handle));
  return COMM_HANDLE_OK;
}

void *fvmGetShmemPtr()
{
  return shm_ptr;
}

fvmSize_t fvmGetShmemSize()
{
  return shm_size;
}

int fvmGetRank()
{
  return gpi_info.rank;
}

int fvmGetNodeCount()
{
  return gpi_info.nodes;
}

WE_MOD_INITIALIZE_START (fvm);
{
  namespace fs = boost::filesystem;

  gpi_space::parser::config_parser_t cfg_parser;
  fs::path config_file
    (std::string(getenv("HOME")) + "/.sdpa/configs/sdpa.rc");

  try
  {
    gpi_space::parser::parse (config_file.string(), boost::ref(cfg_parser));
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "could not parse config file " << config_file << ": " << ex.what());
    throw;
  }

  // parse config files
  //     gpi.rc -> path to socket
  //     pc.rc  -> memory size?
  shm_size =
    boost::lexical_cast<fvmSize_t>(cfg_parser.get( "api.compat.shm_size"
                                                 , "536870912"
                                                 )
                                  );

  int trials(10);
  while (trials --> 0)
  {
    namespace fs = boost::filesystem;
    fs::path socket_path (cfg_parser.get("gpi.socket_path", "/var/tmp"));
    socket_path /=
      ("S-gpi-space." + boost::lexical_cast<std::string>(getuid()) + ".0");

    gpi_api().path (socket_path.string());

    CLOG( INFO
        , LOG_COMPONENT
        , "initializing fvm-pc-compat:"
        << " shm size " << shm_size
        << " socket " << gpi_api().path()
        );

    try
    {
      gpi_api().start ();
      break;
    }
    catch (std::exception const & ex)
    {
      if (trials)
      {
        usleep(1 * 1000 * 1000);
        LOG(WARN, "connection to gpi-space failed, retrying");
      }
      else
      {
        LOG(ERROR, "could not connect to gpi-space: " << ex.what());
        throw;
      }
    }
  }

  gpi_info = gpi_api().collect_info();

  // register segment

  shm_id = gpi_api().register_segment ( "fvm-pc-compat"
                                      , shm_size
//                                       , gpi::pc::F_EXCLUSIVE
//                                       | gpi::pc::F_FORCE_UNLINK
                                      , gpi::pc::F_FORCE_UNLINK
                                      );
  shm_hdl = gpi_api().alloc ( shm_id
                            , shm_size
                            , "fvm-pc-compat"
                            , gpi::pc::F_EXCLUSIVE
                            );
  shm_ptr = gpi_api().segments()[shm_id]->ptr();

  CLOG( INFO
      , LOG_COMPONENT
      , "connected!"
      );
}
WE_MOD_INITIALIZE_END (fvm);

WE_MOD_FINALIZE_START (fvm);
{
  gpi_api().stop ();
}
WE_MOD_FINALIZE_END (fvm);
