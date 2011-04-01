#include <fvm-pc/pc.hpp>

#define LOG_COMPONENT "fvm-pc-compat"
#include <fhglog/minimal.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#warning "the pc module should not require the loader"
#include <we/loader/macros.hpp>

#include <gpi-space/pc/client/api.hpp>

static gpi::pc::client::api_t & gpi_api ()
{
  static gpi::pc::client::api_t a;
  return a;
}

static gpi::pc::type::info::descriptor_t gpi_info;
static void *shm_ptr = 0;
static fvmSize_t shm_size = 0;

int fvmConnect()
{
  throw std::runtime_error ("fvmConnect not implemented anymore");
}

int fvmLeave()
{
  shm_ptr = 0;
  shm_size = 0;
  gpi_api().stop();
  return 0;
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  return gpi_api().alloc ( 1 // GPI
                         , size
                         , "fvm-pc-compat-global-no-name"
                         , gpi::pc::type::handle::F_GLOBAL
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

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{
  return 0;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{
  return 0;
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{
  return 0;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{
  return 0;
}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  gpi::pc::type::size_t num_finished
    (gpi_api().wait (static_cast<gpi::pc::type::queue_id_t>(handle)));
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
  // parse config files
  //     gpi.rc -> path to socket
  //     pc.rc  -> memory size?
  fvmSize_t shmem_size ( 500 * (1 << 20) );
  if (getenv("FVM_PC_SHMSZ"))
  {
    shm_size = boost::lexical_cast<fvmSize_t>(getenv("FVM_PC_SHMSZ"));
  }

  try
  {
    // TODO: loop and wait

    // TODO:
    // collect sockets
    //    if multiple sockets: throw
    // else
    //    set the path
    namespace fs = boost::filesystem;
    fs::path socket_path ("/var/tmp/gpi-space");
    socket_path /= ("GPISpace-" + boost::lexical_cast<std::string>(getuid()));

    gpi_api().path ("/var/tmp/gpi-space/GPISpace-5201/control");

    CLOG( INFO
        , LOG_COMPONENT
        , "initializing fvm-pc-compat:"
        << " shm size " << shmem_size
        << " socket " << gpi_api().path()
        );

    gpi_api().start ();
    gpi_info = gpi_api().collect_info();

    // register segment
    gpi_api().register_segment ( "fvm-pc-compat"
                               , shmem_size
                               , gpi::pc::type::segment::F_EXCLUSIVE
                               | gpi::pc::type::segment::F_FORCE_UNLINK
                               );
    shm_ptr = (gpi_api().segments().begin()->second)->ptr();
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "could not connect to gpi-space: " << ex.what());
    throw;
  }

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
