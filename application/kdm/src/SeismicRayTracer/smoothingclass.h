/***************************************************************************
                          smoothingclass.h  -  description

    Perform smoothing of scalar models.

                             -------------------
    begin                : Tue Jan 25 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2009-11-06
	        threading of smoothing loop in x direction;
                coarsening is accessible

***************************************************************************/

#ifndef SMOOTHINGCLASS_H
#define SMOOTHINGCLASS_H

/**
 *@author Dirk Merten
 */

#include "MCTP1.h"
#include "include/defs.h"
#include "structures/point3d.h"
#include "filehandler/SegYHeader.h"
#include "filehandler/seismicfilereader.h"
#include "filehandler/seismicfilewriter.h"
#include "utils/progressbar.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>



/** 
 * \ingroup group2
 * \brief Perform smoothing of scalar models.
 * \details A 3-dimensional scalar model is read from disk, smoothed, 
 *          coarsened and written back to disk. The model is specified by
 *          the name and type of the input file, the size and scale of
 *          the cube and the name of the output file.
 *          The length of the smoothing operator is given in physical
 *          units. 
 *          The coarsening is specified by an integer factor sepeprately 
 *          for all dimensions.
 */

class SmoothingClass {

typedef struct{
    int Nx_start;
    int Nx_end;
    point3D<unsigned int> Ndim;
    point3D<unsigned int> dcoarse;
    point3D<unsigned int> SmoothingLength;
    float**** buffer;
    int in, out;
    int mode;
    int tid;
} thread_parm_t;

// public methods
public: 
  SmoothingClass(){};
  ~SmoothingClass(){};

  /**
   * \brief   Linear smoothing
   * \details Each output point is substituted by the mean value of 
   *          all points in the surrounding box with a half width
   *          of the smoothing length
   */
  int SmoothLinear(const char* InputFileName, 
		   const char* OutputFileName,
		   const FILE_MODE PropFileMode,
		   const point3D<int> Ndim, 
		   const point3D<float> dx, 
		   const float SmoothingLengthMeter,
		   const point3D<int> dcoarse = point3D<int>(1,1,1));

  /**
   * \brief   Reciprocal or slowness smoothing
   * \details Each output point is substituted by the inverse of the mean 
   *          of the reciprocal values at all points in the surrounding 
   *          box with a half width of the smoothing length
   */
  int SmoothInverse(const char* InputFileName, 
		    const char* OutputFileName, 
		    const FILE_MODE PropFileMode,
		    const point3D<int> Ndim, 
		    const point3D<float> dx, 
		    const float SmoothingLengthMeter,
		    const point3D<int> dcoarse = point3D<int>(1,1,1));

// public attributes
public: 

// private methods
private:
  /// General smoothing driver routine, algorithm depending on mode 
  int Smooth(const char* InputFileName, 
	     const char* OutputFileName,
	     const FILE_MODE PropFileMode,
	     const point3D<int> Ndim, 
	     const point3D<float> dx, 
	     const float SmoothingLengthMeter,
	     const point3D<int> dcoarse,
	     const int mode);

  /// Wrapper function for threaded start-up 
  static void* Smooth_Loop_ThreadStartup(void* parmptr);

  /// Linear smoothing operational loop
  static void SmoothLinear_Loop(const int Nx_start, const int Nx_end,
				const point3D<unsigned int> Ndim,
				const point3D<unsigned int> dcoarse,
				const point3D<unsigned int> SmoothingLength,
				float**** buffer,
				const int in, const int out,
				const int tid);
  
  /// Inverse smoothing operational loop
  static void SmoothInverse_Loop(const int Nx_start, const int Nx_end,
				 const point3D<unsigned int> Ndim,
				 const point3D<unsigned int> dcoarse,
				 const point3D<unsigned int> SmoothingLength,
				 float**** buffer,
				 const int in, const int out,
				 const int tid);
  

// private attributes
private:
  
};
#endif

