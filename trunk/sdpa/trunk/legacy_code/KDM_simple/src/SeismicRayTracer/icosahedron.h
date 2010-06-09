/***************************************************************************
                          icosahedron.h  -  description
                             -------------------
    begin                : Wed Dec 7 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef ICOSAHEDRON_H
#define ICOSAHEDRON_H


/**
 *@author Dirk Merten
 */
#include<vector>
#include "math.h"
#include "polygon.h"
#include "structures/point3d.h"

class Icosahedron {
 public: 
    Icosahedron();
    ~Icosahedron();
 public: // Public attributes
    /**  */
    int N_V;
    /**  */
    int N_F;
    /**  */
    point3D<float> Vertices[12];
    /**  */
    polygon Faces[20];
};

#endif
