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

  // allocate config structure
  cfg_t node_config;
  DLOG(DEBUG, "calling c_read_config");
  c_read_config(config_file, &node_config);

  int retval(1);
  if (node_config.hndScratch == 0)
  {
	LOG(ERROR, "global scratch allocation failed!");
	retval = -1;
  }
  if (node_config.hndGlbVMspace == 0)
  {
	LOG(ERROR, "global vmspace allocation failed!");
	retval = -1;
  }

  if (retval != 1)
  {
	// free the memory!
	fvmGlobalFree(node_config.hndScratch);
	fvmGlobalFree(node_config.hndGlbVMspace);

	throw std::runtime_error("at least one allocation failed!");
  }

  retval = reApplInit(&node_config);

  if (retval != 1)
  {
	// free the memory!
	fvmGlobalFree(node_config.hndScratch);
	fvmGlobalFree(node_config.hndGlbVMspace);

	MLOG(FATAL, "reApplInit failed: " << retval);
	throw std::runtime_error("reApplInit failed!");
  }

  // now we get to some hack, we have to retrieve the remig-global structure
  TReGlbStruct re_global_struct;
  fvm::util::get_data(&re_global_struct, node_config.hndGlbVMspace, 0 /*rank*/);
  
  DLOG(DEBUG, "retrieved reGlbStruct: nx(out)=" << re_global_struct.nx_out << " ny(out)=" << re_global_struct.ny_out << " #freq="<<re_global_struct.nwH << " #depth=" << re_global_struct.nz);
  DLOG(DEBUG, "reGlbHandle=" << node_config.hndGlbVMspace << " reScrHandle=" << node_config.hndScratch);

  const unsigned long number_of_frequencies (re_global_struct.nwH);
  const unsigned long number_of_depthlevels (re_global_struct.nz);
  const unsigned long number_of_parallel_propagators (fvmGetNodeCount());

  // allocate the temporary output volume
  //    the calc_one_level uses this to store its output later
  //    the update will take input from the real output + this temporary, make the update, write back

  const fvmSize_t temp_output_volume_per_node = (1 + ((number_of_frequencies - 1) / fvmGetNodeCount())) * re_global_struct.nx_out * re_global_struct.ny_out * sizeof(float);
  fvm::util::global_allocation memhandle_for_temp_outputvolume(temp_output_volume_per_node);
  ASSERT_GALLOC(memhandle_for_temp_outputvolume);

  // distribute config to all nodes
  fvm::util::distribute_single_data(&node_config, memhandle_for_configuration);

  MLOG (DEBUG, "number_of_frequencies = " << number_of_frequencies);
  MLOG (DEBUG, "number_of_depthlevels = " << number_of_depthlevels);
  MLOG (DEBUG, "number_of_parallel_propagators = " << number_of_parallel_propagators);
  MLOG (DEBUG, "memhandle_for_temp_outputvolume = " << (fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  MLOG (DEBUG, "memhandle_for_configuration = " << (fvmAllocHandle_t)memhandle_for_configuration);

  params["number_of_frequencies"].token().data(number_of_frequencies);
  params["number_of_depthlevels"].token().data(number_of_depthlevels);
  params["number_of_parallel_propagators"].token().data(number_of_parallel_propagators);
  params["memhandle_for_temp_outputvolume_update_A"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_B"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_C"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_D"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_E"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_F"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_G"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_update_H"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_temp_outputvolume_calc"].token().data((fvmAllocHandle_t)memhandle_for_temp_outputvolume);
  params["memhandle_for_configuration"].token().data((fvmAllocHandle_t)memhandle_for_configuration);

  params["seq"].token().data("SEQ");

  // commit allocations
  memhandle_for_temp_outputvolume.commit();
  memhandle_for_configuration.commit();
}

SDPA_MOD_INIT_START(init)
{
  SDPA_REGISTER_FUN_START(init);
    SDPA_ADD_INP( "config_file", char * );

    SDPA_ADD_OUT( "number_of_frequencies", unsigned long );
    SDPA_ADD_OUT( "number_of_depthlevels", unsigned long );
    SDPA_ADD_OUT( "number_of_parallel_propagators", unsigned long );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_A", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_B", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_C", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_D", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_E", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_F", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_G", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_H", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_calc", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_OUT("seq", char *);
  SDPA_REGISTER_FUN_END(init);
}
SDPA_MOD_INIT_END(init)
