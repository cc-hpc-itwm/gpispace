/***************************************************************************
                          wfptsrc.cpp  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "wfptsrc.h"

WFPtSrc::WFPtSrc():Polyeder(NULL),Rays(NULL),Tris(NULL){
}
WFPtSrc::~WFPtSrc(){
    if (Polyeder != NULL)
	delete Polyeder;
    if (Rays != NULL)
	delete[] Rays;
    if (Tris != NULL)
	delete[] Tris;
}
/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const float& _v){
    pos = x;
    vel_s.v = _v;
    vel_s.A11 = _v*_v;
    vel_s.Ex = 0;
    vel_s.alpha_rad = deg2rad(0.f);
    vel_s.beta_rad = deg2rad(0.f);

    Polyeder = NULL;
    N_Rays = 0;
    Rays = NULL;
    N_Tris = 0;
    Tris = NULL;
}
/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const vti_velocity & _vti){
    pos = x;
    vel_s.v = _vti.v;
    vel_s.A11 = _vti.A11;
    vel_s.Ex = _vti.Ex;
    vel_s.alpha_rad = deg2rad(0.f);
    vel_s.beta_rad = deg2rad(0.f);

    Polyeder = NULL;
    N_Rays = 0;
    Rays = NULL;
    N_Tris = 0;
    Tris = NULL;
}
/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const tti_velocity & _tti){
    pos = x;
    vel_s = _tti;

    Polyeder = NULL;
    N_Rays = 0;
    Rays = NULL;
    N_Tris = 0;
    Tris = NULL;
}

/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const float& _v, const float& dang, const float& beta, const int Type){
  pos = x;
   vel_s.v = _v;
   vel_s.A11 = _v*_v;
   vel_s.Ex = 0;
   vel_s.alpha_rad = deg2rad(0.f);
   vel_s.beta_rad = deg2rad(0.f);

   if (Type == 1)
     Polyeder = new SopheHedron;
   else
   {
       std::cerr << "WFPtSrc: PolyHedron with angle resolution is not yet implemented!\n";
       exit(1);
       //Polyeder = new PolyHedron;
   }
   Polyeder->Init(dang, beta);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}

/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const vti_velocity& _vti, const float& dang, const float& beta, const int Type){
  pos = x;
  vel_s.v = _vti.v;
  vel_s.A11 = _vti.A11;
  vel_s.Ex = _vti.Ex;
  vel_s.alpha_rad = deg2rad(0.f);
  vel_s.beta_rad = deg2rad(0.f);

   if (Type == 1)
     Polyeder = new SopheHedron;
   else
   {
       std::cerr << "WFPtSrc: PolyHedron with angle resolution is not yet implemented!\n";
       exit(1);
       //Polyeder = new PolyHedron;
   }
   Polyeder->Init(dang, beta);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}
/** No descriptions */
WFPtSrc::WFPtSrc(const point3D<float>& x, const tti_velocity& _tti, const float& dang, const float& beta, const int Type){
  pos = x;
  vel_s = _tti;

   if (Type == 1)
     Polyeder = new SopheHedron;
   else
   {
       std::cerr << "WFPtSrc: PolyHedron with angle resolution is not yet implemented!\n";
       exit(1);
       //Polyeder = new PolyHedron;
   }
   Polyeder->Init(dang, beta);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}

WFPtSrc::WFPtSrc(const point3D<float>& x, const float& _v, const float& dang, const point3D<float>& R, const float& phi, const int Type){
  pos = x;
   vel_s.v = _v;
   vel_s.A11 = _v*_v;
   vel_s.Ex = 0;
   vel_s.alpha_rad = deg2rad(0.f);
   vel_s.beta_rad = deg2rad(0.f);

   if (Type == 1)
   {
       std::cerr << "WFPtSrc: SopheHedron with number of rays is no longer implemented!\n";
       exit(1);
       //Polyeder = new SopheHedron;
   }
   else
     Polyeder = new PolyHedron;

   Polyeder->Init(dang, R, phi);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}

WFPtSrc::WFPtSrc(const point3D<float>& x, const vti_velocity& _vti, const float& dang, const point3D<float>& R, const float& phi, const int Type){
  pos = x;
  vel_s.v = _vti.v;
  vel_s.A11 = _vti.A11;
  vel_s.Ex = _vti.Ex;
  vel_s.alpha_rad = deg2rad(0.f);
  vel_s.beta_rad = deg2rad(0.f);

   if (Type == 1)
   {
       std::cerr << "WFPtSrc: SopheHedron with number of rays is no longer implemented!\n";
       exit(1);
       //Polyeder = new SopheHedron;
   }
   else
     Polyeder = new PolyHedron;

   Polyeder->Init(dang, R, phi);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}

