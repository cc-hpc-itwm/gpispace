/***************************************************************************
                             interface.h  -  description
                             -------------------                          */
/**
  The Interface class describes a parametrized interface.
  *@author Dirk Merten
  */

/*                           -------------------                         

    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef INTERFACE_H
#define INTERFACE_H


#include "utils/linalgorithms.h"
#include "triface.h"
#include <cstddef>
#include <math.h>

class Interface {
public:
  Interface();
  ~Interface();
  /// Checks if the ray has passed an interface (true) and returns the distance
  /// of the old ray position to the interface in l. 
  void init (const float& z);

  /// List of trifaces
  triface* tri;
  /// Number of trifaces
  int Ntri;
  /// Array of values.
  float** h;
  /// Number of root points.
  int Nx, Ny;
  /// Corner of parametrization.
  float X0, Y0;
  /// Spacing of parametrization.
  float dx, dy;
  /// Minimal and maximal value.
  float min, max;
};

#endif
