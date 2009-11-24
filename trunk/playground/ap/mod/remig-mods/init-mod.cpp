#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

// remig includes
#include <remig/reApplInit.h>
#include <remig/cfg.h>

using namespace sdpa::modules;

void init (data_t &params) throw (std::exception)
{
  const std::string config_file (params.at("config_file").token().data());

  // read config file, parse, setup everything

  MLOG(DEBUG, "config_file = " << config_file);

  fvm::util::global_allocation memhandle_for_configuration(sizeof(cfg_t));
  ASSERT_GALLOC(memhandle_for_configuration);

  DLOG(DEBUG, "allocated memory handle: size=" << sizeof(cfg_t) << " hdl=" << memhandle_for_configuration);
  // allocate config structure
  cfg_t node_configs[fvmGetNodeCount()];
  DLOG(DEBUG, "calling c_read_config");
  c_read_config(config_file, node_configs);

  int retval(1);
  if (node_configs[0].hndScratch == 0)
  {
	LOG(ERROR, "global scratch allocation failed!");
	retval = -1;
  }
  if (node_configs[0].hndGlbVMspace == 0)
  {
	LOG(ERROR, "global vmspace allocation failed!");
	retval = -1;
  }

  if (retval != 1)
  {
	// free the memory!
	fvmGlobalFree(node_configs[0].hndScratch);
	fvmGlobalFree(node_configs[0].hndGlbVMspace);

	throw std::runtime_error("at least one allocation failed!");
  }

  retval = reApplInit(&node_configs[0]);

  if (retval != 1)
  {
	// free the memory!
	fvmGlobalFree(node_configs[0].hndScratch);
	fvmGlobalFree(node_configs[0].hndGlbVMspace);

	throw std::runtime_error("reApplInit failed!");
  }

  // distributing config array to nodes
  fvm::util::distribute_data(node_configs, memhandle_for_configuration);

  // now we get to some hack, we have to retrieve the remig-global structure
  TReGlbStruct re_global_struct;
  fvm::util::get_data(&re_global_struct, node_configs[0].hndGlbVMspace, 0 /*rank*/);
  
  DLOG(DEBUG, "retrieved reGlbStruct: nx(out)=" << re_global_struct.nx_out << " ny(out)=" << re_global_struct.ny_out << " #freq="<<re_global_struct.nwH << " #depth=" << re_global_struct.nz);
  DLOG(DEBUG, "reGlbHandle=" << node_configs[0].hndGlbVMspace << " reScrHandle=" << node_configs[0].hndScratch);

  const unsigned long number_of_frequencies (re_global_struct.nwH);
  const unsigned long number_of_depthlevels (re_global_struct.nz);
  const unsigned long number_of_parallel_propagators (8);

  // allocate the temporary output volume
  //    the calc_one_level uses this to store its output later
  //    the update will take input from the real output + this temporary, make the update, write back

  const fvmSize_t temp_output_volume_per_node = (1 + ((number_of_frequencies - 1) / fvmGetNodeCount())) * re_global_struct.nx_out * re_global_struct.ny_out * sizeof(float);
  fvm::util::global_allocation memhandle_for_outputvolume(temp_output_volume_per_node);
  ASSERT_GALLOC(memhandle_for_outputvolume);

  // for now, just free the mem immediately
  MLOG (DEBUG, "number_of_frequencies = " << number_of_frequencies);
  MLOG (DEBUG, "number_of_depthlevels = " << number_of_depthlevels);
  MLOG (DEBUG, "number_of_parallel_propagators = " << number_of_parallel_propagators);
  MLOG (DEBUG, "memhandle_for_outputvolume = " << (fvmAllocHandle_t)memhandle_for_outputvolume);
  MLOG (DEBUG, "memhandle_for_configuration = " << (fvmAllocHandle_t)memhandle_for_configuration);

  params["number_of_frequencies"].token().data(number_of_frequencies);
  params["number_of_depthlevels"].token().data(number_of_depthlevels);
  params["number_of_parallel_propagators"].token().data(number_of_parallel_propagators);
  params["memhandle_for_outputvolume"].token().data((fvmAllocHandle_t)memhandle_for_outputvolume);
  params["memhandle_for_configuration"].token().data((fvmAllocHandle_t)memhandle_for_configuration);

  params["seq"].token().data("SEQ");

  // commit allocations
  memhandle_for_outputvolume.commit();
  memhandle_for_configuration.commit();
}

SDPA_MOD_INIT_START(init)
{
  SDPA_REGISTER_FUN_START(init);
    SDPA_ADD_INP( "config_file", char * );

    SDPA_ADD_OUT( "number_of_frequencies", unsigned long );
    SDPA_ADD_OUT( "number_of_depthlevels", unsigned long );
    SDPA_ADD_OUT( "number_of_parallel_propagators", unsigned long );
    SDPA_ADD_OUT( "memhandle_for_outputvolume", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_OUT("seq", char *);
  SDPA_REGISTER_FUN_END(init);
}
SDPA_MOD_INIT_END(init)
