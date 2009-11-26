#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

#include <remig/reGlbStructs.h>
#include "remig-helpers.hpp"

using namespace sdpa::modules;

void update (data_t &params)
{
  const unsigned long slice_and_depth
    (params.at("slice_and_depth").token().data_as<unsigned long>());

  const unsigned long number_of_depthlevels 
    (params.at("number_of_depthlevels").token().data_as<unsigned long>());

  const fvmAllocHandle_t memhandle_for_configuration 
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  const fvmAllocHandle_t memhandle_for_temp_outputvolume 
    (params.at("memhandle_for_temp_outputvolume").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_temp_outputvolume);

  DMLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  DMLOG (DEBUG, "memhandle_for_temp_outputvolume = " << memhandle_for_temp_outputvolume);
  DMLOG (DEBUG, "slice_and_depth = " << slice_and_depth);
  DMLOG (DEBUG, "number_of_depthlevels = " << number_of_depthlevels);

  const unsigned long slice = slice_and_depth / number_of_depthlevels;
  const unsigned long depth = slice_and_depth % number_of_depthlevels;

  DMLOG (DEBUG, "slice = " << slice);
  DMLOG (DEBUG, "depth = " << depth);

  cfg_t node_config;
  fvm::util::get_data(&node_config, memhandle_for_configuration);

  TReGlbStruct re_global_struct;
  fvm::util::get_data(&re_global_struct, node_config.hndGlbVMspace, 0);

  std::size_t number_of_frequencies = re_global_struct.nwH;
  // calculate the slice distribution
  int nwHlocal [fvmGetNodeCount()+1];
  int nwHdispls[fvmGetNodeCount()+1+1];
  int pIWonND  [number_of_frequencies];

  remig::detail::calculateDistribution(number_of_frequencies, nwHlocal, nwHdispls);
  remig::detail::fill_frequency_on_node_array(number_of_frequencies, pIWonND, nwHlocal, nwHdispls);

  fvmCommHandle_t comm_handle_slice_a;
  fvmCommHandle_t comm_handle_slice_b;
  fvmCommHandleState_t comm_status;

  // get slice from real output volume
  const std::size_t nx_out = re_global_struct.nx_out;
  const std::size_t ny_out = re_global_struct.ny_out;
  const std::size_t output_slice_size = nx_out * ny_out * sizeof(float);
  fvmOffset_t output_slice_offset = remig::detail::calculateRemigSliceOffset(
		slice
	  , output_slice_size
	  , node_config.nodalSharedSpaceSize
	  , node_config.ofsOutp
	  , pIWonND
	  , nwHdispls);

  fvm::util::local_allocation scratch_a(output_slice_size);
  ASSERT_LALLOC(scratch_a);

  fvm::util::local_allocation scratch_b(output_slice_size);
  ASSERT_LALLOC(scratch_b);

  comm_handle_slice_a = fvmGetGlobalData(
	    node_config.hndGlbVMspace
	  , output_slice_offset
	  , output_slice_size
	  , 0
	  , scratch_a);

  // get slice from temporary output volume
  comm_handle_slice_b = fvmGetGlobalData(
	    memhandle_for_temp_outputvolume
	  , slice * output_slice_size
	  , output_slice_size
	  , output_slice_size
	  , scratch_b);

  // wait for comm to finish
  comm_status = waitComm(comm_handle_slice_a);
  assert(comm_status == COMM_HANDLE_OK);
  comm_status = waitComm(comm_handle_slice_b);
  assert(comm_status == COMM_HANDLE_OK);

  // sum up
  float *output_slice   = (float*)(fvmGetShmemPtr()) + 0;
  float *finished_slice = (float*)(fvmGetShmemPtr()) + nx_out * ny_out;

  std::size_t slice_dim = nx_out * ny_out;
  for (std::size_t i(0); i < slice_dim; ++i)
  {
	output_slice[i] += finished_slice[i];
  }

  // write back to real output volume
  comm_handle_slice_a = fvmPutGlobalData(
	    node_config.hndGlbVMspace
	  , output_slice_offset
	  , output_slice_size
	  , 0
	  , scratch_a);
  // wait for comm to finish
  comm_status = waitComm(comm_handle_slice_a);
  assert(comm_status == COMM_HANDLE_OK);

  params["slice_and_depth_OUT"].token().data(slice_and_depth);
  params["memhandle_for_temp_outputvolume_OUT"].token().data(memhandle_for_temp_outputvolume);
}

SDPA_MOD_INIT_START(update)
{
  SDPA_REGISTER_FUN_START(update);
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_INP( "memhandle_for_temp_outputvolume", fvmAllocHandle_t );
    SDPA_ADD_INP( "slice_and_depth", unsigned long );
    SDPA_ADD_INP( "number_of_depthlevels", unsigned long );

    SDPA_ADD_OUT("slice_and_depth_OUT", unsigned long );
    SDPA_ADD_OUT("memhandle_for_temp_outputvolume_OUT", fvmAllocHandle_t );
  SDPA_REGISTER_FUN_END(update);
}
SDPA_MOD_INIT_END(update)
