/***************************************************************************
                          spherical.cpp  -  description
                             -------------------
    begin                : Tue Jan 31 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "spherical.h"

Spherical::Spherical():phi(-1),theta(-1){
}
Spherical::Spherical(const double& _phi, const double& _theta)
    :phi(_phi),theta(_theta){}
Spherical::~Spherical(){
}
