/***************************************************************************
                          trisplit.cpp  -  description
                             -------------------
    begin                : Tue Dec 6 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "trisplit.h"

TriSplit::TriSplit(){
}
TriSplit::~TriSplit(){
}
/** No descriptions */
point3D<float> TriSplit::MidPoint(const point3D<float>& p1, const point3D<float>& p2, const point3D<float>& p3){

  const float x0 = (p1[0] + p2[0] + p3[0]) / 3.0;
  const float x1 = (p1[1] + p2[1] + p3[1]) / 3.0;
  const float x2 = (p1[2] + p2[2] + p3[2]) / 3.0;

  const float norm = sqrt(x0*x0 + x1*x1 + x2*x2);
  point3D<float> p_mid(x0/norm, x1/norm, x2/norm);

  return p_mid;
}
/** No descriptions */
point3D<float> TriSplit::MidLine(const point3D<float> & p1, const point3D<float> & p2){

  const float x0 = (p1[0] + p2[0]) / 2.0;
  const float x1 = (p1[1] + p2[1]) / 2.0;
  const float x2 = (p1[2] + p2[2]) / 2.0;

  const float norm = sqrt(x0*x0 + x1*x1 + x2*x2);
  point3D<float> p_mid(x0/norm, x1/norm, x2/norm);

  return p_mid;
}
