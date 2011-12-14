/***************************************************************************
                          boundarytubeoperator.h  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef BOUNDARYTUBEOPERATOR_H
#define BOUNDARYTUBEOPERATOR_H


/**
  *@author Dirk Merten
  */
#include "ray3d.h"
#include <math.h>

class BoundaryTubeOperator {
public:
    BoundaryTubeOperator();
    BoundaryTubeOperator(const point3D<float>& _x0, const point3D<float>& _x1, const point3D<float>& _P0, const point3D<float>& _P1, const float& _r);
  /** No descriptions */
  bool operator () (const ray3D& ray){return (*this)(ray.x);}
  /** No descriptions */
  bool operator () (const point3D<float>& );

  bool reflect(ray3D& ray);
	~BoundaryTubeOperator();
private: // Private attributes
  /**  */
  float x0, x1;
  float y0, y1;
  float z0, z1;
  point3D<float> P0, P1, N;
  float r_r;
};

#endif
