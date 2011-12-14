/***************************************************************************
                          grid3d.cpp  -  description
                             -------------------
    begin                : Tue Feb 21 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "grid3d.h"

grid3D::grid3D(){
}
grid3D::grid3D(const point3D<float>& _X0, const point3D<int>& _N, const point3D<float>& _dx)
{
  Init(_X0, _N, _dx);
}

void grid3D::Init(const point3D<float>& _X0, const point3D<int>& _N, const point3D<float>& _dx)
{
#ifdef DEBUG
    if ( fabs(_dx[0]) < 1e-4 )
	 std::cout << "grid3D::Init: _dx[0] = " << _dx[0] << " is too small.\n";
    if ( fabs(_dx[1]) < 1e-4 )
	 std::cout << "grid3D::Init: _dx[1] = " << _dx[1] << " is too small.\n";
    if ( fabs(_dx[2]) < 1e-4 )
	 std::cout << "grid3D::Init: _dx[2] = " << _dx[2] << " is too small.\n";
#endif

  x0 = _X0[0]; y0 = _X0[1]; z0 = _X0[2];

  dx = _dx[0]; dy = _dx[1]; dz = _dx[2];

  dx_inv = 1.0/dx; dy_inv = 1.0/dy; dz_inv = 1.0/dz;
  
  Nx = _N[0]; Ny = _N[1]; Nz = _N[2];

#ifdef DEBUG
  if ( (Nx < 0) || (Ny < 0) || (Nz < 0) )
    std::cout << "grid3D::grid3D: Illegal dimensions " << Nx << " " << Ny << " " << Nz << std::endl;
#endif
}

void grid3D::GetLowerIndex(const point3D<float>& x, int* ijk) const
{
  ijk[0] = (int) floorf ((x[0]-x0)*dx_inv);
  ijk[1] = (int) floorf ((x[1]-y0)*dy_inv);
  ijk[2] = (int) floorf ((x[2]-z0)*dz_inv);

#ifdef DEBUG
  if ((ijk[0] < 0) || (ijk[0] >= Nx))
    std::cout << "grid3D: Index out of range: ijk[0] = " << ijk[0]
              << " for x = " << x[0] << ", x0 = " << x0 << ", dx = " << dx << std::endl;
  if ((ijk[1] < 0) || (ijk[1] >= Ny))
    std::cout << "grid3D: Index out of range: ijk[1] = " << ijk[1]
              << " for y = " << x[1] << ", y0 = " << y0 << ", dy = " << dy << std::endl;
  if ((ijk[2] < 0) || (ijk[2] >= Nz))
    std::cout << "grid3D: Index out of range: ijk[2] = " << ijk[2]
              << " for z = " << x[2] << ", z0 = " << z0 << ", dz = " << dz << std::endl;
#endif
}

void grid3D::GetIndex(const point3D<float>& x, int* ijk) const
{
  ijk[0] = (int) ((x[0]-x0)*dx_inv + 0.5);
  ijk[1] = (int) ((x[1]-y0)*dy_inv + 0.5);
  ijk[2] = (int) ((x[2]-z0)*dz_inv + 0.5);

}
