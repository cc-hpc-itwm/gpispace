/***************************************************************************
                          reflectionoperator.h  -  description
                             -------------------                          */
/**
  The ReflectionOperator class provides the checking routines if a given
  point or ray has passed an interface and calculates the new ray parameters.
  *@author Dirk Merten
  */

/*                           -------------------                         

    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef REFLECTIONOPERATOR_H
#define REFLECTIONOPERATOR_H


#include "utils/linalgorithms.h"
#include "ray3d.h"
#include "interface.h"
#include <math.h>

class ReflectionOperator {
public:
  ReflectionOperator();
  ~ReflectionOperator();
  /// Checks if the ray has passed an interface (true) and returns the distance
  /// of the old ray position to the interface in l. 
  bool operator () (const ray3D* ray, float& l);

  /// Calculate the parameters of the reflected ray;
  void reflect(ray3D* ray);

private: // Private attributes
  /// Number of interfaces in the model.
  int NIfcs;
  /// Array of Interfaces
  Interface* Ifcs;
  /// normal of the triangle at work.
  point3D<float> normal;
  /// Distance of triangle.
  float d;
  bool Intersection(const point3D<float>& P0, const point3D<float>& P1, const point3D<float>& P2, 
                    const point3D<float>& x0, const point3D<float>& x1, float& l);
};

#endif
