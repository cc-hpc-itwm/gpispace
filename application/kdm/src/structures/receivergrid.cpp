/***************************************************************************
                          receivergrid.cpp  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "receivergrid.h"

ReceiverGrid::ReceiverGrid(){
}
ReceiverGrid::~ReceiverGrid(){
}
ReceiverGrid::ReceiverGrid(const grid3D& _G):
G(_G)
{
  RcvGrid.Init(G.getNx(), G.getNy(), G.getNz());

  for (int i = 0; i < G.getNx(); i++)
    for (int j = 0; j < G.getNy(); j++)
      for (int k = 0; k < G.getNz(); k++)
      {
        RcvGrid.InitPoint(i, j, k, Receiver(G.GetCoord(i, j, k)));
      }
}

ReceiverGrid::ReceiverGrid(const point3D<float>& X0Rcv, const point3D<int>& NRcv, const point3D<float>& dxRcv):
G(X0Rcv, NRcv, dxRcv)
{
  RcvGrid.Init(G.getNx(), G.getNy(), G.getNz());

  for (int i = 0; i < G.getNx(); i++)
    for (int j = 0; j < G.getNy(); j++)
      for (int k = 0; k < G.getNz(); k++)
      {
        RcvGrid.InitPoint(i, j, k, Receiver(G.GetCoord(i, j, k)));
      }
}

Receiver* ReceiverGrid::GetPointAt(const int& ix, const int& iy, const int& iz) const{
  return RcvGrid.GetPointAt(ix, iy, iz);
}


ReceiverGrid::iterator ReceiverGrid::begin() const
{
  const int Nx = G.getNx();
  const int Ny = G.getNy();
  const int Nz = G.getNz();

  return iterator(&RcvGrid, 0, 0, 0, Nx-1, Ny-1, Nz-1);
}

ReceiverGrid::iterator ReceiverGrid::begin(const Triangle* Tri) const
{
  const point3D<float>& X1_o = Tri->points[1]->mainray.x_old;
  const point3D<float>& X2_o = Tri->points[2]->mainray.x_old;

  const point3D<float>& X0_n = Tri->points[0]->mainray.x;
  const point3D<float>& X1_n = Tri->points[1]->mainray.x;
  const point3D<float>& X2_n = Tri->points[2]->mainray.x;

  point3D<float> Xmax(Tri->points[0]->mainray.x_old);
  point3D<float> Xmin(Xmax);

  for (int i = 0; i < 3; i++)
  {
    if ( X1_o[i] > Xmax[i])
      Xmax[i] = X1_o[i];
    else
      if ( X1_o[i] < Xmin[i])
        Xmin[i] = X1_o[i];

    if ( X2_o[i] > Xmax[i])
      Xmax[i] = X2_o[i];
    else
      if ( X2_o[i] < Xmin[i])
        Xmin[i] = X2_o[i];

    if ( X0_n[i] > Xmax[i])
      Xmax[i] = X0_n[i];
    else
      if ( X0_n[i] < Xmin[i])
        Xmin[i] = X0_n[i];

    if ( X1_n[i] > Xmax[i])
      Xmax[i] = X1_n[i];
    else
      if ( X1_n[i] < Xmin[i])
        Xmin[i] = X1_n[i];

    if ( X2_n[i] > Xmax[i])
      Xmax[i] = X2_n[i];
    else
      if ( X2_n[i] < Xmin[i])
        Xmin[i] = X2_n[i];
  }

  return begin(Xmin, Xmax);
}

ReceiverGrid::iterator ReceiverGrid::begin(const point3D<float>& X0, const point3D<float>& X1) const
{
  int ijk_b[3];
  G.GetLowerIndex(X0, ijk_b);

  ijk_b[0] = (ijk_b[0]<0)?-1:ijk_b[0];
  ijk_b[1] = (ijk_b[1]<0)?-1:ijk_b[1];
  ijk_b[2] = (ijk_b[2]<0)?-1:ijk_b[2];

  int ijk_e[3];
  G.GetLowerIndex(X1, ijk_e);

  const int Nx = G.getNx();
  const int Ny = G.getNy();
  const int Nz = G.getNz();

  ijk_e[0] = (ijk_e[0]>=Nx)?(Nx-1):ijk_e[0];
  ijk_e[1] = (ijk_e[1]>=Ny)?(Ny-1):ijk_e[1];
  ijk_e[2] = (ijk_e[2]>=Nz)?(Nz-1):ijk_e[2];

//  if ( (X0[0] <= 185) && (X1[0] >= 185)
//       && (X0[1] <= 235) && (X1[1] >= 235)
//       && (X0[2] <= -2000) && (X1[2] >= -2000) )
//  {
//    std::cout << "ReceiverGrid::begin : X0 = " << X0 << ", X1 = " << X1 << std::endl;
//    std::cout << "          ---> ijk_b = " << ijk_b[0] << ijk_b[1] << ijk_b[2];
//    std::cout << "   ijk_e = " << ijk_e[0] << ijk_e[1] << ijk_e[2] << std::endl;
//    std::cout << "with NxNyNz = " << Nx << Ny << Nz << std::endl;
//  }  

  return iterator(&RcvGrid, ijk_b[0]+1, ijk_b[1]+1, ijk_b[2]+1, ijk_e[0]+1, ijk_e[1]+1, ijk_e[2]+1);
}

void ReceiverGrid::clear()
{
  const int Nx = G.getNx();
  const int Ny = G.getNy();
  const int Nz = G.getNz();

  for (int i = 0; i < Nx; i++)
    for (int j = 0; j < Ny; j++)
      for (int k = 0; k < Nz; k++)
      {
        Receiver* Rcv = RcvGrid.GetPointAt(i,j,k);
        Rcv->clear();
      }
}
