#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

#include <remig/calcOneLevl.h>
#include "remig-helpers.hpp"

using namespace sdpa::modules;

void calc (data_t &params) throw (std::exception)
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

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  MLOG (DEBUG, "memhandle_for_temp_outputvolume = " << memhandle_for_temp_outputvolume);
  MLOG (DEBUG, "slice_and_depth = " << slice_and_depth);
  MLOG (DEBUG, "number_of_depthlevels = " << number_of_depthlevels);

  const unsigned long slice = slice_and_depth / number_of_depthlevels;
  const unsigned long depth = slice_and_depth % number_of_depthlevels;

  MLOG (DEBUG, "slice = " << slice);
  MLOG (DEBUG, "depth = " << depth);

  cfg_t node_config;
  fvm::util::get_data(&node_config, memhandle_for_configuration);

  TReGlbStruct re_global_struct;
  fvm::util::get_data(&re_global_struct, node_config.hndGlbVMspace);

  std::size_t number_of_frequencies = re_global_struct.nwH;
  // calculate the slice distribution
  int nwHlocal [fvmGetNodeCount()+1];
  int nwHdispls[fvmGetNodeCount()+1+1];
  int pIWonND  [number_of_frequencies];

  remig::detail::calculateDistribution(number_of_frequencies, nwHlocal, nwHdispls);
  remig::detail::fill_frequency_on_node_array(number_of_frequencies, pIWonND, nwHlocal, nwHdispls);

  // get the input slice
  const std::size_t nx_in = re_global_struct.nx_fft;
  const std::size_t ny_in = re_global_struct.ny_fft;
  const std::size_t input_slice_size = nx_in * ny_in * sizeof(MKL_Complex8);
  fvmOffset_t input_slice_offset = remig::detail::calculateRemigSliceOffset(
		slice
	  , input_slice_size
	  , node_config.nodalSharedSpaceSize
	  , node_config.ofsInp
	  , pIWonND
	  , nwHdispls);

  MKL_Complex8 *input_slice = new MKL_Complex8[nx_in * ny_in];

  DLOG(DEBUG, "input starts at: " << node_config.ofsInp);
  DLOG(DEBUG, "input slice offset: " << input_slice_offset);

  fvmCommHandle_t comm_handle;
  fvmCommHandleState_t comm_status;

  // FIXME: can we use the node_config.scratch?
  fvm::util::local_allocation scratch(input_slice_size);
  comm_handle = fvmGetGlobalData(
		  node_config.hndGlbVMspace
		, input_slice_offset
		, nx_in * ny_in * sizeof(MKL_Complex8)
		, 0 // shmem
		, scratch
  );
  comm_status = waitComm(comm_handle);
  assert(comm_status == COMM_HANDLE_OK);
  memcpy(input_slice, fvmGetShmemPtr(), input_slice_size);

  const std::size_t nx_out = re_global_struct.nx_out;
  const std::size_t ny_out = re_global_struct.ny_out;
  const std::size_t output_slice_size = nx_out * ny_out * sizeof(float);
  float *output_slice = new float[nx_out * ny_out];

  int retval = reCalcOneLevl(&node_config, slice, depth, input_slice, output_slice);

  {
	// write output slice back to temporary output volume
	memcpy(fvmGetShmemPtr(), output_slice, output_slice_size);
	comm_handle = fvmPutGlobalData(
		memhandle_for_temp_outputvolume
	  , slice * output_slice_size
	  , output_slice_size
	  , 0 // shmem
	  , scratch
	);
	comm_status = waitComm(comm_handle);
	assert(comm_status == COMM_HANDLE_OK);
	delete [] output_slice;
  }

  {
	// write modified input slice back to input data
	memcpy(fvmGetShmemPtr(), input_slice,  nx_in * ny_in * sizeof(MKL_Complex8));
	comm_handle = fvmPutGlobalData(
		node_config.hndGlbVMspace
	  , input_slice_offset
	  , nx_in * ny_in * sizeof(MKL_Complex8)
	  , 0 // shmem
	  , scratch
	);
	comm_status = waitComm(comm_handle);
	assert(comm_status == COMM_HANDLE_OK);
	delete [] input_slice;
  }

  params["slice_and_depth_OUT"].token().data(slice_and_depth);
}

SDPA_MOD_INIT_START(calc)
{
  SDPA_REGISTER_FUN_START(calc);
    SDPA_ADD_INP( "slice_and_depth", unsigned long );
    SDPA_ADD_INP( "number_of_depthlevels", unsigned long );
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );
    SDPA_ADD_INP( "memhandle_for_temp_outputvolume", fvmAllocHandle_t );
    SDPA_ADD_OUT( "slice_and_depth_OUT", unsigned long );
  SDPA_REGISTER_FUN_END(calc);
}
SDPA_MOD_INIT_END(calc)
