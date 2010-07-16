/***************************************************************************
                          boundaryoperator.h  -  description
                             -------------------                          */
/**
  The BoundaryOperator class provides the checking routines if a given
  point or ray is outside of the volume or aperture angle.
  *@author Dirk Merten
  */

/*                           -------------------                         

    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-06
                Aperture() changed to accuracy of float

 ***************************************************************************/


#ifndef BOUNDARYOPERATOR_H
#define BOUNDARYOPERATOR_H


#include "ray3d.h"
#include <math.h>
#include "structures/deg.h"

class BoundaryOperator {
public:
  BoundaryOperator(){};
  /// The boundary of the whole volume is initialized with two points
  /** The order of these points is not important. The minimum and
      maximum of all directions is used. */
  BoundaryOperator(const point3D<float>& _x0, const point3D<float>& _x1);
  ~BoundaryOperator(){};
  /// Checks if the ray is outside (true). 
  bool operator () (const ray3D& ray){return (*this)(ray.x);}
  /// Checks if the point is outside (true). 
  bool operator () (const point3D<float>& );

  /// Checks if the point is outside (true) of the aperture half angle above/below point Src.
  bool Aperture(const point3D<float>& pos, const point3D<float>& Src, const DEG& AperAng);

private: // Private attributes
  /// Corners of the volume cube, x0 < x1, etc.
  float x0, x1;
  float y0, y1;
  float z0, z1;
};

#endif
