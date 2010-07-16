/***************************************************************************
                          BlockVolume.h  -  description

    Block partitioning of a 3-dimensional volume.

                             -------------------
    begin                : Thi Jan 30 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2009-08-04
               Deprecated initialization routine has been removed.
      merten | 2009-08-03
               Adapted to modified seisgrid3D class internal structure
 ***************************************************************************/


#ifndef BLOCKVOLUME_H
#define BLOCKVOLUME_H


/**
 *@author Dirk Merten
 */

#include "include/defs.h"
#include "structures/seisgrid3d.h"
#include "structures/migrationjob.h"
#include "utils/Acq_geometry.hpp"
#include "Box.h"

#include <stdlib.h>



class BlockVolume {

// public methods
 public:
  BlockVolume()
    {
      Blocks = NULL;
    }

  BlockVolume(const seisgrid3D& MigVol, const seisgrid3D& TTVol, int& ierr)
    {
	Blocks = NULL;
	ierr = 0;
	
	// Output volume dimensions
	x0 = MigVol.first_x_coord.v;
	y0 = MigVol.first_y_coord.v;
	z0 = MigVol.first_z_coord;
	  
	dx = MigVol.dx_between_xlines;
	dy = MigVol.dy_between_inlines;
	dz = MigVol.dz;


	// TT table dimensions
	float XVol0 = TTVol.first_x_coord.v;
	float YVol0 = TTVol.first_y_coord.v;
	float ZVol0 = TTVol.first_z_coord;
	float dxVol = TTVol.dx_between_xlines;
	float dyVol = TTVol.dy_between_inlines;
	float dzVol = TTVol.dz;
	int NVolx = TTVol.nx_xlines;
	int NVoly = TTVol.ny_inlines;
	int NVolz = TTVol.nz;

	// Determine number of boxes in x, y, z

	int Boxminindex_x;
	int Boxminindex_y;
	int Boxminindex_z;
	determine_Box_index(MigVol, TTVol, 0, 0, 0, Boxminindex_x, Boxminindex_y , Boxminindex_z);

	int Boxmaxindex_x = Boxminindex_x;
	int Boxmaxindex_y = Boxminindex_y;
	int Boxmaxindex_z = Boxminindex_z;

	// Loop over output points along xline
	for (int ix = 0; ix < MigVol.nx_xlines; ix++)
	{
	    // determine Box xindex
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, ix, 0, 0, ibx, iby, ibz);

	    // set to minimal index
	    Boxminindex_x = (ibx<Boxminindex_x)?ibx:Boxminindex_x;
	    // set to maximal index
	    Boxmaxindex_x = (ibx>Boxmaxindex_x)?ibx:Boxmaxindex_x;
	}

	// Loop over output points along xline
	for (int iy = 0; iy < MigVol.ny_inlines; iy++)
	{
	    // determine Box xindex
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, 0, iy, 0, ibx, iby, ibz);

