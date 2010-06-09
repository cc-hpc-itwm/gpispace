#include <sdpa_writeoutvol2disk.hpp>

bool SDPA_writeoc2disk(int _oid,MigrationJob &_Job,ParallelEnvironment &_PE)
{

  const int VMRank=_PE.GetRank();

  // write offset class for a  given subvolume to disk
  char * pVMMemSubVol=(char *)_PE.getMemPtr()+_Job.SubVolMemOff;

  // Reconstruct the subvolume out of memory
  point3D<float> X0(_Job.MigVol.first_x_coord.v,
        	    _Job.MigVol.first_y_coord.v,
                    _Job.MigVol.first_z_coord);
  point3D<int>   Nx(_Job.MigVol.nx_xlines,
   		    _Job.MigVol.ny_inlines,
                    _Job.MigVol.nz);
  point3D<float> dx(_Job.MigVol.dx_between_xlines,
   	            _Job.MigVol.dy_between_inlines,
                    _Job.MigVol.dz);
  MigVol3D MigVol(X0,Nx,dx);

  grid3D G(X0,Nx,dx);

  // create the subvolume
  MigSubVol3D MigSubVol(MigVol,(VMRank%_Job.NSubVols)+1,_Job.NSubVols);

  MigrationFileHandler MFHandler;

  MFHandler.WriteOffsetClassForSubVol(_Job.MigFileName,G,(float *)pVMMemSubVol,
                                      MigSubVol.getix0(),MigSubVol.getNx(),MigSubVol.getiy0(),
                                      MigSubVol.getNy(),_oid-1,1,_Job.NtotOffVol,_Job.MigFileMode);

  return true;

}
