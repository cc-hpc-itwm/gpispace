#include <fstream>

#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

#include <remig/reGlbStructs.h>
#include <remig/reWriteOutp.h>

using namespace sdpa::modules;

void writeoutp (data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  cfg_t node_config;
  fvm::util::get_data(&node_config, memhandle_for_configuration);

  int retval = reWriteOutp(&node_config);
  if (retval != 1)
  {
	MLOG(ERROR, "reWriteOutp failed!");
	throw std::runtime_error("reWriteOutp failed!");
  }

  params["seq"].token().data("SEQ");
  params["output_file"].token().data(node_config.output_file);
}

SDPA_MOD_INIT_START(writeoutp)
{
  SDPA_REGISTER_FUN_START(writeoutp);
    SDPA_ADD_INP("memhandle_for_configuration", fvmAllocHandle_t );
    SDPA_ADD_OUT("seq", char* );
    SDPA_ADD_OUT("output_file", char* );
  SDPA_REGISTER_FUN_END(writeoutp);
}
SDPA_MOD_INIT_END(writeoutp)
