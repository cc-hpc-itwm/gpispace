/***************************************************************************
                          grid3d.h  -  description

    3-dimensional grid defined by origin (x0, y0, z0), spacing (dx, dy, dy)
    and dimensions (Nx, Ny, Nz)

                             -------------------
    begin                : Tue Feb 21 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef GRID3D_H
#define GRID3D_H


/**
  *@author Dirk Merten
  */

#include "point3d.h"

#ifdef __INTEL_COMPILER
#include <mathimf.h>
#endif

#include <math.h>

class grid3D {

// public methods
public: 
    grid3D();
    grid3D(const point3D<float>& _X0, const point3D<int>& _N, const point3D<float>& _dx);
    ~grid3D(){};
    /** Initialization */
    void Init(const point3D<float>& _X0, const point3D<int>& _N, const point3D<float>& _dx);
    /** getter for x0 */
    const float& getx0() const {return x0;};
    /** getter for y0 */
    const float& gety0() const {return y0;};
    /** getter for z0 */
    const float& getz0() const {return z0;};
    /** getter for Nx */
    const int& getNx() const {return Nx;};
    /** getter for Ny */
    const int& getNy() const {return Ny;};
    /** getter for Nz */
    const int& getNz() const {return Nz;};
    /** getter for dx */
    const float& getdx() const {return dx;};
    /** getter for dy */
    const float& getdy() const {return dy;};
    /** getter for dz */
    const float& getdz() const {return dz;};
    /** getter for inverse of dx */
    const float& getdx_inv() const {return dx_inv;};
    /** getter for inverse of dy */
    const float& getdy_inv() const {return dy_inv;};
    /** getter for inverse of dz */
    const float& getdz_inv() const {return dz_inv;};
    /** return indicees ijk for lower grid point for given coordinates */
    void GetLowerIndex(const point3D<float>& x, int* ijk) const;
    /** return indicees ijk for closest grid point for given coordinates */
    void GetIndex(const point3D<float>& x, int* ijk) const;
    /** return the point coordinates in x for given indicees ix, iy, iz */
    point3D<float> GetCoord(const int& ix, const int& iy, const int& iz) const
	{
	    return point3D<float> ( x0 + ix * dx, y0 + iy * dy, z0 + iz * dz);
	}

// public attributes
 public:

// private methods
 private:

// private attributes
 private:
    /** Position of one corner of the volume in space. */
    float x0, y0, z0;
    /**  Distance of successive grid points. Constant grid size per direction is assumed. */
    float dx, dy, dz;
    /**  Inverted Distances of successive grid points.*/
    float dx_inv, dy_inv, dz_inv;
    /** Number of grid points in each direction. */
    int Nx, Ny, Nz;
};

#endif
