#include <fhglog/fhglog.hpp>
#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fstream>
#include <string>
#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

static void dump_memory(data_t &params) throw (std::exception)
{
  const fvmAllocHandle_t memory_handle = params.at("handle").token().data_as<fvmAllocHandle_t>();
  ASSERT_HANDLE(memory_handle);

  const fvmOffset_t offset = params.at("offset").token().data_as<fvmOffset_t>();
  const fvmSize_t size = params.at("size").token().data_as<fvmSize_t>();
  const std::string file = params.at("file").token().data_as<std::string>();

  DMLOG(INFO, "dumping memory: handle=" << memory_handle << " offset=" << offset << " size=" << size << " to file '" << file << "'");

  params["error_code"].token().data(0);
}

SDPA_MOD_INIT_START(debug)
{
  SDPA_REGISTER_FUN_START(dump_memory);
	SDPA_ADD_INP( "handle", fvmAllocHandle_t );
	SDPA_ADD_INP( "offset", fvmOffset_t );
	SDPA_ADD_INP( "size", fvmSize_t );
	SDPA_ADD_INP( "file", std::string );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(dump_memory);
}
SDPA_MOD_INIT_END(debug)
