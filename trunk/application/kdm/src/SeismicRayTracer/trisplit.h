/***************************************************************************
                          trisplit.h  -  description
                             -------------------
    begin                : Tue Dec 6 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef TRISPLIT_H
#define TRISPLIT_H


/**
  *@author Dirk Merten
  */

#include "structures/point3d.h"

#include <math.h>

class TriSplit {
public: 
	TriSplit();
	~TriSplit();
  /** No descriptions */
  point3D<float> MidPoint(const point3D<float>& p1, const point3D<float>& p2, const point3D<float>& p3);
  /** No descriptions */
  point3D<float> MidLine(const point3D<float>& p1, const point3D<float>& p2);
};

#endif
