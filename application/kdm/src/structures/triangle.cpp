/***************************************************************************
                          triangle.cpp  -  description
                             -------------------
    begin                : Wed Nov 16 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "triangle.h"

Triangle::Triangle(){
  id = -1;
  status = false;
  lifesign = UNDEFINED;
  for (int i = 0; i < 3; i++)
  {
    points[i] = NULL;
    neighs[i] = NULL;
  }
}
Triangle::Triangle(const int& _id){
  if (_id < 0)
    std::cerr << "Error in Triangle: Constructor with id = " << _id << std::endl;
  id = _id;
  status = false;
  lifesign = UNDEFINED;
  for (int i = 0; i < 3; i++)
  {
    points[i] = NULL;
    neighs[i] = NULL;
  }
}
Triangle::~Triangle(){
}

Triangle::Triangle(const int& _id, triray3D* r0, triray3D* r1, triray3D* r2, Triangle* t0, Triangle* t1, Triangle* t2){
  if (_id < 0)
    std::cerr << "Error in Triangle: Constructor with id = " << _id << std::endl;
  id = _id;

  points[0] = r0;
  points[1] = r1;
  points[2] = r2;

  neighs[0] = t0;
  neighs[1] = t1;
  neighs[2] = t2;

  status = false;
  lifesign = UNDEFINED;
}
