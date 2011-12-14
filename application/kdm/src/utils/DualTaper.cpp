/***************************************************************************
                          DualTaper.cpp  -  description
                             -------------------
    begin                : Tue Oct 06 2009
    copyright            : (C) 2009 by Dominik Michel
    email                : micheld@itwm.fhg.de
 ***************************************************************************/

#include "DualTaper.h"

DualTaper::DualTaper(const float _a, const float _b, const float _width)
{
    Init(_a, _b, _width);
}

void DualTaper::Init(const float _a, const float _b, const float _width)
{
    a = _a; b = _b; width = _width;

    xtaper.Init(-width, 3*width, 2*width);
}


float DualTaper::operator()(const float z, const float x) const
{
  float ramp = a*z+b;
  if (x<=ramp-width) return 1.;
  if (x>=ramp+width) return 0.;
  return xtaper(x-ramp);
}

void DualTaper::test(float z0, float z1, float x0, float x1) {
  
  float dx=(x1-x0)/5.;
  float dz=(z1-z0)/5.;
  
  
  
  
  
  
  
  
}