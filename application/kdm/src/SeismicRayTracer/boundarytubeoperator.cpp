/***************************************************************************
                          boundarytubeoperator.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "boundarytubeoperator.h"

BoundaryTubeOperator::BoundaryTubeOperator(){
}
BoundaryTubeOperator::~BoundaryTubeOperator(){
}
BoundaryTubeOperator::BoundaryTubeOperator(const point3D<float>& _x0, const point3D<float>& _x1,
 const point3D<float>& _P0, const point3D<float>& _P1, const float& _r)
{
  x0 = std::min(_x0[0], _x1[0]);
  y0 = std::min(_x0[1], _x1[1]);
  z0 = std::min(_x0[2], _x1[2]);
  x1 = std::max(_x0[0], _x1[0]);
  y1 = std::max(_x0[1], _x1[1]);
  z1 = std::max(_x0[2], _x1[2]);

  P0 = _P0;
  P1 = _P1;
  r_r = _r*_r;

  N = P1-P0;
  N = N/sqrt(N*N);
}
/** No descriptions */
bool BoundaryTubeOperator::operator () (const point3D<float>& x){
  if ( (x[0] < x0) || (x[0] >= x1) ||
       (x[1] < y0) || (x[1] >= y1) ||
       (x[2] < z0) || (x[2] >= z1) )

       return true;
  else
  {
    point3D<float> D = (x-P0) - N*((x-P0)*N);
    if ( (D*D) > r_r)
      return true;
  }

  return false;
}
