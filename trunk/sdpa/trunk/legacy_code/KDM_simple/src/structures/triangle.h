/***************************************************************************
                          triangle.h  -  description

    Base entity for wave front triangulation. One triangle is defined by
    three rays and its neighboring triangles.

                             -------------------
    begin                : Wed Nov 16 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef TRIANGLE_H
#define TRIANGLE_H


/**
 *@author Dirk Merten
 */

#include "SeismicRayTracer/triray3d.h"

class Triangle {

// public methods
public: 
    Triangle();
    Triangle(const int& _id);
    Triangle(const int& _id, triray3D* r0, triray3D* r1, triray3D* r2, Triangle* t0, Triangle* t1, Triangle* t2);
    ~Triangle();

    enum{DELETED, UNDEFINED};

// public attributes
 public:
    /** Identity number */
    int id;
    /** Corner rays */
    triray3D* points[3];
    /** Neighboring triangles */
    Triangle* neighs[3];
    /** Flag for updating */
    bool status;
    /** Status */    
    int lifesign;
};

#endif
