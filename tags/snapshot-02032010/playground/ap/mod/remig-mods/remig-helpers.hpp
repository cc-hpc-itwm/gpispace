#ifndef SDPA_REMIG_HELPERS_HPP
#define SDPA_REMIG_HELPERS_HPP 1

#include <fvm-pc/pc.hpp>
#include <ostream>
#include <remig/reGlbStructs.h>

namespace remig { namespace detail {
  inline void calculateDistribution(int number_of_frequencies, int *nwHlocal, int *nwHdispls)
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

  inline void fill_frequency_on_node_array(int number_of_frequencies, int *pIWonND, const int * const nwHlocal, const int * const nwHdispls)
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

  inline fvmOffset_t calculateRemigSliceOffset(const std::size_t slice
											 , const std::size_t size_of_slice
											 , const fvmSize_t memory_per_node
											 , const fvmOffset_t base_offset
											 , const int *pIWonND
											 , const int *nwHdispls)
  {
	fvmOffset_t offset(0);

	int node_on_which_the_slice_is = pIWonND[slice];
	int slice_relative_on_node = slice - nwHdispls[node_on_which_the_slice_is+1] + 1;

	offset = node_on_which_the_slice_is * memory_per_node + base_offset;
	offset += slice_relative_on_node * size_of_slice;
	return offset;
  }
}}

inline std::ostream &operator<<(std::ostream &os, const TReGlbStruct &cfg)
{
  os << "Image parameters:" << std::endl;
  os << "\tnz ......" << cfg.nz     << std::endl;
  os << "\tnx in ..." << cfg.nx_in  << std::endl;
  os << "\tny in...." << cfg.ny_in  << std::endl;
  os << "\tnx out..." << cfg.nx_out << std::endl;
  os << "\tny out..." << cfg.ny_out << std::endl;
  os << "\tnwH ....." << cfg.nwH    << std::endl;
  os << "\tnz ......" << cfg.nz     << std::endl;
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const cfg_t &cfg)
{
  os << "sizes:" << std::endl;
  os << "\tshared=" << cfg.nodalSharedSpaceSize
	 << " scratch=" << cfg.nodalScratchSize << std::endl;
  os << "handels:" << std::endl;
  os << "\tglobal=" << cfg.hndGlbVMspace << " scratch=" << cfg.hndScratch << std::endl;
  os << "offsets:" << std::endl;
  os << "\tglobal=" << cfg.ofsGlbDat << std::endl
	 << "\tofsInp=" << cfg.ofsInp << std::endl
	 << "\tofsVel=" << cfg.ofsVel << std::endl
	 << "\tofsOutp=" << cfg.ofsOutp << std::endl
	 << "\tofsVmin=" << cfg.ofsVmin << std::endl
	 << "\tofsVmax=" << cfg.ofsVmax << std::endl;
  os << "paths:" << std::endl
	 << "\tconfig=" << cfg.config_file << std::endl
	 << "\tdata=" << cfg.data_file << std::endl
	 << "\tvelocity=" << cfg.velocity_file << std::endl
	 << "\toutput=" << cfg.output_file << std::endl;
  return os;
}

#endif
