#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <cstdlib>
#include <unistd.h>

#include <remig/calcOneLevl.h>

using namespace sdpa::modules;

static void calcDistribution(int number_of_frequencies, int *nwHlocal, int *nwHdispls)
{
    int ix;
    int rank = fvmGetRank();
    int size = fvmGetNodeCount();
    int cSize = size+1; // this is the new C-style array size

	int base=(number_of_frequencies/size);
	int rem=number_of_frequencies-size*base;
	for (ix=1; ix <= size; ix++)
	{ //do ix=1,size
	    nwHlocal[ix]=base;
	    if( ix < (rem+1) )
		{ //if (ix.lt.rem+1) then
			nwHlocal[ix] = nwHlocal[ix]+1;  //nwHlocal(ix) = nwHlocal(ix)+1
	    }//end if
	} //end do

    nwHdispls[1]=1;
    for (ix=1; ix <= size; ix++)
	{ //do ix=1,size
	  nwHdispls[ix+1]=nwHlocal[ix]+nwHdispls[ix]; //nwHdispls(ix+1)=nwHlocal(ix)+nwHdispls(ix)
    } //end do
}

static void fill_frequency_on_node_array(int number_of_frequencies, int *pIWonND, int *nwHlocal, int *nwHdispls)
{
	int i, j;
	int size = fvmGetNodeCount();

    j=0;
    for(i = 0; i < size; i++) {	 
        if(i < size-1) {
           while((j >= nwHdispls[i+1]-1) && (j < nwHdispls[i+1 +1]-1)) {
            pIWonND[j] = i;
            j++;
           }
        }
        if(i == size-1) {
           while((j >= nwHdispls[i+1]-1) && (j < number_of_frequencies)) {
            pIWonND[j] = i;
            j++;
           }
        }
    }
}

void calc (data_t &params) throw (std::exception)
{
  const unsigned long slice_and_depth
    (params.at("slice_and_depth").token().data_as<unsigned long>());

  const unsigned long number_of_depthlevels 
    (params.at("number_of_depthlevels").token().data_as<unsigned long>());

  const fvmAllocHandle_t memhandle_for_configuration 
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  const fvmAllocHandle_t memhandle_for_temp_output_volume
	(params.at("memhandle_for_temp_output_volume_calc").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_temp_output_volume);

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  MLOG (DEBUG, "memhandle_for_temp_output_volume = " << memhandle_for_temp_output_volume);
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

  calcDistribution(number_of_frequencies, nwHlocal, nwHdispls);
  fill_frequency_on_node_array(number_of_frequencies, pIWonND, nwHlocal, nwHdispls);

  fvmCommHandle_t comm_handle;
  fvmCommHandleState_t comm_status;
  // get the input slice
  const std::size_t nx_in = re_global_struct.nx_fft;
  const std::size_t ny_in = re_global_struct.ny_fft;
  MKL_Complex8 *input_slice = new MKL_Complex8[nx_in * ny_in];

  std::size_t input_slice_offset;
  {
	int node_on_which_the_slice_is = pIWonND[slice];
	int slice_relative_on_node = slice - nwHdispls[node_on_which_the_slice_is+1] + 1;

	input_slice_offset = node_on_which_the_slice_is * node_config.nodalSharedSpaceSize + node_config.ofsInp;
	input_slice_offset += slice_relative_on_node * nx_in * ny_in * sizeof(MKL_Complex8);
  }

  DLOG(DEBUG, "input starts at: " << node_config.ofsInp);
  DLOG(DEBUG, "input slice offset: " << input_slice_offset);

  fvm::util::local_allocation scratch(nx_in * ny_in * sizeof(MKL_Complex8));
  comm_handle = fvmGetGlobalData(
		  node_config.hndGlbVMspace
		, input_slice_offset
		, nx_in * ny_in * sizeof(MKL_Complex8)
		, 0 // shmem
		, scratch
  );
  comm_status = waitComm(comm_handle);
  assert(comm_status == COMM_HANDLE_OK);
  memcpy(input_slice, fvmGetShmemPtr(),  nx_in * ny_in * sizeof(MKL_Complex8));

  const std::size_t nx_out = re_global_struct.nx_out;
  const std::size_t ny_out = re_global_struct.ny_out;
  const std::size_t output_slice_size = nx_out * ny_out * sizeof(float);
  float *output_slice = new float[nx_out * ny_out];

  int retval = reCalcOneLevl(&node_config, slice, depth, input_slice, output_slice);

  {
	// write output slice back to temporary output volume
	memcpy(fvmGetShmemPtr(), output_slice, nx_out * ny_out * sizeof(float));
	comm_handle = fvmPutGlobalData(
		memhandle_for_temp_output_volume
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
	// write modified input slice back to input data!
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
    SDPA_ADD_INP( "memhandle_for_temp_output_volume_calc", fvmAllocHandle_t );
    SDPA_ADD_OUT( "slice_and_depth_OUT", unsigned long );
  SDPA_REGISTER_FUN_END(calc);
}
SDPA_MOD_INIT_END(calc)