	    // set to minimal index
	    Boxminindex_y = (iby<Boxminindex_y)?iby:Boxminindex_y;
	    // set to maximal index
	    Boxmaxindex_y = (iby>Boxmaxindex_y)?iby:Boxmaxindex_y;
	}

	// Loop over output points along xline
	for (int iz = 0; iz < MigVol.nz; iz++)
	{
	    // determine Box xindex
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, 0, 0, iz, ibx, iby, ibz);

	    // set to minimal index
	    Boxminindex_z = (ibz<Boxminindex_z)?ibz:Boxminindex_z;
	    // set to maximal index
	    Boxmaxindex_z = (ibz>Boxmaxindex_z)?ibz:Boxmaxindex_z;
	}

	//Allocate Box Memory and set general values
	NBx = Boxmaxindex_x - Boxminindex_x + 1;
	NBy = Boxmaxindex_y - Boxminindex_y + 1;
	NBz = Boxmaxindex_z - Boxminindex_z + 1;

	if (NBx * NBy * NBz == 0)
	{
	    
	    ierr = -1;
	    return;
	}

	

	Blocks = new Box***[NBx];
	for (int ix = 0; ix < NBx; ix++)
	{
	    Blocks[ix] = new Box**[NBy];
	    for (int iy = 0; iy < NBy; iy++)
	    {
		Blocks[ix][iy] = new Box*[NBz];
		for (int iz = 0; iz < NBz; iz++)
		{
		    Blocks[ix][iy][iz] = new Box();
		    
		    Blocks[ix][iy][iz]->ix = Boxminindex_x + ix;
		    Blocks[ix][iy][iz]->iy = Boxminindex_y + iy;
		    Blocks[ix][iy][iz]->iz = Boxminindex_z + iz;

		    Blocks[ix][iy][iz]->Nix = NVolx;
		    Blocks[ix][iy][iz]->Niy = NVoly;
		    Blocks[ix][iy][iz]->Niz = NVolz;

		    Blocks[ix][iy][iz]->x0 = XVol0 + Blocks[ix][iy][iz]->ix*dxVol;
		    Blocks[ix][iy][iz]->y0 = YVol0 + Blocks[ix][iy][iz]->iy*dyVol;
		    Blocks[ix][iy][iz]->z0 = ZVol0 + Blocks[ix][iy][iz]->iz*dzVol;

		    Blocks[ix][iy][iz]->dx = dxVol;
		    Blocks[ix][iy][iz]->dy = dyVol;
		    Blocks[ix][iy][iz]->dz = dzVol;

		}
	    }
	}

	//Loop over points in x
	for (int ix = 0; ix < MigVol.nx_xlines; ix++)
	{
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, ix, 0, 0, ibx, iby, ibz);

	    ibx = ibx - Boxminindex_x;

	    Blocks[ibx][0][0]->Nx++;

	    // first point ?
	    if (Blocks[ibx][0][0]->Nx == 1)
	    {
		Blocks[ibx][0][0]->ixp = ix;
		Blocks[ibx][0][0]->xp0 = x0 + ix * dx - Blocks[ibx][0][0]->x0;
		Blocks[ibx][0][0]->dxp = dx;
	    }


	    Blocks[ibx][0][0]->idx = (fabs(x0 + ix * dx - Blocks[ibx][0][0]->x0) < 0.25f)?0:1;
	}
	  
	// Copy x along y and z
	for (int ibx = 0; ibx < NBx; ibx++)
	    for (int iby = 0; iby < NBy; iby++)
		for (int ibz = 0; ibz < NBz; ibz++)
		{
		    Blocks[ibx][iby][ibz]->Nx = Blocks[ibx][0][0]->Nx;
		    Blocks[ibx][iby][ibz]->ixp = Blocks[ibx][0][0]->ixp;
		    Blocks[ibx][iby][ibz]->xp0 = Blocks[ibx][0][0]->xp0;
		    Blocks[ibx][iby][ibz]->dxp = Blocks[ibx][0][0]->dxp;
		    Blocks[ibx][iby][ibz]->idx = Blocks[ibx][0][0]->idx;
		}
	  

	//Loop over points in y
	for (int iy = 0; iy < MigVol.ny_inlines; iy++)
	{
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, 0, iy, 0, ibx, iby, ibz);

	    iby = iby - Boxminindex_y;

	    Blocks[0][iby][0]->Ny++;

	    // first point ?
	    if (Blocks[0][iby][0]->Ny == 1)
	    {
		Blocks[0][iby][0]->iyp = iy;
		Blocks[0][iby][0]->yp0 = y0 + iy * dy - Blocks[0][iby][0]->y0;
		Blocks[0][iby][0]->dyp = dy;
	    }


	    Blocks[0][iby][0]->idy = (fabs(y0 + iy * dy - Blocks[0][iby][0]->y0) < 0.25f)?0:1;
	}
	  
	// Copy y along x and z
	for (int ibx = 0; ibx < NBx; ibx++)
	    for (int iby = 0; iby < NBy; iby++)
		for (int ibz = 0; ibz < NBz; ibz++)
		{
		    Blocks[ibx][iby][ibz]->Ny = Blocks[0][iby][0]->Ny;
		    Blocks[ibx][iby][ibz]->iyp = Blocks[0][iby][0]->iyp;
		    Blocks[ibx][iby][ibz]->yp0 = Blocks[0][iby][0]->yp0;
		    Blocks[ibx][iby][ibz]->dyp = Blocks[0][iby][0]->dyp;
		    Blocks[ibx][iby][ibz]->idy = Blocks[0][iby][0]->idy;
		}
	  

	//Loop over points in z
	for (int iz = 0; iz < MigVol.nz; iz++)
	{
	    int ibx, iby, ibz;
	    determine_Box_index(MigVol, TTVol, 0, 0, iz, ibx, iby, ibz);

	    ibz = ibz - Boxminindex_z;

	    Blocks[0][0][ibz]->Nz++;

	    // first point ?
	    if (Blocks[0][0][ibz]->Nz == 1)
	    {
		Blocks[0][0][ibz]->izp = iz;
		Blocks[0][0][ibz]->zp0 = z0 + iz * dz - Blocks[0][0][ibz]->z0;
		Blocks[0][0][ibz]->dzp = dz;
	    }


	    Blocks[0][0][ibz]->idz = (fabs(z0 + iz * dz - Blocks[0][0][ibz]->z0) < 0.25f)?0:1;
	}
	  
	// Copy z along x and y
	for (int ibx = 0; ibx < NBx; ibx++)
	    for (int iby = 0; iby < NBy; iby++)
		for (int ibz = 0; ibz < NBz; ibz++)
		{
		    Blocks[ibx][iby][ibz]->Nz = Blocks[0][0][ibz]->Nz;
		    Blocks[ibx][iby][ibz]->izp = Blocks[0][0][ibz]->izp;
		    Blocks[ibx][iby][ibz]->zp0 = Blocks[0][0][ibz]->zp0;
		    Blocks[ibx][iby][ibz]->dzp = Blocks[0][0][ibz]->dzp;
		    Blocks[ibx][iby][ibz]->idz = Blocks[0][0][ibz]->idz;
		}

	// determine maximum number of points in a block and activity of block
        bool totallycovered = true;
	bool partiallycovered = false;

	nx = 0; ny = 0; nz = 0;
	for (int ibx = 0; ibx < NBx; ibx++)
	    for (int iby = 0; iby < NBy; iby++)
		for (int ibz = 0; ibz < NBz; ibz++)
		{
		    nx = (Blocks[ibx][iby][ibz]->Nx > nx)?Blocks[ibx][iby][ibz]->Nx:nx;
		    ny = (Blocks[ibx][iby][ibz]->Ny > ny)?Blocks[ibx][iby][ibz]->Ny:ny;
		    nz = (Blocks[ibx][iby][ibz]->Nz > nz)?Blocks[ibx][iby][ibz]->Nz:nz;

		    if ( (Blocks[ibx][iby][ibz]->ix < 0) || (Blocks[ibx][iby][ibz]->ix + Blocks[ibx][iby][ibz]->idx >= TTVol.nx_xlines)
			 || (Blocks[ibx][iby][ibz]->iy < 0) || (Blocks[ibx][iby][ibz]->iy + Blocks[ibx][iby][ibz]->idy >= TTVol.ny_inlines)
			 || (Blocks[ibx][iby][ibz]->iz < 0) || (Blocks[ibx][iby][ibz]->iz + Blocks[ibx][iby][ibz]->idz >= TTVol.nz) )
		    {
			Blocks[ibx][iby][ibz]->active = false;
			totallycovered = false;
		    }
		    else
		    {
			Blocks[ibx][iby][ibz]->active = true;
			partiallycovered = true;
		    }

		}

	if ( (!totallycovered) && (partiallycovered))
        {
            ierr = -1;
            ///
	    
        }
	  
	if ( (!totallycovered) && (!partiallycovered))
	{
	    
	    ierr = -1;
	}
	  

    }


  ~BlockVolume()
    {
	if (Blocks != NULL)
	{
	    for (int ix = 0; ix < NBx; ix++)
	    {
		for (int iy = 0; iy < NBy; iy++)
		{
		    for (int iz = 0; iz < NBz; iz++)
		    {
			delete Blocks[ix][iy][iz];
		    }
		    delete[] Blocks[ix][iy];
		}
		delete[] Blocks[ix];
	    }
	    delete[] Blocks;
	    Blocks = NULL;
	}
    }


