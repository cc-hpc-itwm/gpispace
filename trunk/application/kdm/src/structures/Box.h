/***************************************************************************
                          Box.h  -  description

    3-dimensional sub-volume of a 3-dimensional volume.

                             -------------------
    begin                : Fri Nov 11 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef BOX_H
#define BOX_H

/**
 *@author Dirk Merten
 */

#include <stdlib.h>

class Box {

// public methods
 public:
  Box()
      { 
	  /// Indicees of box in total TT volume
	  ix = 0; 
	  iy = 0;
	  iz = 0;

	  /// Distance in Indicees of neighboring boxes in total TT volume
	  idx = 0;
	  idy = 0;
	  idz = 0;

	  /// Number of boxes in total TT volume
	  Nix = 0;
	  Niy = 0;
	  Niz = 0;

	  /// Initial Coordinates of this box
	  x0 = 0;
	  y0 = 0;
	  z0 = 0;

	  /// line length of this Box
	  dx = 1;
	  dy = 1;
	  dz = 1;

	  /// Indicees of first point in total volume
	  ixp = 0;
	  iyp = 0;
	  izp = 0;

	  /// Number of points in this box
	  Nx = 0; 
	  Ny = 0;
	  Nz = 0;

	  /// Initial relative Coordinates of first point in this box
	  xp0 = 0;
	  yp0 = 0;
	  zp0 = 0;

	  /// Spacing of points in this Box
	  dxp = 1;
	  dyp = 1;
	  dzp = 1;
	  
	  active = false;

      };
  Box(const int _Nx, const int _Ny, const int _Nz, 
      const float _x0, const float _y0, const float _z0,
      const float _dx, const float _dy, const float _dz,
      const int _ix, const int _iy, const int _iz,
      const int _idx, const int _idy, const int _idz,
      const int _Nix, const int _Niy,const int _Niz,
      const float _xp0, const float _yp0, const float _zp0,      
      const float _dxp, const float _dyp, const float _dzp,
      const int _ixp, const int _iyp, const int _izp )
    {
      Nx = _Nx; Ny = _Ny; Nz = _Nz;
      x0 = _x0; y0 = _y0; z0 = _z0;
      dx = _dx; dy = _dy; dz = _dz;

      ix = _ix; iy = _iy; iz = _iz;
      idx = _idx; idy = _idy; idz = _idz;
      Nix = _Nix, Niy = _Niy; Niz = _Niz;
      xp0 = _xp0; yp0 = _yp0; zp0 = _zp0;
      dxp = _dxp; dyp = _dyp; dzp = _dzp;
      ixp = _ixp; iyp = _iyp; izp = _izp;
   }

  ~Box()
    {}

  bool isactive() const {return active;};
  float midx() const { return x0 + xp0 + (Nx/2) * dxp;};
  float midy() const { return y0 + yp0 + (Ny/2) * dyp;};
  float midz() const { return z0 + zp0 + (Nz/2) * dzp;};
  int midix() const { return Nx/2;};
  int midiy() const { return Ny/2;};
  int midiz() const { return Nz/2;};
  float x(const int& _ix) const { return x0 + xp0 + _ix * dxp;};
  float y(const int& _iy) const { return y0 + yp0 + _iy * dyp;};
  float z(const int& _iz) const { return z0 + zp0 + _iz * dzp;};


  
  /// Indicees of box in total TT volume
  int ix, iy, iz;
  /// Distance in Indicees of neighboring boxes in total TT volume
  int idx, idy, idz;
  /// Number of boxes in total TT volume
  int Nix, Niy, Niz;

  /// Initial Coordinates of this box
  float x0, y0, z0;
  /// line length of this Box
  float dx, dy, dz;

  /// Indicees of first point in total volume
  int ixp, iyp, izp;
  /// Number of points in this box
  int Nx, Ny, Nz;
  /// Initial relative Coordinates of first point in this box
  float xp0, yp0, zp0;
  /// Spacing of points in this Box
  float dxp, dyp, dzp;

  bool active;

// private methods
 private:

// private attributes
 private:

};
#endif
