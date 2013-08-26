#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <mmgr/dtmmgr.h>
#include <we/loader/macros.hpp>
#include <fvm-pc/pc.hpp>
#if ! defined(MAX_SHMEM_SIZE)
#error "MAX_SHMEM_SIZE must be defined!"
#endif
#if ! defined(MAX_FVM_SIZE)
#error "MAX_FVM_SIZE must be defined!"
#endif

typedef enum {
  START,
  FGLOBALLOC,
  FLOCALLOC,
  FGLOBALFREE,
  FLOCALFREE,
  PUTGLOBAL,
  GETGLOBAL,
  PUTLOCAL,
  GETLOCAL,
  READDMA,
  WRITEDMA,
  WAITCOMM,
  LEAVE
} fvmOperation_t;

// global variables
static DTmmgr_t dtmmgr(NULL);

static fvmSize_t pcFVMSize(0);
static void * pcFVM(0);

static fvmSize_t pcShmSize(0);
static void * pcShm(0);

enum FVM_PC_API_ERRORS
{
        FVM_EGENERAL   = -1
  , FVM_EINVALSZ   = -2
  , FVM_EOUTOFMEM  = -3
  , FVM_ETOOLARGE  = -4
  , FVM_ESRCH      = -5
};

static void cleanUp()
{
  if (pcShm)
  {
        fprintf (stderr, "fvm-pc: removing shared memory segment: %p\n", pcShm);
        free(pcShm); pcShm = NULL;
        pcShmSize = 0;
  }
  if (pcFVM)
  {
        fprintf (stderr, "fvm-pc: removing FVM segment: %p\n", pcFVM);
        free(pcFVM); pcFVM = NULL;
        pcFVMSize = 0;
  }

  if (dtmmgr)
  {
    fprintf (stderr, "fvm-pc: finalizing mmgr: %p %p\n", &dtmmgr, dtmmgr);
    Size_t Bytes = dtmmgr_finalize (&dtmmgr);
        fprintf (stderr, "fvm-pc: mmgr freed bytes = %lu" "\n", Bytes);
  }
}

// ------ Internal functions for Process Container (not to be used by appllication)
//static int doRequest(fvmRequest_t /* op_request */ ) { return 0; }

//-------------------- Interface for Process Container -----------------------
int fvmConnect(fvm_pc_config_t cfg)
{
  if (!cfg.shmemsize)
  {
    fprintf(stderr, "fvm-pc: shared-memory size (0) is illegal!\n");
    return FVM_EINVALSZ;
  }

  if (cfg.shmemsize > MAX_SHMEM_SIZE)
  {
    fprintf(stderr, "fvm-pc: sorry, the configured shared-memory size is too large for me! maximum: %lu, wanted: %lu\n", (fvmSize_t)(MAX_SHMEM_SIZE), cfg.shmemsize);
    return FVM_ETOOLARGE;
  }
  if (cfg.fvmsize > MAX_FVM_SIZE)
  {
    fprintf(stderr, "fvm-pc: sorry, the configured vm-memory size is too large for me! maximum: %lu, wanted: %lu\n", (fvmSize_t)(MAX_FVM_SIZE), cfg.fvmsize);
    return FVM_ETOOLARGE;
  }

  else
  {
    pcShmSize = cfg.shmemsize;
    fprintf(stderr, "fvm-pc: allocating %lu bytes of shared-memory\n", pcShmSize);
    pcShm = malloc(pcShmSize);
        if (! pcShm)
        {
          fprintf(stderr, "fvm-pc: shared memory allocation failed!");
          cleanUp();
          return FVM_EOUTOFMEM;
        }
        memset(pcShm, 0, pcShmSize);

    pcFVMSize = cfg.fvmsize;
    fprintf(stderr, "fvm-pc: allocating %lu bytes of VM\n", pcFVMSize);
    pcFVM = malloc(pcFVMSize);
        if (! pcFVM)
        {
          fprintf(stderr, "fvm-pc: VM memory allocation failed!");
          cleanUp();
          return FVM_EOUTOFMEM;
        }
        memset(pcFVM, 0, pcFVMSize);

    fprintf(stderr, "fvm-pc: using %lu bytes of VM memory\n", pcFVMSize);

    dtmmgr_init (&dtmmgr, pcFVMSize, 2 /* aligned to 2-byte boundary */);
        dtmmgr_info (dtmmgr);

    return 0;
  }
}

int fvmLeave()
{
  cleanUp();
  return 0;
}

