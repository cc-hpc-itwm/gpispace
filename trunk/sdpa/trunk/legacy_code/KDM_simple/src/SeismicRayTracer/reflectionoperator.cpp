/***************************************************************************
                          reflectionoperator.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "reflectionoperator.h"

ReflectionOperator::ReflectionOperator()
{
  NIfcs = 1;
  Ifcs = new Interface[NIfcs];
  Ifcs[0].init(-100);
}
ReflectionOperator::~ReflectionOperator()
{
  delete[] Ifcs;
}
/** No descriptions */
bool ReflectionOperator::operator() (const ray3D* ray, float& l)
{
  l = 0;
  if ( (fabs(ray->StartDir.phi - 5.41899) < 1e-4) && (fabs(ray->StartDir.theta - 2.78699) < 1e-4) ) 
    l = -1;

  bool ret = false;
  for (int iIfcs = 0; iIfcs < NIfcs; iIfcs++)
    {
      if ( ((ray->x[2] > Ifcs[iIfcs].min) || (ray->x_old[2] > Ifcs[iIfcs].min))
           && ((ray->x[2] < Ifcs[iIfcs].max) || (ray->x_old[2] < Ifcs[iIfcs].max)) ) 
        {
          for (int it = 0; it < Ifcs[iIfcs].Ntri; it++)
            {
              if ( ((ray->x[2] > Ifcs[iIfcs].tri[it].min) || (ray->x_old[2] > Ifcs[iIfcs].tri[it].min))
                   && ((ray->x[2] < Ifcs[iIfcs].tri[it].max) || (ray->x_old[2] < Ifcs[iIfcs].tri[it].max)) ) 
                {
                  const float x0 = Ifcs[iIfcs].X0 + Ifcs[iIfcs].tri[it].ix0 * Ifcs[iIfcs].dx;
                  const float y0 = Ifcs[iIfcs].Y0 + Ifcs[iIfcs].tri[it].iy0 * Ifcs[iIfcs].dy;
                  const float z0 = Ifcs[iIfcs].h[Ifcs[iIfcs].tri[it].ix0][Ifcs[iIfcs].tri[it].iy0];

                  point3D<float> P0(x0,y0,z0);
                
                  const float x1 = Ifcs[iIfcs].X0 + Ifcs[iIfcs].tri[it].ix1 * Ifcs[iIfcs].dx;
                  const float y1 = Ifcs[iIfcs].Y0 + Ifcs[iIfcs].tri[it].iy1 * Ifcs[iIfcs].dy;
                  const float z1 = Ifcs[iIfcs].h[Ifcs[iIfcs].tri[it].ix1][Ifcs[iIfcs].tri[it].iy1];
                  point3D<float> P1(x1, y1, z1);
                
                  const float x2 = Ifcs[iIfcs].X0 + Ifcs[iIfcs].tri[it].ix2 * Ifcs[iIfcs].dx;
                  const float y2 = Ifcs[iIfcs].Y0 + Ifcs[iIfcs].tri[it].iy2 * Ifcs[iIfcs].dy;
                  const float z2 = Ifcs[iIfcs].h[Ifcs[iIfcs].tri[it].ix2][Ifcs[iIfcs].tri[it].iy2];
                  point3D<float> P2(x2, y2, z2);

                  normal = Ifcs[iIfcs].tri[it].normal;
                  d = Ifcs[iIfcs].tri[it].d;

                  ret = Intersection(P0, P1, P2, ray->x_old, ray->x, l);

                  if (ret)
                    return ret;
                }
            }
        }
    }

  return ret;
}

void ReflectionOperator::reflect(ray3D* ray)
{
  std::cout << ray->p << "    ";
  ray->p = ray->p - (normal * (2*(ray->p * normal)));
  std::cout << ray->p << std::endl;

  //ray->x = ray->x + (normal * (2*(d - (ray->x * normal))));
  //ray->p[2] = -ray->p[2];
}

bool ReflectionOperator::Intersection(const point3D<float>& P0, const point3D<float>& P1, const point3D<float>& P2, 
                    const point3D<float>& x0, const point3D<float>& x1, float& l)
{
  // Distance of x0 to face
  const float d0 = x0 * normal - d;
  // Distance of x1 to face
  const float d1 = x1 * normal - d;

  // On same side?
  if ( (d1*d0) > 0)
    return false;
  // Distance of old point to face
  float l_r = fabs(d0);

  const point3D<float> v = (x1-x0)/(x1-x0).norm();

  // Intersection of ray with face
  const point3D<float> P_proj = x0 + v * l_r;

  int axis;
  if ( fabs(normal[0]) > fabs(normal[1]) )
    if ( fabs(normal[0]) > fabs(normal[2]) )
      axis = 0;
    else
      axis = 2;
  else
    if ( fabs(normal[1]) > fabs(normal[2]) )
      axis = 1;
    else
      axis = 2;
    
  int uaxis = (axis + 1)%3;
  int vaxis = (uaxis + 1)%3;
  const float dx10 = (P1[uaxis] - P0[uaxis]);
  const float dy10 = (P1[vaxis] - P0[vaxis]);
  const float dx20 = (P2[uaxis] - P0[uaxis]);
  const float dy20 = (P2[vaxis] - P0[vaxis]);
  float t2 = ( (P0[uaxis] - P_proj[uaxis])*dy10 + (P_proj[vaxis] - P0[vaxis])*dx10 ) / ( dy20*dx10 - dx20*dy10);
  float t1 = ( (P0[vaxis] - P_proj[vaxis])*dx20 + (P_proj[uaxis] - P0[uaxis])*dy20 ) / ( dy20*dx10 - dx20*dy10);

//   if (l == -1)
//     {
//       std::cout << "axis = " << axis << std::endl;
//       std::cout << "normal, d, l_r = " << normal << ", " << d << ", " << l_r << std::endl;
//       std::cout << "x0, x1 = " << x0 << ", " << x1 << std::endl;
//       std::cout << "v = " << v << std::endl;
//       std::cout << "P0, P1, P2 = " << P0 << ", " << P1 << ", " << P2 << std::endl;
//       std::cout << "P_proj = " << P_proj << std::endl;
//       std::cout << "dx10, dx20 = " << dx10 << ", " << dx20 << std::endl;
//       std::cout << "dy10, dy20 = " << dy10 << ", " << dy20 << std::endl;
//       std::cout << "t1, t2 = " << t1 << ", " << t2 << std::endl;
//     }

  if ( (t1 < 0) || (t2 < 0) || (t1+t2 > 1) )
    {
        return false;
    }
  else
    {
      l = fabs(l_r);
      return true;
    }
};

