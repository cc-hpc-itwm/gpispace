/***************************************************************************
                          polygon.cpp  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "polygon.h"

polygon::polygon(){
  for (int i = 0; i < 3; i++)
  {
    v[i] = -1;
    n[i] = -1;
  }
}
polygon::~polygon(){
}
