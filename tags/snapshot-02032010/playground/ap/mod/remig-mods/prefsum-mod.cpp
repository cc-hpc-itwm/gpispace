#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <remig/rePrefxSum.h>

using namespace sdpa::modules;

void prefsum (data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  cfg_t node_config;
  fvm::util::get_data(&node_config, memhandle_for_configuration);

  int retval = rePrefixSum(&node_config);
  if (retval != 1)
  {
	LOG(ERROR, "rePrefxSum failed!");
	throw std::runtime_error("rePrefxSum failed!");
  }

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(prefsum)
{
  SDPA_REGISTER_FUN_START(prefsum);
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_OUT("seq", char *);
  SDPA_REGISTER_FUN_END(prefsum);
}
SDPA_MOD_INIT_END(prefsum)
