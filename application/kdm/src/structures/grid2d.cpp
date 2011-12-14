/***************************************************************************
                          grid2d.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "grid2d.h"

grid2D::grid2D(){
}
grid2D::grid2D(const double& _x0, const double& _y0, const int& _Nx, const int& _Ny, const double& _dx, const double& _dy)
:x0(_x0),y0(_y0),Nx(_Nx),Ny(_Ny),dx(_dx),dy(_dy)
{}

void grid2D::Init(const double& _x0, const double& _y0, const int& _Nx, const int& _Ny, const double& _dx, const double& _dy)
{
  x0 = _x0;
  y0 = _y0;
  dx = _dx;
  dy = _dy;
  Nx = _Nx;
  Ny = _Ny;
}

int grid2D::GetLowerIndex(const double& x, const double& y, int& i, int& j) const
{
  i = static_cast<int>( (x-x0)/dx );
  j = static_cast<int>( (y-y0)/dy );

#ifdef DEBUG
  if ((i < 0) || (i >= Nx))
    std::cerr << "grid2D: Index out of range: i = " << i
              << " for x = " << x << ", x0 = " << x0 << ", dx = " << dx << std::endl;
  if ((j < 0) || (j >= Ny))
    std::cerr << "grid2D: Index out of range: j = " << j
              << " for y = " << y << ", y0 = " << y0 << ", dy = " << dy << std::endl;
#endif

  return 0;
}

void grid2D::GetCoord(const int& i, const int& j, double& x, double& y) const
{
    x = x0 + i * dx;
    y = y0 + j * dy;
}