fvmAllocHandle_t fvmGlobalAllocExact (fvmSize_t size, const char *name)
{
  return fvmGlobalAlloc (size, name);
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size, const char *)
{
  return fvmGlobalAlloc(size);
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  static fvmAllocHandle_t num_allocs = 0;

  fvmAllocHandle_t ptr= (++num_allocs);
  AllocReturn_t AllocReturn = dtmmgr_alloc( &dtmmgr, (Handle_t)ptr, (Arena_t)ARENA_UP, (Size_t)size);
  switch (AllocReturn)
  {
        case ALLOC_SUCCESS:
          return ptr;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
#ifndef NDEBUG
          fprintf(stderr, "fvm-pc: global alloc failed: not enough contigiuous memory of size %lu!\n", size);
      dtmmgr_status (dtmmgr);
#endif
          // FIXME: set errno!
          return 0;
        default:
#ifndef NDEBUG
          fprintf(stderr, "fvm-pc: global alloc of size %lu failed!\n", size);
      dtmmgr_status (dtmmgr);
#endif
          return 0;
  }
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
  switch (HandleReturn_t ret = dtmmgr_free (&dtmmgr, ptr, ARENA_UP))
  {
        case RET_SUCCESS:
          return 0;
        case RET_HANDLE_UNKNOWN:
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          return FVM_ESRCH;
        case RET_FAILURE:
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          return FVM_EGENERAL;
        default:
          fprintf(stderr, "fvm-pc: global free failed: unknown return code: %d\n", ret);
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          return FVM_EGENERAL;
  }
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char *)
{
  return fvmLocalAlloc(size);
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
{
  static fvmAllocHandle_t num_allocs = 0;

  fvmAllocHandle_t ptr= (++num_allocs);
  AllocReturn_t AllocReturn = dtmmgr_alloc( &dtmmgr, (Handle_t)ptr, (Arena_t)ARENA_DOWN, (Size_t)size);
  switch (AllocReturn)
  {
        case ALLOC_SUCCESS:
          return ptr;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
#ifndef NDEBUG
          fprintf(stderr, "fvm-pc: local alloc failed: not enough contigiuous memory of size %lu!\n", size);
          dtmmgr_status (dtmmgr);
#endif
          // FIXME: set errno!
          return 0;
        default:
#ifndef NDEBUG
          fprintf(stderr, "fvm-pc: local alloc of size %lu failed!\n", size);
          dtmmgr_status (dtmmgr);
#endif
          return 0;
  }
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{
  switch (HandleReturn_t ret = dtmmgr_free (&dtmmgr, ptr, ARENA_DOWN))
  {
        case RET_SUCCESS:
          return 0;
        case RET_HANDLE_UNKNOWN:
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          return FVM_ESRCH;
        case RET_FAILURE:
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          return FVM_EGENERAL;
        default:
#ifndef NDEBUG
          dtmmgr_status (dtmmgr);
#endif
          // FIXME: set errno instead!
          fprintf(stderr, "fvm-pc: local free failed: unknown return code: %d\n", ret);
          return FVM_EGENERAL;
  }
}

static fvmCommHandleState_t check_bounds
( const fvmAllocHandle_t handle
, const fvmOffset_t fvmOffset /* relative to handle */
, const fvmSize_t size /* should stay inside the space allocated for handle */
, const Arena_t Arena
, POffset_t PBaseOffset
, const char * /*descr*/
)
{
  MemSize_t HandleSize = 0;

  HandleReturn_t HandleReturn =
    dtmmgr_offset_size (dtmmgr, handle, Arena, PBaseOffset, &HandleSize);

  switch (HandleReturn)
    {
    case RET_SUCCESS:
      if ((fvmOffset + size) > HandleSize)
        {
#ifndef NDEBUG
          fprintf( stderr
                 , "fvm-pc: %s out of range: \
                            handle=%lu offset=%lu size=%lu HandleSize=%lu\n"
                 , descr, handle, fvmOffset, size, HandleSize
                 );
#endif
          return COMM_HANDLE_ERROR_INVALID_SIZE;
        }
      break;
    case RET_HANDLE_UNKNOWN: return COMM_HANDLE_ERROR_INVALID_HANDLE; break;
    default: return COMM_HANDLE_ERROR; break;
    }

  return COMM_HANDLE_OK;
}

static fvmCommHandle_t fvmCommData(const fvmAllocHandle_t handle,
                                   const fvmOffset_t fvmOffset,
                                   const fvmSize_t size,
                                   const fvmShmemOffset_t shmemOffset,
                                   const fvmAllocHandle_t /* scratchHandle */,
                                   const fvmOperation_t op)
{
  Offset_t BaseOffset = 0;
  switch (op)
  {
        // FIXME: implement error handling (size mismatch)
        //        return COMM_HANDLE_ERROR_SHMEM_BOUNDARY etc as handles
        case GETGLOBAL:
          {
            fvmCommHandleState_t fvmCommHandleState =
              check_bounds (handle, fvmOffset, size, ARENA_UP, &BaseOffset, "GetGlobalData");

            if (fvmCommHandleState == COMM_HANDLE_OK)
              {
                memcpy((char*)(pcShm) + shmemOffset, (char*)(pcFVM) + BaseOffset + fvmOffset, size);
              }
            /* FIXME: encode error into handle */
            return fvmCommHandleState;
          }
          break;
        case PUTGLOBAL:
          {
            fvmCommHandleState_t fvmCommHandleState =
              check_bounds (handle, fvmOffset, size, ARENA_UP, &BaseOffset, "PutGlobalData");

            if (fvmCommHandleState == COMM_HANDLE_OK)
              {
                memcpy((char*)(pcFVM) + BaseOffset + fvmOffset, (char*)(pcShm) + shmemOffset, size);
              }
            return fvmCommHandleState;
          }
          break;
        case GETLOCAL:
          {
            fvmCommHandleState_t fvmCommHandleState =
              check_bounds (handle, fvmOffset, size, ARENA_DOWN, &BaseOffset, "GetLocalData");

            if (fvmCommHandleState == COMM_HANDLE_OK)
              {
                memcpy((char*)(pcShm) + shmemOffset, (char*)(pcFVM) + BaseOffset + fvmOffset, size);
              }
            return fvmCommHandleState;
          }
          break;
        case PUTLOCAL:
          {
            fvmCommHandleState_t fvmCommHandleState =
              check_bounds (handle, fvmOffset, size, ARENA_DOWN, &BaseOffset, "PutLocalData");

            if (fvmCommHandleState == COMM_HANDLE_OK)
              {
                memcpy((char*)(pcFVM) + BaseOffset + fvmOffset, (char*)(pcShm) + shmemOffset, size);
              }
            return fvmCommHandleState;
          }
          break;
        default:
          fprintf(stderr, "fvm-pc: fvmCommData() unsupported comm-operation:%d\n", op);
          return -1;
  }

  return COMM_HANDLE_OK;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t scratchHandle)
{
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, GETGLOBAL);
  return commhandle;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
                                 const fvmOffset_t fvmOffset,
                                 const fvmSize_t size,
                                 const fvmShmemOffset_t shmemOffset,
                                 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, PUTGLOBAL);
  return commhandle;

}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, PUTLOCAL);
  return commhandle;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
                                const fvmOffset_t fvmOffset,
                                const fvmSize_t size,
                                const fvmShmemOffset_t shmemOffset)
{
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, GETLOCAL);
  return commhandle;

}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  if (handle < 0)
    {
      throw std::runtime_error ("waitComm: invalid handle");
    }

  // this is done only in the fake module, the communication is always synchronous!
  // error codes are encoded in the handle's value.
  return (fvmCommHandleState_t)handle;
}

