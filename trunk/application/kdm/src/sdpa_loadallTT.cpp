#include <sdpa_loadallTT.hpp>

bool SDPA_loadallTT(MigrationJob & _Job, ParallelEnvironment &_PE, int NThreads)
{

  _PE.Barrier();

  TTVMMemHandler TTVMMem;
  grid2D GSrc;
  grid3D GVol;

  // Create the 2-D surface grid description
  double x0Srfc = _Job.SrfcGridX0.v;
  double y0Srfc = _Job.SrfcGridY0.v;

  double dxSrfc = _Job.SrfcGriddx;
  double dySrfc = _Job.SrfcGriddy;

  int NxSrc = _Job.SrfcGridNx;
  int NySrc = _Job.SrfcGridNy;

  GSrc.Init(x0Srfc, y0Srfc, NxSrc, NySrc, dxSrfc, dySrfc);

  // Create the 3-D subsurface grid description
  int Nx = _Job.TTVol.nx_xlines;
  int Ny = _Job.TTVol.ny_inlines;
  int Nz = _Job.TTVol.nz;

  point3D<float> X0, dx;
  X0[0] = _Job.TTVol.first_x_coord.v;
  X0[1] = _Job.TTVol.first_y_coord.v;
  X0[2] = _Job.TTVol.first_z_coord;
  dx[0] = _Job.TTVol.dx_between_xlines;
  dx[1] = _Job.TTVol.dy_between_inlines;
  dx[2] = _Job.TTVol.dz;

  GVol.Init(X0, point3D<int>(Nx,Ny,Nz), dx);

  // Load the entire travel time table data into memory
  TTVMMem.InitVol(_Job,_Job.RTFileName,GSrc,GVol,&_PE,NThreads,0);

  return true;
}
