#include <sdpa_initsubvol.hpp>

bool SDPA_initsubvol(MigrationJob &_Job,ParallelEnvironment &PE)
{
  const int VMRank=PE.GetRank();

  // initialize subvol to 0.
  char * pVMMemSubVol=(char *)PE.getMemPtr()+_Job.SubVolMemOff;

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

  // create the subvolume
  MigSubVol3D MigSubVol(MigVol,( VMRank % _Job.NSubVols ) + 1,_Job.NSubVols);

  MigSubVol.setMemPtr( (float *)pVMMemSubVol,_Job.SubVolMemSize);

  // now, initialize the subvol
  MigSubVol.clear();

  return true;

}