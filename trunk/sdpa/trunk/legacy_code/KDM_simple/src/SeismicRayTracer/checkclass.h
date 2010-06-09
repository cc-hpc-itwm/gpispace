/***************************************************************************
                          checkclass.h  -  description
                             -------------------
    begin                : Tue Jan 24 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef CHECKCLASS_H
#define CHECKCLASS_H


/**
  *@author Dirk Merten
  */
#include "utils/propmodel.h"
#include "memory/memman3d_bucket.h"
#include "utils/timer.h"
#include "include/types.h"

#include <iostream>

class CheckClass {
public: 
	CheckClass();
	~CheckClass();
  /** Checks the TriangleList for consistency of the neighborhood relations. */
  bool CheckTriangleList(triMem& TriangleList);
  /** Checks and measures time for the access operations via the memory manager MemMan3D_bucket etc.*/
  void CheckMemoryAccess();
  void CheckVelocityField(PropModel<VelGridPointBSpline>& VF);
};

#endif