void *fvmGetShmemPtr()
{
#ifdef SHMEM
  return pcShm;
#else
  return 0;
#endif
}

fvmSize_t fvmGetShmemSize()
{
  return pcShmSize;
}

int fvmGetRank()
{
  return 0;
}
int fvmGetNodeCount()
{
  return 1;
}

static void selftest (void *, const expr::eval::context &, expr::eval::context & out)
{
  std::cerr << "running self test" << std::endl;
  out.bind ("result", pnet::type::value::value_type (0L));
}

static inline long getenvlong (const char * name, const long dflt)
{
  const char * read (getenv (name));

  return read ? atol (read) : dflt;
}

WE_MOD_INITIALIZE_START (fvm);
{


  fvm_pc_config_t cfg ( getenv("FVM_PC_MSQ")
                      , getenv("FVM_PC_SHM")
                      , getenvlong ("FVM_PC_SHMSZ", 50*1024*1024)
                      , getenvlong ("FVM_PC_FVMSZ", 100*1024*1024)
                      );
  fvmConnect (cfg);
  WE_REGISTER_FUN (selftest);
}
WE_MOD_INITIALIZE_END (fvm);

WE_MOD_FINALIZE_START (fvm);
{
  fvmLeave ();
}
WE_MOD_FINALIZE_END (fvm);
