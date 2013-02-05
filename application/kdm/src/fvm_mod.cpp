#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

static void selftest (void *, const we::loader::input_t & , we::loader::output_t & output)
{
  std::cerr << "rank := " << fvmGetRank() << std::endl;
  output.bind ("result", 0L);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (fvm_test);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (fvm-test)");

  WE_REGISTER_FUN (selftest);

  fvmAllocHandle_t l_alloc (fvmLocalAlloc (1024 * 1024 * 1024));
  std::cerr << "l_alloc = " << l_alloc << std::endl;
  std::cerr << "shmem ptr = " << fvmGetShmemPtr() << std::endl;

  int data (0xdeadbeef);
  memcpy (fvmGetShmemPtr(), &data, sizeof (data));

  std::cerr << "copied data to shmem" << std::endl;

  fvmCommHandle_t c_hdl (fvmPutLocalData (l_alloc, 0, sizeof(data), 0));

  std::cerr << "comm handle = " << c_hdl << std::endl;
  std::cerr << "waitComm(c_hdl) = " << waitComm (c_hdl) << std::endl;

  std::cerr << "localFree(l_alloc) = " << fvmLocalFree (l_alloc) << std::endl;

  fvmAllocHandle_t g_alloc (fvmGlobalAlloc (1024 * 1024 * 1024));
  waitComm (fvmPutGlobalData (g_alloc, 0, sizeof(data), 0, l_alloc));

  fvmGlobalFree (g_alloc);
}
WE_MOD_INITIALIZE_END (fvm_test);

WE_MOD_FINALIZE_START (fvm_test);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (fvm-test)");
}
WE_MOD_FINALIZE_END (fvm_test);
