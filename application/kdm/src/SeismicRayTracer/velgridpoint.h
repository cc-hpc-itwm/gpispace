/***************************************************************************
                          velgridpoint.h  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef VELGRIDPOINT_H
#define VELGRIDPOINT_H


/**
  *@author Dirk Merten
  */
#include "structures/point3d.h"
#include <iostream>

class VelGridPoint {
public:
  VelGridPoint();
  ~VelGridPoint();
  VelGridPoint(const point3D<float>& _P, const float& _vel);
  VelGridPoint (const VelGridPoint& );
public: // Private attributes
  /** Position of the point */
  //point3D<float> x;
  /** Scalar velocity at the point */
  float vel;
};

#endif
