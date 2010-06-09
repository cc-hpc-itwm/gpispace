/***************************************************************************
                          boundaryoperator.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "boundaryoperator.h"

BoundaryOperator::BoundaryOperator(const point3D<float>& _x0, const point3D<float>& _x1)
{
  x0 = std::min(_x0[0], _x1[0]);
  y0 = std::min(_x0[1], _x1[1]);
  z0 = std::min(_x0[2], _x1[2]);
  x1 = std::max(_x0[0], _x1[0]);
  y1 = std::max(_x0[1], _x1[1]);
  z1 = std::max(_x0[2], _x1[2]);
}
/** No descriptions */
bool BoundaryOperator::operator () (const point3D<float>& x)
{
  if ( (x[0] < x0) || (x[0] > x1) ||
       (x[1] < y0) || (x[1] > y1) ||
       (x[2] < z0) || (x[2] > z1) )
  {
       return true;
  }     
  else
  {
//    std::cout << "Boundary = false\n";
    return false;
  }
}

bool BoundaryOperator::Aperture(const point3D<float>& pos, const point3D<float>& Src, const DEG& AperAng)
{
   const float d = (Src[2]-pos[2]) * tanf(deg2rad(AperAng));
   const float d_2 = d*d;
   return ( ((Src[0]-pos[0])*(Src[0]-pos[0]) + (Src[1]-pos[1])*(Src[1]-pos[1])) > d_2 );
}