// public attributes
 public:

  /// Number of blocks in the volume
  int NBx, NBy, NBz;
  /// Initial Coordinates of the volume
  float x0, y0, z0;
  /// Spacing of points in the Volume
  float dx, dy, dz;
  /// Maximum number of points in a block
  int nx, ny, nz;

  const Box* getBox(const int i, const int j, const int k){return Blocks[i][j][k];};

// private methods
 private:
  void determine_Box_index(const seisgrid3D& MigVol, const seisgrid3D& TTVol,
			   const int point_x_index, const int point_y_index, const int point_z_index,
			   int& box_x_index, int& box_y_index, int& box_z_index)
      {
	  box_x_index = (int) ((MigVol.first_x_coord.v + point_x_index * MigVol.dx_between_xlines - TTVol.first_x_coord.v + 1e-2) / TTVol.dx_between_xlines);
	  if ((MigVol.first_x_coord.v + point_x_index * MigVol.dx_between_xlines - TTVol.first_x_coord.v) < -0.5)
	      box_x_index = box_x_index - 1;
	  box_y_index = (int) ((MigVol.first_y_coord.v + point_y_index * MigVol.dy_between_inlines - TTVol.first_y_coord.v + 1e-2) / TTVol.dy_between_inlines);
	  if ((MigVol.first_y_coord.v + point_y_index * MigVol.dy_between_inlines - TTVol.first_y_coord.v) < -0.5)
	      box_y_index = box_y_index - 1;
	  box_z_index = (int) ((MigVol.first_z_coord + point_z_index * MigVol.dz - TTVol.first_z_coord + 1e-2) / TTVol.dz);
	  if ((MigVol.first_z_coord + point_z_index * MigVol.dz - TTVol.first_z_coord) < -0.5)
	      box_z_index = box_z_index - 1;
									
      }


// private attributes
 private:
  Box ****Blocks;

  
};
#endif
