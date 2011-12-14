/*************************************************************************
                          refinementoperator.h  -  description
                             -------------------                          */
/**
  The RefinementOperator class provides routines for the refinement of the
  wave front triangulation, for inserting and for deleting rays, i.e. vertices
  of triangles. The triangulation is refined if the length of a side of a
  triangle becomes larger then the reference length max_line. In this case
  the midpoint of this line is inserted as a new vertex and the observables on
  the wave front are interpolated to this new vertex. The list of triangles 
  of the triangulation is updated, new triangles are inserted and the 
  neighboring information of each triangle is upgraded.
  *@author Dirk Merten
  */

/*                           -------------------                         
    begin                : Wed Nov 16 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-14
                changed to template with parameter TracingOperator_T

 ***************************************************************************/


#ifndef REFINEMENTOPERATOR_H
#define REFINEMENTOPERATOR_H


#include "include/types.h"
#include "structures/triangle.h"
#include "ray3d.h"
#include "memory/memman3d_bucket.h"
#include "memory/nodemem.h"
#include "wfptsrc.h"

#include <iostream>
#include <math.h>

template<class TracingOperator_T> 
class RefinementOperator {
public: 
  /// Standard constructor of RefinementOperator sets max_line to a large number,
  /// i.e. no refinement.
  RefinementOperator();
  /// Constructor of RefinementOperator initializing max_line to lmax.
  RefinementOperator(const int& lmax, TracingOperator_T* TrOp, WFPtSrc* _Source, const int _ith = 0);
  ~RefinementOperator();
  /// Check for refinement and refine the whole triangulation. 
  void operator () (triMem& _TList, trirayMem& _RList, const float& t );
  /// Check for refinement and refine the triangle T of the triangulation.
  void operator () (triMem& _TList, trirayMem& _RList, Triangle* T, const float& t );
  /// Delete triangle T from the triangulation. 
  /** T will no longer appear in the list of neighbors of its neighbors and the lifesign 
     is set to Triangle::DELETED. But the memory allocated for T is not freed*/
  void DeleteTri(Triangle* T);
//  void DeleteRay(Triangle* T, ray3D* r_old, ray3D* r_new, Triangle* n1_old, Triangle* n1_new, Triangle* n2_old, Triangle* n2_new);
private: // Private attributes
  /// Maximum lenght of side of triangles before refinement is invoked. 
  int max_line;
  int min_line;
private: // Private methods
  /// Check for refinement and refine the sides of triangle T. 
  /** If any side of T exceeds max_line a midpoint and two new triangles
      are inserted. This operation is done for each side of T but not recursively.*/
  int Subdivide(triMem* TList, trirayMem* RList, Triangle* T, const float& t);
  /// Check if line #ld of triangle T exceeds max_line. 
  int CheckDistances(const Triangle& T, int* ld);
  /// Create a new ray with interpolated observables from rays r1 and r2. 
  /** The new ray is located half way between r1 and r2. Therefore 
      the simple mean value of x, p, beta and gamma is used.*/
  ray3D InterpolateRay(const ray3D& r1, const ray3D& r2);
  kinray3D InterpolateRay(const kinray3D& r1, const kinray3D& r2);
  /// Trace a new ray from the source with interpolated starting values from rays r1 and r2. 
  triray3D InterpolateRay(const triray3D& r1, const triray3D& r2, const float& t);

  /// Accoustic Source
  WFPtSrc* Source;
  /// Pointer to the operator for tracing interpolated rays.
  TracingOperator_T * Tracer;

  int ith;
};

#include "refinementoperator.cpp"
#endif
