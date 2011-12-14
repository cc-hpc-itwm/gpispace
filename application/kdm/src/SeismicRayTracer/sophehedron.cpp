/***************************************************************************
                          sophehedron.cpp  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "sophehedron.h"

SopheHedron::SopheHedron(){
}
SopheHedron::~SopheHedron(){
}
/** A SopheHedron with a minimum number of Nmin Verticees is generated. */
void SopheHedron::Init_N(const int& N){
  std::cerr << "Sorry. A SopheHedron with given number of verticees is no longer implemented!\n";
}
/** A SopheHedron with a minimum number of Nmin Verticees is generated. */
void SopheHedron::Init(const float& dang, const float& beta){

  SopheParam Base(rad2deg(dang), rad2deg(beta));

  N_V = Base.GetNtot(); N_F = 1;

  Vertices.clear();
  Vertices.reserve(N_V);

  Vertices.resize(N_V);

  int count = 0;
  for (int itheta = 0; itheta < Base.GetNtheta(); itheta++)
    for (int iphi = 0; iphi < Base.GetNphi(itheta); iphi++)
      {
        const float x = Base.Getsintheta(itheta)*Base.Getcosphi(itheta, iphi);
        const float y = Base.Getsintheta(itheta)*Base.Getsinphi(itheta, iphi);
        const float z = Base.Getcostheta(itheta);

        Vertices[count] = point3D<float>(x, y, z);
        count++;
      }

  Faces.clear();
  Faces.resize(N_F);


  width = 64; // this width is not reasonable
}

/** A SopheHedron Cap from a SopheHedron with a minimum number of Nmin Verticees pointing into  direction X with opening angle beta is generated. */
void SopheHedron::Init(const int& Nmin, const point3D<float>& X, const float& beta)
{
  std::cerr << "Sorry. A SopheHedron Cap is not implemented yet!\n";
}
/** A SopheHedron Cap from a SopheHedron with a minimal angular resolution pointing into  direction X with opening angle beta is generated. */
void SopheHedron::Init(const float& dang, const point3D<float>& X, const float& beta)
{
  std::cerr << "Sorry. A SopheHedron Cap is not implemented yet!\n";
}

void SopheHedron::GetVertices(point3D<float>* _vert, const int& _N_V){
  for (int i = 0; i < _N_V; i++)
    _vert[i] = Vertices[i];
}

void SopheHedron::GetFaces(polygon* _faces, const int& _N_F){
    //std::cerr << "Sorry. SopheHedron Faces are not implemented yet!\n";
  for (int i = 0; i < _N_F; i++)
    _faces[i] = Faces[i];
}

