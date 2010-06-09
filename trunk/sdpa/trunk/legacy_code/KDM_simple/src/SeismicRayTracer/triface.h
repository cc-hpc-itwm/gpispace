/***************************************************************************
                             triface.h  -  description
                             -------------------                          */
/**
  The Triface class describes a parametrized triface.
  *@author Dirk Merten
  */

/*                           -------------------                         

    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef TRIFACE_H
#define TRIFACE_H


#include "structures/point3d.h"
#include <cstddef>
#include <math.h>

class triface {
public:
  triface(){};
  ~triface(){};
  /// Indicees of corners.
  int ix0, iy0;
  int ix1, iy1;
  int ix2, iy2;
  /// Normal of triface.
  point3D<float> normal;
  /// Distance of triface.
  float d;
  /// Minimal and maximal value.
  float min, max;
};

#endif
