#include <fhglog/fhglog.hpp>
#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fstream>
#include <string>
#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

static void dump_local_memory(data_t &params) throw (std::exception)
{
  const fvmAllocHandle_t memory_handle = params.at("handle").token().data_as<fvmAllocHandle_t>();
  ASSERT_HANDLE(memory_handle);

  const fvmOffset_t offset = params.at("offset").token().data_as<fvmOffset_t>();
  const fvmSize_t size = params.at("size").token().data_as<fvmSize_t>();
  const std::string file = params.at("file").token().data_as<std::string>();

  DMLOG(INFO, "dumping local memory: handle=" << memory_handle << " offset=" << offset << " size=" << size << " to file '" << file << "'");

  std::ofstream ofs(file.c_str());
  if (! ofs)
  {
	MLOG(ERROR, "could not open output file for writing!");
	throw std::runtime_error("could not open output file for writing!");
  }

  fvmSize_t bytes_to_transfer = size;
  fvmSize_t bytes_transfered = 0;

  const fvmSize_t max_chunk_size = (1 << 23); // 8 MB

  while (bytes_to_transfer > 0)
  {
	const fvmSize_t transfer_size = std::min (bytes_to_transfer, max_chunk_size);
	MLOG(TRACE, "transfering " << transfer_size << " bytes of memory");
	fvmCommHandle_t comm = fvmGetLocalData (memory_handle, offset + bytes_transfered, transfer_size, 0);
	fvmCommHandleState_t comm_state = waitComm(comm);
	if (comm_state != COMM_HANDLE_OK)
	{
	  MLOG(ERROR, "could not fvmGetLocalData(): " << comm_state);
	  throw std::runtime_error("could not fvmGetLocalData()");
	}

	ofs.write( (char*)fvmGetShmemPtr(), transfer_size);
	bytes_to_transfer -= transfer_size;
	bytes_transfered  += transfer_size;
  }
  DMLOG(INFO, "dumped local memory");
}

static void dump_global_memory(data_t &params) throw (std::exception)
{
  const fvmAllocHandle_t memory_handle = params.at("handle").token().data_as<fvmAllocHandle_t>();
  ASSERT_HANDLE(memory_handle);

  const fvmOffset_t offset = params.at("offset").token().data_as<fvmOffset_t>();
  const fvmSize_t size = params.at("size").token().data_as<fvmSize_t>();
  const std::string file = params.at("file").token().data_as<std::string>();

  DMLOG(INFO, "dumping global memory: handle=" << memory_handle << " offset=" << offset << " size=" << size << " to file '" << file << "'");

  std::ofstream ofs(file.c_str());
  if (! ofs)
  {
	MLOG(ERROR, "could not open output file for writing!");
	throw std::runtime_error("could not open output file for writing!");
  }

  fvmSize_t bytes_to_transfer = size;
  fvmSize_t bytes_transfered = 0;

  const fvmSize_t max_chunk_size = (1 << 23); // 8 MB
  fvm::util::local_allocation scratch(max_chunk_size);
  ASSERT_LALLOC(scratch);

  while (bytes_to_transfer > 0)
  {
	const fvmSize_t transfer_size = std::min (bytes_to_transfer, max_chunk_size);
	MLOG(TRACE, "transfering " << transfer_size << " bytes of memory");
	fvmCommHandle_t comm = fvmGetGlobalData (memory_handle, offset + bytes_transfered, transfer_size, 0, scratch);
	fvmCommHandleState_t comm_state = waitComm(comm);
	if (comm_state != COMM_HANDLE_OK)
	{
	  MLOG(ERROR, "could not fvmGetLocalData(): " << comm_state);
	  throw std::runtime_error("could not fvmGetLocalData()");
	}

	ofs.write( (char*)fvmGetShmemPtr(), transfer_size);
	bytes_to_transfer -= transfer_size;
	bytes_transfered  += transfer_size;
  }
  DMLOG(INFO, "dumped local memory");
}

SDPA_MOD_INIT_START(debug)
{
  SDPA_REGISTER_FUN_START(dump_local_memory);
	SDPA_ADD_INP( "handle", fvmAllocHandle_t );
	SDPA_ADD_INP( "offset", fvmOffset_t );
	SDPA_ADD_INP( "size", fvmSize_t );
	SDPA_ADD_INP( "file", std::string );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(dump_local_memory);

  SDPA_REGISTER_FUN_START(dump_global_memory);
	SDPA_ADD_INP( "handle", fvmAllocHandle_t );
	SDPA_ADD_INP( "offset", fvmOffset_t );
	SDPA_ADD_INP( "size", fvmSize_t );
	SDPA_ADD_INP( "file", std::string );
    SDPA_ADD_OUT( "error_code", int );
  SDPA_REGISTER_FUN_END(dump_global_memory);
}
SDPA_MOD_INIT_END(debug)
