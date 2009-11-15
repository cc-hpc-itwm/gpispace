#include <fhglog/fhglog.hpp>
#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

static void global_alloc(Module::data_t &params) throw (std::exception)
{
  const fvmSize_t number_of_bytes = params.at("bytes").token().data_as<fvmSize_t>();
  DMLOG(INFO, "allocating (global) " << number_of_bytes << " bytes of data");

  fvmAllocHandle_t memhandle = fvmGlobalAlloc(number_of_bytes);
  if (memhandle == 0)
  {
     params["handle"].token().data(memhandle);
     MLOG(ERROR, "global allocation failed: probably out of memory");
     throw std::runtime_error("global allocation failed: probably out of memory");
  }

  DMLOG(INFO, "global handle " << memhandle << " points to " << number_of_bytes << " bytes of data");
  params["handle"].token().data(memhandle);
}

static void global_free(Module::data_t &params) throw (std::exception)
{
  fvmAllocHandle_t memhandle = params.at("handle").token().data_as<fvmAllocHandle_t>();
  DMLOG(INFO, "de-allocating global handle " << memhandle);

  int result = fvmGlobalFree(memhandle);
  if (result < 0)
  {
     MLOG(ERROR, "global de-allocation failed: " << result);
     params["error_code"].token().data(result);
     throw std::runtime_error("global de-allocation failed");
  }

  DMLOG(INFO, "global handle " << memhandle << " successfully deallocated");
  params["error_code"].token().data(result);
}

static void local_alloc(Module::data_t &params) throw (std::exception)
{
  const fvmSize_t number_of_bytes = params.at("bytes").token().data_as<fvmSize_t>();
  DMLOG(INFO, "locally allocating " << number_of_bytes << " bytes of data");

  fvmAllocHandle_t memhandle = fvmLocalAlloc(number_of_bytes);
  if (memhandle == 0)
  {
     params["handle"].token().data(memhandle);
     MLOG(ERROR, "local allocation failed: probably out of memory");
     throw std::runtime_error("local allocation failed: probably out of memory");
  }

  DMLOG(INFO, "local handle " << memhandle << " points to " << number_of_bytes << " bytes of data");
  params["handle"].token().data(memhandle);
}

static void local_free(Module::data_t &params) throw (std::exception)
{
  fvmAllocHandle_t memhandle = params.at("handle").token().data_as<fvmAllocHandle_t>();
  DMLOG(INFO, "de-allocating local handle " << memhandle);

  int result = fvmLocalFree(memhandle);
  if (result < 0)
  {
     MLOG(ERROR, "local de-allocation failed: " << result);
     params["error_code"].token().data(result);
     throw std::runtime_error("local de-allocation failed");
  }

  params["error_code"].token().data(result);
}

SDPA_MOD_INIT_START(fvm)
{
  // Global allocation related

  SDPA_REGISTER_FUN_START(global_alloc);
    SDPA_ADD_INP( "bytes",  fvmSize_t );
    SDPA_ADD_OUT( "handle", fvmAllocHandle_t );
  SDPA_REGISTER_FUN_END(global_alloc);

  SDPA_REGISTER_FUN_START(global_free);
    SDPA_ADD_INP( "handle",  fvmAllocHandle_t );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(global_free);


  // Local allocation related

  SDPA_REGISTER_FUN_START(local_alloc);
    SDPA_ADD_INP( "bytes",  fvmSize_t );
    SDPA_ADD_OUT( "handle", fvmAllocHandle_t );
  SDPA_REGISTER_FUN_END(local_alloc);

  SDPA_REGISTER_FUN_START(local_free);
    SDPA_ADD_INP( "handle",  fvmAllocHandle_t );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(local_free);
}
SDPA_MOD_INIT_END(fvm)
