/***************************************************************************
                          polygon.h  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef POLYGON_H
#define POLYGON_H


/**
  *@author Dirk Merten
  */

class polygon {
public: 
	polygon();
	~polygon();
public: // Public attributes
  /**  */
  int v[3];
  /**  */
  int n[3];
};

#endif
