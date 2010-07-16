/***************************************************************************
                          seisgrid3d.h  -  description

    3-dimensional grid defined by origin (x0, y0, z0), spacing (dx, dy, dy)
    and dimensions (Nx, Ny, Nz)

                             -------------------
    begin                : Tue Mar 10 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2009-08-03
               Grid defining variables changed to a more consistent scheme:
               - X0,NX,dx removed
               - MOD first_x_coord, first_y_coord added
               - float first_z_coord, dz, int nz added
               - nx_xlines, dx_between_xlines etc. changed names only.
 ***************************************************************************/


#ifndef SEISGRID3D_H
#define SEISGRID3D_H


/**
  *@author Dirk Merten
  */

#include "structures/mod.h"
#include "structures/point3d.h"

#ifdef __INTEL_COMPILER
#include <mathimf.h>
#endif

#include <math.h>

class seisgrid3D {

// public methods
public: 
    seisgrid3D():first_inline_num(0),first_xline_num(0),
	first_x_coord(0),first_y_coord(0),first_z_coord(0.0f),
	nx_xlines(1),ny_inlines(1),nz(1),
	dy_between_inlines(50.0f),dx_between_xlines(50.0f),dz(50.0f)
	{};
    ~seisgrid3D(){};

// public attributes
 public:
    int first_inline_num, first_xline_num;
    MOD first_x_coord, first_y_coord;
    float first_z_coord;
    int  nx_xlines, ny_inlines, nz;
    float dy_between_inlines, dx_between_xlines, dz;

// private methods
 private:

// private attributes
 private:
};

#endif
