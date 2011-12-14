/***************************************************************************
                          types.h  -  description
                             -------------------
    begin                : Wed Dec 7 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/
#ifndef TYPES_H
#define TYPES_H


#include <vector>

#include "SeismicRayTracer/ray3d.h"
#include "SeismicRayTracer/triray3d.h"
#include "structures/triangle.h"
#include "SeismicRayTracer/receiver.h"
#include "memory/nodemem.h"
#include "memory/memman3d_malloc.h"

typedef NodeMem<triray3D> trirayMem;
typedef NodeMem<ray3D> rayMem;
typedef NodeMem<Triangle> triMem;

typedef MemMan3D_malloc<Receiver> RcvMem;

#endif
