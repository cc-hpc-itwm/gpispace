/***************************************************************************
                          grid2d.h  -  description

    2-dimensional grid defined by origin (x0, y0), spacing (dx, dy)
    and dimensions (Nx, Ny)

                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef GRID2D_H
#define GRID2D_H


/**
  *@author Dirk Merten
  */

#include <math.h>
#include <iostream>

class grid2D {

// public methods
public: 
    grid2D();
    grid2D(const double& _x0, const double& _y0, const int& _Nx, const int& _Ny, const double& _dx, const double& _dy);
    ~grid2D(){};
    /** Initialization */
    void Init(const double& _x0, const double& _y0, const int& _Nx, const int& _Ny, const double& _dx, const double& _dy); 
    /** getter for Nx */
    const int& getNx() const {return Nx;};
    /** getter for Ny */
    const int& getNy() const {return Ny;};
    /** getter for x0 */
    const double& getx0() const {return x0;};
    /** getter for y0 */
    const double& gety0() const {return y0;};
    /** getter for dx() */
    const double& getdx() const {return dx;};
    /** getter for dy() */
    const double& getdy() const {return dy;};
    /** Return indicees of lower grid point for given coordinates */
    int GetLowerIndex(const double& x, const double& y, int& i, int& j) const;
    /** Return indicees of closest grid point for given coordinates */
    void GetCoord(const int& i, const int& j, double& x, double& y) const;


// public attributes
 public:

// private methods
 private:

// private attributes
 private:
/** Number of grid points */
  int Nx, Ny;
/** Origin coordinates */
  double x0, y0;
/** Grid spacing */
  double dx, dy;
};

#endif