WFPtSrc::WFPtSrc(const point3D<float>& x, const tti_velocity& _tti, const float& dang, const point3D<float>& R, const float& phi, const int Type){
  pos = x;
  vel_s = _tti;

   if (Type == 1)
   {
       std::cerr << "WFPtSrc: SopheHedron with number of rays is no longer implemented!\n";
       exit(1);
       //Polyeder = new SopheHedron;
   }
   else
     Polyeder = new PolyHedron;

   Polyeder->Init(dang, R, phi);

   // Initialize the Rays
   N_Rays = Polyeder->N_V;
   Rays = new ray3D[N_Rays];
   for (int i = 0; i < N_Rays; i++)
     Rays[i].x = x;

   point3D<float> * Vertices = new point3D<float>[N_Rays];
   Polyeder->GetVertices(Vertices, N_Rays);

   Set_Slowness(Vertices);
   delete[] Vertices;
   Vertices = NULL;

   // Initialize the Triangles
   N_Tris = Polyeder->N_F;
   //   std::cout << "N_Tris = " << N_Tris << std::endl;
   Tris = new polygon[N_Tris];

   Polyeder->GetFaces(Tris, N_Tris);
}

ray3D WFPtSrc::CreateRay(const Spherical& SD)
{
  point3D<float> p;
  p[0] = sin(SD.theta)*cos(SD.phi)/vel_s.v;
  p[1] = sin(SD.theta)*sin(SD.phi)/vel_s.v;
  p[2] = cos(SD.theta)/vel_s.v;
  ray3D newray(pos, p, vel_s.v, SD, 0);

  newray.Q[0][0] = 0;
  newray.Q[0][1] = 0;
  newray.Q[1][0] = 0;
  newray.Q[1][1] = 0;
  newray.Q[2][0] = 0;
  newray.Q[2][1] = 0;
  
  newray.P[0][0] = -sin(SD.phi)*sin(SD.theta)/vel_s.v;
  newray.P[0][1] = cos(SD.phi)*cos(SD.theta)/vel_s.v;
  newray.P[1][0] = cos(SD.phi)*sin(SD.theta)/vel_s.v;
  newray.P[1][1] = sin(SD.phi)*cos(SD.theta)/vel_s.v;
  newray.P[2][0] = 0;
  newray.P[2][1] = -sin(SD.theta)/vel_s.v;

  return newray;
}

