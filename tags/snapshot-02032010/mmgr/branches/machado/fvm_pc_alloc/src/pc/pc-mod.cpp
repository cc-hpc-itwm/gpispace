#include <string>
#include <cstdlib> // malloc, free

#include <Module.hpp>
#include <fvmConfig.h>
#include <pc.h>

using namespace sdpa::modules;

void readConfigFile(Module::data_t &params)
{
  // char * path = params["path"].token().data_as<char*>();
  std::string path = params["path"].token().data();
  int numoptions = params["numoptions"].token().data_as<int>();

//   char * path =   "/u/herc/machado/bin/fvmconfig";
//   int numoptions = 4;
  configFile_t *config = (configFile_t *) malloc(sizeof(configFile_t));
  *config = readConfigFile(path.c_str(), numoptions);

  params["out"].token().data((void *)config);
}

//void fvmConnect(sdpa::modules::Module::data_t &params)
void fvmConnect(Module::data_t &params)
{
  
  configFile_t *config  = (configFile_t *) params["configFile"].token().data_as<void*>();

  int ret = fvmConnect(*config);
  
  params["out"].token().data(ret);
  
}

//void fvmLeave(sdpa::modules::Module::data_t &params)
void fvmLeave(Module::data_t &params)
{
  fvmLeave();
}

//void fvmGlobalAlloc(sdpa::modules::Module::data_t &params);
void fvmGlobalAlloc(Module::data_t &params)
{
  const std::size_t bytes = params["size"].token().data_as<std::size_t>();
  fvmAllocHandle_t handle = fvmGlobalAlloc(bytes);
  if (0 == handle) {
    throw std::runtime_error("Global memory allocation failed");
  }

  params["out"].token().data(handle);

}

//void fvmGlobalFree(sdpa::modules::Module::data_t &params)
void fvmGlobalFree(Module::data_t &params)
{
  fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  int ret = fvmGlobalFree(handle);

  params["out"].token().data(ret);
  
}

//void fvmLocalAlloc(sdpa::modules::Module::data_t &params);
void fvmLocalAlloc(Module::data_t &params)
{
  const std::size_t bytes = params["size"].token().data_as<std::size_t>();
  fvmAllocHandle_t handle = fvmLocalAlloc(bytes);
  if (0 == handle) {
    throw std::runtime_error("Global memory allocation failed");
  }

  params["out"].token().data(handle);

}

//void fvmLocalFree(sdpa::modules::Module::data_t &params)
void fvmLocalFree(Module::data_t &params)
{
  fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  int ret = fvmLocalFree(handle);

  params["out"].token().data(ret);

}

//void fvmGetGlobalData(sdpa::modules::Module::data_t &params)
void fvmGetGlobalData(Module::data_t &params)
{
  const fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  const fvmOffset_t fvmOffset = params["fvmOffset"].token().data_as<fvmOffset_t>();
  const size_t size = params["size"].token().data_as<std::size_t>();
  const fvmShmemOffset_t shmemOffset = params["shmemOffset"].token().data_as<fvmShmemOffset_t>();
  const fvmAllocHandle_t scratchHandle = params["scratchHandle"].token().data_as<fvmAllocHandle_t>();

  fvmCommHandle_t commHandle = fvmGetGlobalData(handle, fvmOffset, size,
						shmemOffset, scratchHandle);

  params["out"].token().data(commHandle);

}

//void fvmPutGlobalData(sdpa::modules::Module::data_t &params)
void fvmPutGlobalData(Module::data_t &params)
{
  const fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  const fvmOffset_t fvmOffset = params["fvmOffset"].token().data_as<fvmOffset_t>();
  const size_t size = params["size"].token().data_as<std::size_t>();
  const fvmShmemOffset_t shmemOffset = params["shmemOffset"].token().data_as<fvmShmemOffset_t>();
  const fvmAllocHandle_t scratchHandle = params["scratchHandle"].token().data_as<fvmAllocHandle_t>();

  fvmCommHandle_t commHandle = fvmPutGlobalData(handle, fvmOffset, size,
						shmemOffset, scratchHandle);

  params["out"].token().data(commHandle);

}


void fvmGetLocalData(Module::data_t &params)
{
  const fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  const fvmOffset_t fvmOffset = params["fvmOffset"].token().data_as<fvmOffset_t>();
  const size_t size = params["size"].token().data_as<std::size_t>();
  const fvmShmemOffset_t shmemOffset = params["shmemOffset"].token().data_as<fvmShmemOffset_t>();

  fvmCommHandle_t commHandle = fvmGetLocalData(handle, fvmOffset, size, shmemOffset);

  params["out"].token().data(commHandle);

}


void fvmPutLocalData(Module::data_t &params)
{
  const fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  const fvmOffset_t fvmOffset = params["fvmOffset"].token().data_as<fvmOffset_t>();
  const size_t size = params["size"].token().data_as<std::size_t>();
  const fvmShmemOffset_t shmemOffset = params["shmemOffset"].token().data_as<fvmShmemOffset_t>();

  fvmCommHandle_t commHandle = fvmPutLocalData(handle, fvmOffset, size,shmemOffset);

  params["out"].token().data(commHandle);

}

//void waitComm(sdpa::modules::Module::data_t &params)
void waitComm(Module::data_t &params)
{
  const fvmCommHandle_t handle = params["handle"].token().data_as<fvmCommHandle_t>();

  int ret = waitComm(handle);

  params["out"].token().data(ret);
}

void fvmGetShmemPtr(Module::data_t &params)
{
  void *ptr = fvmGetShmemPtr();
  params["out"].token().data(ptr);

}


//for now is here for the LEAVE but should go away
int doRequest(fvmRequest_t op_request);

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, readConfigFile);
    SDPA_REGISTER_FUN(mod, fvmConnect);
    SDPA_REGISTER_FUN(mod, fvmLeave);
    SDPA_REGISTER_FUN(mod, fvmGlobalAlloc);
    SDPA_REGISTER_FUN(mod, fvmGlobalFree);
    SDPA_REGISTER_FUN(mod, fvmLocalAlloc);
    SDPA_REGISTER_FUN(mod, fvmLocalFree);
    SDPA_REGISTER_FUN(mod, fvmGetGlobalData);
    SDPA_REGISTER_FUN(mod, fvmPutGlobalData);
    SDPA_REGISTER_FUN(mod, fvmGetLocalData);
    SDPA_REGISTER_FUN(mod, fvmPutLocalData);
    SDPA_REGISTER_FUN(mod, waitComm);
    SDPA_REGISTER_FUN(mod, fvmGetShmemPtr);    

  }
}
