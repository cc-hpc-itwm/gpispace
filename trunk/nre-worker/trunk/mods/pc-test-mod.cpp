#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <string>
#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

static void test_local_alloc(data_t &p) throw (std::exception)
{
  MLOG(INFO, "testing local allocation...");
  const std::size_t num_ints = p.at("size").token().data_as<std::size_t>();
  fvm::util::local_allocation test_alloc(num_ints * sizeof(int));
  ASSERT_LALLOC(test_alloc);

  // fill shared memory with ints
  int *int_array = (int*)(fvmGetShmemPtr());
  for (std::size_t i(0); i < num_ints; ++i)
  {
	int_array[i] = 0xdeadbeef;
  }

  // put memory to local allocation
  fvmCommHandle_t comm = fvmPutLocalData(test_alloc
									   , 0
									   , test_alloc.size
									   , 0
									   );
  fvmCommHandleState_t commState = waitComm(comm);
  if (commState != COMM_HANDLE_OK)
  {
	MLOG(ERROR, "fvmPutLocalData() failed: " << commState);
	throw std::runtime_error("fvmPutLocalData failed!");
  }

  // clear shared memory again
  memset((int*)(fvmGetShmemPtr()), 0, test_alloc.size);

  // get memory back from local allocation
  comm = fvmGetLocalData(test_alloc
					   , 0
					   , test_alloc.size
					   , 0
					   );
  commState = waitComm(comm);
  if (commState != COMM_HANDLE_OK)
  {
	MLOG(ERROR, "fvmGetLocalData() failed: " << commState);
	throw std::runtime_error("fvmGetLocalData failed!");
  }

  // verify data
  int_array = (int*)(fvmGetShmemPtr());
  for (std::size_t i(0); i < num_ints; ++i)
  {
	if (int_array[i] != (int)0xdeadbeef)
	{
	  MLOG(ERROR, "data verification failed, expected: " << 0xdeadbeef << " got: " << int_array[i]);
	  throw std::runtime_error("data verification failed!");
	}
  }
  MLOG(INFO, "local allocation test successful");

  p["error_code"].token().data(0);
}

SDPA_MOD_INIT_START(pc-test)
{
  SDPA_REGISTER_FUN_START(test_local_alloc);
    SDPA_ADD_INP( "size", std::size_t );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(test_local_alloc);
}
SDPA_MOD_INIT_END(pc-test)