/** No descriptions */
void WFPtSrc::Set_Slowness(point3D<float>* _V){
  for (int i = 0; i < N_Rays; i++)
  {
    const double DXY = sqrt( _V[i][0]*_V[i][0] + _V[i][1]*_V[i][1]);
    double phi, theta;

    if ( fabs(_V[i][2]) > 1.0)
      theta = 0;
    else
      theta = acos( _V[i][2]);

    if (fabs(sin(theta)) < 1e-4)
        phi = 0;
    else
      if ( fabs( _V[i][0] /  sin(theta)) > 1.0 )
        {
          phi = 0;
          if (_V[i][0] < 0)
            phi = _PI;
        } 
      else
        phi = acos( _V[i][0] /  sin(theta));    
    if (_V[i][1] < 0)
      phi = 2*_PI-phi;

    if ( phi > _PI)
      phi -=  2*_PI;

    const float calpha = cosf(vel_s.alpha_rad);
    const float salpha = sinf(vel_s.alpha_rad);
    const float cbeta = cosf(vel_s.beta_rad);
    const float sbeta = sinf(vel_s.beta_rad);

    // tti rotation matrix 
    const float R00 = calpha*cbeta; 
    const float R01 = calpha*sbeta; 
    const float R02 = -salpha; 
    const float R10 = -sbeta; 
    const float R11 = cbeta; 
    const float R12 = 0;
    const float R20 = salpha*cbeta;
    const float R21 = salpha*sbeta;
    const float R22 = calpha;
    
    const float _V_rotated_z = R20*_V[i][0] + R21*_V[i][1] + R22*_V[i][2];
    
    const float ctheta_rotated = _V_rotated_z;
    const float c2theta_rotated = _V_rotated_z*_V_rotated_z;
    const float s2theta_rotated = 1.f - c2theta_rotated;

    const float v_ph = sqrtf( vel_s.v*vel_s.v*c2theta_rotated 
 			      + vel_s.A11*s2theta_rotated 
 			      + vel_s.Ex*(c2theta_rotated*s2theta_rotated));

    Rays[i].p[0] = _V[i][0] / v_ph;
    Rays[i].p[1] = _V[i][1] / v_ph;
    Rays[i].p[2] = _V[i][2] / v_ph;

    Rays[i].StartDir.phi = phi;
    Rays[i].StartDir.theta = theta;

    Rays[i].Q[0][0] = 0;
    Rays[i].Q[1][0] = 0;
    Rays[i].Q[2][0] = 0;
    Rays[i].Q[0][1] = 0;
    Rays[i].Q[1][1] = 0;
    Rays[i].Q[2][1] = 0;
    Rays[i].Q[0][2] = Rays[i].p[0];
    Rays[i].Q[1][2] = Rays[i].p[1];
    Rays[i].Q[2][2] = Rays[i].p[2];

    const float dc2theta_rotated_dtheta = 2.f*ctheta_rotated * (R20*cosf(phi)*cosf(theta) + R21*sinf(phi)*cosf(theta) - R22*sinf(theta));
    const float dc2theta_rotated_dphi = 2.f*ctheta_rotated * ( -R20* sinf(phi) + R21*cosf(phi));

    const float dv_ph_dtheta = dc2theta_rotated_dtheta * (vel_s.v*vel_s.v - vel_s.A11 + vel_s.Ex*(s2theta_rotated - c2theta_rotated) )/(2.f*v_ph*v_ph);
    const float dv_ph_dphi = dc2theta_rotated_dphi * (vel_s.v*vel_s.v - vel_s.A11 + vel_s.Ex*(s2theta_rotated - c2theta_rotated) )/(2.f*v_ph*v_ph);
    const float dv_ph_dtheta_tmp = sqrtf(c2theta_rotated * s2theta_rotated) * (vel_s.A11 - vel_s.v*vel_s.v + vel_s.Ex*(c2theta_rotated - s2theta_rotated) )/(v_ph*v_ph);

    Rays[i].P[0][0] = -sinf(phi)/v_ph - dv_ph_dphi * cosf(phi)*sinf(theta)/v_ph;
    Rays[i].P[1][0] = cosf(phi)/v_ph  - dv_ph_dphi * sinf(phi)*sinf(theta)/v_ph;
    Rays[i].P[2][0] = - dv_ph_dphi * cosf(theta)/v_ph;

    Rays[i].P[0][1] = cosf(phi)*cosf(theta)/v_ph - dv_ph_dtheta * cosf(phi)*sinf(theta)/v_ph;
    Rays[i].P[1][1] = sinf(phi)*cosf(theta)/v_ph - dv_ph_dtheta * sinf(phi)*sinf(theta)/v_ph;
    Rays[i].P[2][1] = -sinf(theta)/v_ph - dv_ph_dtheta * cosf(theta)/v_ph;


    Rays[i].P[0][2] = cosf(phi)*sinf(theta)/v_ph;
    Rays[i].P[1][2] = sinf(phi)*sinf(theta)/v_ph;
    Rays[i].P[2][2] = cosf(theta)/v_ph;
  }
}
/** Return the number of rays that are generated. */
int WFPtSrc::GetNoRays() const{
  return N_Rays;
}
/** Return ray with index i. */
void WFPtSrc::GetRays(ray3D* _rays){
  if ( _rays == NULL )
    std::cerr << "WFSource::GetRays : Memory has not been allocated.\n";

  for (int i = 0; i < N_Rays; i++)
    _rays[i] = Rays[i];
}
/** Return ray with index i. */
ray3D WFPtSrc::GetRay(const int & i) const{
  if ( (i < 0) || (i >= N_Rays))
    std::cerr << "WFSource::GetRay : Index i out of Range.\n";
  return Rays[i];
}
/** Return ray with index i that starts from a translated source to position x with velocity _v. */
ray3D WFPtSrc::GetTranslatedRay(const int & i, point3D<float>& new_x, const float& new_v){
  if ( (i < 0) || (i >= N_Rays))
    std::cerr << "WFSource::GetRay : Index i out of Range.\n";
  ray3D new_ray = Rays[i];

  new_ray.x = new_x;

  new_ray.p = new_ray.p * (vel_s.v/new_v);

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 2; j++)
      new_ray.P[i][j] = new_ray.P[i][j] * (vel_s.v/new_v);

  return new_ray;
}
/** Return the number of triangles that are generated. */
int WFPtSrc::GetNoTriangles(){
  return N_Tris;
}
/** Return ray with index i. */
void WFPtSrc::GetTris(polygon* _tris){
  if ( _tris == NULL )
    std::cerr << "WFSource::GetTris : Memory has not been allocated.\n";

  for (int i = 0; i < N_Tris; i++)
    _tris[i] = Tris[i];
}
/** Return ray with index i. */
polygon WFPtSrc::GetTri(const int & i){
  if ( (i < 0) || (i >= N_Tris))
    std::cerr << "WFSource::GetTri : Index i out of Range.\n";
  return Tris[i];
}

int WFPtSrc::GetVWidth(){
  return Polyeder->width;
}

/** No descriptions */
int WFPtSrc::GetFWidth(){
  return 2 * Polyeder->width;
}
