/***************************************************************************
                          MigVol.hpp  -  description
*/

/**
    Doxygen Style
    Multi-Line Documentation
**/

/*                           -------------------
    begin                :
    copyright            : (C) 2010 by Daniel Grünewald
    email                : Daniel.Gruenewald@itwm.fraunhofer.de
 ***************************************************************************/


#ifndef MIGVOL_H
#define MIGVOL_H

// user defined includes
#include <./structures/grid3d.h>
#include <./include/defs.h>

// standard includes
#include <iostream>

/**
  *@author Daniel Grünewald doxygen stale
  */

class MigVol3D : public grid3D
{

// public methods
 public:

  // construct the migration volume:
  MigVol3D();

  // _X0 is the left corner of the migration volume
  // _N  is the number of lattice sites along the three directions
  // _dx corresponds to the physical spacing between two lattice sites
  //     along the three directions
  MigVol3D(const point3D<float>& _x0, const point3D<int>& _N, const point3D<float>& _dx);

  // Destruct the migration volume:
  ~MigVol3D();

  void InitVol(float * _pVolMemPtr);

  // Reinitialize the volume
  void clear();
  // get Memory pointer to the output volume
  float * getMemPtr();

  // migrate the given trace to the subvolume using the given traveltime information
  void migrate();

// public attributes
 public:

// private methods
 private:

    MigVol3D(const MigVol3D& MigVol);

    MigVol3D& operator = (const MigVol3D& SubVol);

// private attributes
 private:
  float * pVolMem;	// Volume

};

#endif
