/***************************************************************************
                          interface.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "interface.h"

Interface::Interface()
{
  Nx = 0; 
  Ny = 0;
  h = NULL;
  tri = NULL;
}
Interface::~Interface()
{
  if (h != NULL)
    {
      for (int ix = 0; ix < Nx; ix++)
        delete[] h[ix];
      delete[] h;
    }
  if (tri != NULL)
      delete[] tri;
}
/** No descriptions */
void Interface::init(const float& z)
{
  Nx = 100;
  Ny = 100;

  X0 = 0;
  Y0 = 0;

  dx = 40;
  dy = 40;

  h = new float*[Nx];
  for (int ix = 0; ix < Nx; ix++)
    {
      h[ix] = new float[Ny];
      for (int iy = 0; iy < Ny; iy++)
        h[ix][iy] = z - 1.0*iy*dy;
    }

  Ntri = 2*(Nx-1)*(Ny-1);
  tri = new triface[Ntri];
  int index = 0;
  for (int ix = 0; ix < Nx-1; ix++)
    {
      for (int iy = 0; iy < Ny-1; iy++)
        {
          tri[index].ix0 = ix;
          tri[index].iy0 = iy;
          tri[index].ix1 = ix;
          tri[index].iy1 = iy+1;
          tri[index].ix2 = ix+1;
          tri[index].iy2 = iy+1;

          tri[index].min = h[ix][iy];
          if (tri[index].min > h[ix][iy+1])
            tri[index].min = h[ix][iy+1];
          if (tri[index].min > h[ix+1][iy+1])
            tri[index].min = h[ix+1][iy+1];

          tri[index].max = h[ix][iy];
          if (tri[index].max < h[ix][iy+1])
            tri[index].max = h[ix][iy+1];
          if (tri[index].max < h[ix+1][iy+1])
            tri[index].max = h[ix+1][iy+1];

          const float x0 = X0 + ix * dx;
          const float y0 = Y0 + iy * dy;
          const float z0 = h[ix][iy];

          point3D<float> P0(x0,y0,z0);
                
          const float z1 = h[ix][iy+1];
          point3D<float> P1(x0, y0+dy, z1);
                
          const float z2 = h[ix+1][iy+1];
          point3D<float> P2(x0+dx, y0+dy, z2);

          tri[index].normal = (P1 - P0).VecProd(P2 - P0); 
          if ((tri[index].normal).norm() > 1.0e-6)
            tri[index].normal = tri[index].normal / ((tri[index].normal).norm());

          tri[index].d = P0 * tri[index].normal;

          index++;
          tri[index].ix0 = ix;
          tri[index].iy0 = iy;
          tri[index].ix1 = ix+1;
          tri[index].iy1 = iy;
          tri[index].ix2 = ix+1;
          tri[index].iy2 = iy+1;
 
         tri[index].min = h[ix][iy];
          if (tri[index].min > h[ix+1][iy])
            tri[index].min = h[ix+1][iy];
          if (tri[index].min > h[ix+1][iy+1])
            tri[index].min = h[ix+1][iy+1];

         tri[index].max = h[ix][iy];
          if (tri[index].max < h[ix+1][iy])
            tri[index].max = h[ix+1][iy];
          if (tri[index].max < h[ix+1][iy+1])
            tri[index].max = h[ix+1][iy+1];

          const float z3 = h[ix+1][iy];
          point3D<float> P3(x0+dx, y0, z3);

          tri[index].normal = (P3 - P0).VecProd(P2 - P0); 
          if ((tri[index].normal).norm() > 1.0e-6)
            tri[index].normal = tri[index].normal / (( tri[index].normal ).norm());

          tri[index].d = P0 * tri[index].normal;

          index++;
        }
    }

  min = z - 1.0*Ny*dy;
  max = z;

}

