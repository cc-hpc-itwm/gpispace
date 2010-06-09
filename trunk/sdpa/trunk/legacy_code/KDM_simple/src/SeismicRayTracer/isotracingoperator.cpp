/***************************************************************************
                          isotracingoperator.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


template<class VelModelType>
isoTracingOperator<VelModelType>::isoTracingOperator():VelModel(NULL)
{
    dtstep_max = 0;
}

template<class VelModelType>
isoTracingOperator<VelModelType>::~isoTracingOperator(){
}

/** operator () (const isoTracingOperator& ) */
template<class VelModelType>
void isoTracingOperator<VelModelType>::operator () (ray3D& ray, const int ith)
{
  ray.Store();

  point3D<float> gradv;

  const float v = VelModel->GetProperty(ray.x, gradv, ith);
  const float gv = gradv.norm();

  const int nt_tmp = max(1, (int) (100.0f * gv * dtstep_max));
  const float dt_max = (dtstep_max)/nt_tmp;

  for (int it = 0; it < nt_tmp; it++)
      DRT(dt_max, ray, ith);



   ray.detQ = ((double)ray.Q[0][0]*(double)ray.Q[1][1]*(double)ray.Q[2][2] + (double)ray.Q[0][1]*(double)ray.Q[1][2]*(double)ray.Q[2][0] + (double)ray.Q[0][2]*(double)ray.Q[1][0]*(double)ray.Q[2][1] - 
                (double)ray.Q[0][0]*(double)ray.Q[2][1]*(double)ray.Q[1][2] - (double)ray.Q[0][1]*(double)ray.Q[1][0]*(double)ray.Q[2][2] - (double)ray.Q[0][2]*(double)ray.Q[1][1]*(double)ray.Q[2][0]);

   if ( (ray.detQ * ray.detQ_old) < 0)
      {
        ray.kmah++;
      }
}

/** operator () (const isoTracingOperator& ) */
template<class VelModelType>
void isoTracingOperator<VelModelType>::operator () (ray3D* rays[4], const int ith)
{
  for (int n = 0; n < 4; n++)
    rays[n]->Store();

  for (int n = 0; n < 4; n++)
      DRT(dtstep_max, *rays[n], ith);

  for (int n = 0; n < 4; n++)
    {
      rays[n]->detQ = rays[n]->Q[0][0]*rays[n]->Q[1][1]*rays[n]->Q[2][2] + rays[n]->Q[0][1]*rays[n]->Q[1][2]*rays[n]->Q[2][0] + rays[n]->Q[0][2]*rays[n]->Q[1][0]*rays[n]->Q[2][1] - 
        rays[n]->Q[0][0]*rays[n]->Q[2][1]*rays[n]->Q[1][2] - rays[n]->Q[0][1]*rays[n]->Q[1][0]*rays[n]->Q[2][2] - rays[n]->Q[0][2]*rays[n]->Q[1][1]*rays[n]->Q[2][0];

       if ( (rays[n]->detQ * rays[n]->detQ_old) < 0)
         rays[n]->kmah++;
    }
}

/** operator () (const isoTracingOperator& ) */
template<class VelModelType>
void isoTracingOperator<VelModelType>::operator () (triray3D& ray, const int ith)
{
  ray.Store();

  point3D<float> gradv;

  const float v = VelModel->GetProperty(ray.mainray.x, gradv, ith);
  const float gv = gradv.norm();

  const int nt_tmp = max(1, (int) (100.0f * gv * dtstep_max));
  const float dt_max = (dtstep_max)/nt_tmp;

  for (int it = 0; it < nt_tmp; it++)
  {
      DRT(dt_max, ray.mainray, ith);
      TraceTstep_RungeKutta(dt_max, ray.dxray, ith);
      TraceTstep_RungeKutta(dt_max, ray.dyray, ith);
  }
  
  ray.mainray.detQ = ((double)ray.mainray.Q[0][0]*(double)ray.mainray.Q[1][1]*(double)ray.mainray.Q[2][2] + (double)ray.mainray.Q[0][1]*(double)ray.mainray.Q[1][2]*(double)ray.mainray.Q[2][0] + (double)ray.mainray.Q[0][2]*(double)ray.mainray.Q[1][0]*(double)ray.mainray.Q[2][1] - 
	      (double)ray.mainray.Q[0][0]*(double)ray.mainray.Q[2][1]*(double)ray.mainray.Q[1][2] - (double)ray.mainray.Q[0][1]*(double)ray.mainray.Q[1][0]*(double)ray.mainray.Q[2][2] - (double)ray.mainray.Q[0][2]*(double)ray.mainray.Q[1][1]*(double)ray.mainray.Q[2][0]);

   if ( (ray.mainray.detQ * ray.mainray.detQ_old) < 0)
      {
        ray.mainray.kmah++;
      }
}

/** operator () (const isoTracingOperator& ) */
template<class VelModelType>
void isoTracingOperator<VelModelType>::TraceT(triray3D& ray, const float& _dt, const int ith)
{
  int nt_tmp = (int) (_dt/dtstep_max);
  for (int it = 0; it < nt_tmp; it++)
    {
	DRT(dtstep_max, ray.mainray, ith);
	TraceTstep_RungeKutta(dtstep_max, ray.dxray, ith);
	TraceTstep_RungeKutta(dtstep_max, ray.dyray, ith);
      //std::cout << ray.x << std::endl;
    }


  DRT(_dt - nt_tmp*dtstep_max, ray.mainray, ith);
  TraceTstep_RungeKutta(_dt - nt_tmp*dtstep_max, ray.dxray, ith);
  TraceTstep_RungeKutta(_dt - nt_tmp*dtstep_max, ray.dyray, ith);
}

template<class VelModelType>
void isoTracingOperator<VelModelType>::TraceTstep(const double _dt, ray3D& ray, const double& v, const int ith){
  ray.x[0] += _dt * v*v * ray.p[0];
  ray.x[1] += _dt * v*v * ray.p[1];
  ray.x[2] += _dt * v*v * ray.p[2];
}


/** No descriptions */
template<class VelModelType>
void isoTracingOperator<VelModelType>::Init(const float& _dt){
  if (_dt <= 0)
    {
      std::cerr << "FATAL ERROR in isoTracingOperator<VelModelType>::Init\n";
      std::cerr << "            The time interval _dt = " << _dt << std::endl;
      std::cerr << "            is <= 0.\n";
      exit(1);
    }
  dtstep_max = _dt;
}
/** No descriptions */
// void isoTracingOperator<VelModelType>::TraceTstep_Vinje(const double _dt, ray3D & ray, const double& v,
//                                        const point3D<float>& vgrad, double (*vgrad2)[3]){
//   const point3D<float> d = ray.p * v;

//   double gv_d = 0;
//   double g2v_d2 = 0;
//   double gv_ddtd = 0;
//   for (int j = 0; j < 3; j++)
//   {
//    gv_d += vgrad[j] * d[j];
//    for (int l = 0; l < 3; l++)
//      g2v_d2 += vgrad2[j][l] * d[j] * d[l];
//   }

//   for (int i = 0; i < 3; i++)
//   {
//     ray.x[i] += _dt * v * (d[i] + _dt * ( gv_d * d[i] - vgrad[i] / 2 ));

//     const double ddt_d = gv_d * d[i] - vgrad[i];
//     gv_ddtd += vgrad[i] * ddt_d;

//     ray.p[i] += _dt * ddt_d / v;
//   }


//   // second order in p
//   for (int i = 0; i < 3; i++)
//   {
//     const double ddt_d = gv_d * d[i] - vgrad[i];
//     double gv2_d = 0;
//     for (int j = 0; j < 2; j++)
//       gv2_d += vgrad2[i][j]*d[j];

//     const double ddt2_d2 = v*( g2v_d2 * d[i] - gv2_d) + d[i]*gv_ddtd + gv_d * ddt_d;

//     ray.p[i] += _dt*_dt/2 * ddt2_d2/v;
//   }

//   // normalize p to 1/v
//   const double p_norm = sqrt(ray.p[0]*ray.p[0] + ray.p[1]*ray.p[1] + ray.p[2]*ray.p[2]);
//   for (int i = 0; i < 3; i++)
//   {
//      ray.p[i] = ray.p[i] / (p_norm * v);
//    }
//   //cout << "Gradient is " << vgrad[0] << ", " << vgrad[1] << ", " << vgrad[2] << endl;
// }
// /** No descriptions */
// void isoTracingOperator<VelModelType>::TraceTstep_Dynamic_Vinje_One(const double _dt, ray3D & ray, const double& v,
//                                        const point3D<float>& vgrad)
// {
//   double dv_e1 = 0;
//   for (int i = 0; i < 3; i++)
//     dv_e1 += vgrad[i]*ray.e1_half[i];

//   ray.e1 = ray.e1 +ray.p * (dv_e1 * _dt);
// }

// /** No descriptions */
// void isoTracingOperator<VelModelType>::TraceTstep_Dynamic_Vinje_Two(const double _dt, ray3D & ray, const double& v,
//                                        const point3D<float>& vgrad, double vgrad2[3][3])
// {
//   const point3D<float> e2 = VecProd(ray.p, ray.e1);
//   const double det_Q = fabs(ray.Q[0][0]*ray.Q[1][1]-ray.Q[0][1]*ray.Q[1][0]);

//   double V[2][2] = {{0, 0}, {0, 0}};

//   for (int k = 0; k < 3; k++)
//     for (int l = 0; l < 3; l++)
//     {
//       V[0][0] += ray.e1[k] * vgrad2[k][l] * ray.e1[k];
//       V[0][1] += ray.e1[k] * vgrad2[k][l] * e2[k];
//       V[1][0] += e2[k] * vgrad2[k][l] * ray.e1[k];
//       V[1][1] += e2[k] * vgrad2[k][l] * e2[k];
//     }

//   for (int i = 0; i < 2; i++)
//   {
//     for (int j = 0; j < 2; j++)
//     {
//       const double VQ_ij = V[i][0]*ray.Q[0][j] + V[i][1]*ray.Q[1][j];
//       ray.P[i][j] -= _dt * VQ_ij / v;
//       ray.Q[i][j] += _dt * v*v * ray.P[i][j];
//     }
//   }

//   const double det_Q_new = fabs(ray.Q[0][0]*ray.Q[1][1]-ray.Q[0][1]*ray.Q[1][0]);


//   if ( fabs(det_Q_new) > EPS)
//     ray.A *= sqrt( det_Q / det_Q_new); // new and old velocity are missing

//   double dv_e1 = 0;
//   for (int i = 0; i < 3; i++)
//     dv_e1 += vgrad[i]*ray.e1[i];

//   ray.e1_half = ray.e1_half + ray.p * (dv_e1 * _dt);

// }

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_RungeKutta(const double _dt, kinray3D & ray, const int ith){
  const double _1_ov_6 = 1.0/6.0;

  point3D<float> vgrad, vgradxx, vgradxy;
  double v = VelModel->GetProperty(ray.x, vgrad, vgradxx, vgradxy, ith);

//   float v_tmp[4];
//   point3D<float> x_vec_tmp[4];
//   point3D<float> vgrad_tmp[4], vgradxx_tmp[4], vgradxy_tmp[4];
//   for (int n = 0; n < 4; n++)
//     x_vec_tmp[n] = ray.x;
    
//   VelModel->GetProperty(x_vec_tmp, v_tmp, vgrad_tmp, vgradxx_tmp, vgradxy_tmp);
//   v = v_tmp[0];
//   vgrad = vgrad_tmp[0];
//   vgradxx = vgradxx_tmp[0];
//   vgradxy = vgradxy_tmp[0];
  

 //std::cout << "Starting:\n";
  //std::cout << ray.x << " : " << v << std::endl;
  //  std::cout << "vgradxx at " << ray.x << ": " << vgradxx << std::endl;
  //  std::cout << "vgradxy at " << ray.x << ": " << vgradxy << std::endl;
    

//   const double V0 = 1800;
//   const double a = -800;
//   const double b = 300;
//   const double xc = 4300;
//   const double yc = 7650;
//   const double zc = -1500;

//   const double x = ray.x[0];
//   const double y = ray.x[1];
//   const double z = ray.x[2];

//   const double f1 = a*exp( - ( (x-xc)*(x-xc) + (y-yc)*(y-yc) + (z-zc)*(z-zc) ) / (b*b));

//   v = V0 + f1;
  
//   vgrad[0] =  -2.0/(b*b) * (x-xc) * f1;
//   vgrad[1] =  -2.0/(b*b) * (y-yc) * f1;
//   vgrad[2] =  -2.0/(b*b) * (z-zc) * f1;

//   vgradxx[0] = -2.0/(b*b) * ( 1 - 2*(x-xc)*(x-xc)/(b*b)) * f1;
//   vgradxx[1] = -2.0/(b*b) * ( 1 - 2*(y-yc)*(y-yc)/(b*b)) * f1;
//   vgradxx[2] = -2.0/(b*b) * ( 1 - 2*(z-zc)*(z-zc)/(b*b)) * f1;

//   vgradxy[0] = 4*(y-yc)*(z-zc)/(b*b*b*b) * f1;
//   vgradxy[1] = 4*(x-xc)*(z-zc)/(b*b*b*b) * f1;
//   vgradxy[2] = 4*(x-xc)*(y-yc)/(b*b*b*b) * f1;

  const double v_2 = v*v;
  const double v_inv = 1.0/v;

  point3D<float> x_tmp = ray.x;

  double _dt_v_2 = _dt * v*v;
  double _mdt_ov_v = -_dt/v;

  const point3D<float> k1_x(ray.p * _dt_v_2);
  const point3D<float> k1_p(vgrad * _mdt_ov_v);

  v = VelModel->GetProperty(ray.x + k1_x/2, vgrad, vgradxx, vgradxy, ith);
  //std::cout << ray.x + k1_x/2 << " : " << v << std::endl;

  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k2_x((ray.p + k1_p/2) * _dt_v_2);
  const point3D<float> k2_p(vgrad * _mdt_ov_v);

  v = VelModel->GetProperty(ray.x + k2_x/2, vgrad, vgradxx, vgradxy, ith);
  //std::cout << ray.x + k2_x/2 << " : " << v << std::endl;
  v = VelModel->GetProperty(ray.x + k2_x/2, vgrad, vgradxx, vgradxy, ith);
  //std::cout << "again: " << ray.x + k2_x/2 << " : " << v << std::endl;

  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k3_x((ray.p + k2_p/2) * _dt_v_2);
  const point3D<float> k3_p(vgrad * _mdt_ov_v);

  v = VelModel->GetProperty(ray.x + k3_x, vgrad, vgradxx, vgradxy, ith);
  //std::cout << ray.x + k3_x << " : " << v << std::endl;
  
  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k4_x((ray.p + k3_p) * _dt_v_2);
  const point3D<float> k4_p(vgrad * _mdt_ov_v);

  ray.x = ray.x + ( k1_x + (k2_x + k3_x) * 2  + k4_x) * _1_ov_6;
  ray.p = ray.p + ( k1_p + (k2_p + k3_p) * 2  + k4_p) * _1_ov_6;
  ray.v = v;

  // renorm p
  float np = ray.p.norm();
  ray.p = ray.p / (v*np);
}

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_RungeKutta_dynamic(const double _dt, ray3D & ray, const int ith){
  const double _1_ov_6 = 1.0/6.0;
  
  point3D<float> vgrad, vgradxx, vgradxy;
  double v = VelModel->GetProperty(ray.x, vgrad, vgradxx, vgradxy, ith);

  const double v_2 = v*v;
  const double v_inv = 1.0/v;
  // Dynamic Variables

  point3D<float> x_tmp = ray.x;
  point3D<float> p_tmp = ray.p;
//   for (int k = 0; k < 3; k++)
//     {
//       float p_P_tmp(0);
//       float vgrad_Q_tmp(0);
//       for (int i = 0; i < 3; i++)
//         {
//           p_P_tmp += ray.p[i]*P_tmp[i][k];
//           vgrad_Q_tmp += vgrad[i]*Q_tmp[i][k];
//         }
//       for (int i = 0; i < 3; i++)
//         {
//           //Q_tmp[i][k] = ray.Q[i][k];
          
//           ray.Q[i][k] += v_2*(P_tmp[i][k] - 2*v_2*ray.p[i] * p_P_tmp) * _dt;
//         }
//        ray.P[0][k] += v_inv * (  v_inv * vgrad[0] * vgrad_Q_tmp - (vgradxx[0]*Q_tmp[0][k] + vgradxy[2]*Q_tmp[1][k] + vgradxy[1]*Q_tmp[2][k])) * _dt;
//        ray.P[1][k] += v_inv * (  v_inv * vgrad[1] * vgrad_Q_tmp - (vgradxy[2]*Q_tmp[0][k] + vgradxx[1]*Q_tmp[1][k] + vgradxy[0]*Q_tmp[2][k])) * _dt;
//        ray.P[2][k] += v_inv * (  v_inv * vgrad[2] * vgrad_Q_tmp - (vgradxy[1]*Q_tmp[0][k] + vgradxy[0]*Q_tmp[1][k] + vgradxx[2]*Q_tmp[2][k])) * _dt;
//     }

//   const double dt_v2 = _dt * v*v;
//   ray.x[0] += dt_v2 * ray.p[0];
//   ray.x[1] += dt_v2 * ray.p[1];
//   ray.x[2] += dt_v2 * ray.p[2];

//   const double dt_v = _dt / v;
//   ray.p[0] -= dt_v * vgrad[0];
//   ray.p[1] -= dt_v * vgrad[1];
//   ray.p[2] -= dt_v * vgrad[2];

//   ray.v = v;

  double _dt_v_2 = _dt * v*v;
  double _mdt_ov_v = -_dt/v;

  const point3D<float> k1_x(ray.p[0] * _dt_v_2, ray.p[1] * _dt_v_2, ray.p[2] * _dt_v_2);
  const point3D<float> k1_p(vgrad[0] * _mdt_ov_v, vgrad[1] * _mdt_ov_v, vgrad[2] * _mdt_ov_v);

  double k1_Q[3][3];
  double k1_P[3][3];
  for (int k = 0; k < 3; k++)
    {
      double p_P_tmp(0);
      double vgrad_Q_tmp(0);
      for (int i = 0; i < 3; i++)
        {
          p_P_tmp += ray.p[i]*ray.P[i][k];
          vgrad_Q_tmp += vgrad[i]*ray.Q[i][k];
        }
      for (int i = 0; i < 3; i++)
        {
          k1_Q[i][k] = v_2*(ray.P[i][k] - 2*v_2*ray.p[i] * p_P_tmp) * _dt;
        }
       k1_P[0][k] = v_inv * (  v_inv * vgrad[0] * vgrad_Q_tmp - (vgradxx[0]*ray.Q[0][k] + vgradxy[2]*ray.Q[1][k] + vgradxy[1]*ray.Q[2][k])) * _dt;
       k1_P[1][k] = v_inv * (  v_inv * vgrad[1] * vgrad_Q_tmp - (vgradxy[2]*ray.Q[0][k] + vgradxx[1]*ray.Q[1][k] + vgradxy[0]*ray.Q[2][k])) * _dt;
       k1_P[2][k] = v_inv * (  v_inv * vgrad[2] * vgrad_Q_tmp - (vgradxy[1]*ray.Q[0][k] + vgradxy[0]*ray.Q[1][k] + vgradxx[2]*ray.Q[2][k])) * _dt;
    }


  v = VelModel->GetProperty(ray.x + k1_x/2, vgrad, vgradxx, vgradxy, ith);

  //std::cout << ray.x + k1_x/2 << " : " << v << std::endl;

  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k2_x((ray.p + k1_p/2) * _dt_v_2);
  const point3D<float> k2_p(vgrad * _mdt_ov_v);

  double k2_Q[3][3];
  double k2_P[3][3];
  for (int k = 0; k < 3; k++)
    {
      double p_P_tmp(0);
      double vgrad_Q_tmp(0);
      for (int i = 0; i < 3; i++)
        {
          p_P_tmp += ray.p[i]*ray.P[i][k];
          vgrad_Q_tmp += vgrad[i]*ray.Q[i][k];
        }
      for (int i = 0; i < 3; i++)
        {
          k2_Q[i][k] = v_2*(ray.P[i][k] - 2*v_2*ray.p[i] * p_P_tmp) * _dt;
        }
       k2_P[0][k] = v_inv * (  v_inv * vgrad[0] * vgrad_Q_tmp - (vgradxx[0]*ray.Q[0][k] + vgradxy[2]*ray.Q[1][k] + vgradxy[1]*ray.Q[2][k])) * _dt;
       k2_P[1][k] = v_inv * (  v_inv * vgrad[1] * vgrad_Q_tmp - (vgradxy[2]*ray.Q[0][k] + vgradxx[1]*ray.Q[1][k] + vgradxy[0]*ray.Q[2][k])) * _dt;
       k2_P[2][k] = v_inv * (  v_inv * vgrad[2] * vgrad_Q_tmp - (vgradxy[1]*ray.Q[0][k] + vgradxy[0]*ray.Q[1][k] + vgradxx[2]*ray.Q[2][k])) * _dt;
    }

  v = VelModel->GetProperty(ray.x + k2_x/2, vgrad, vgradxx, vgradxy, ith);



  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k3_x((ray.p + k2_p/2) * _dt_v_2);
  const point3D<float> k3_p(vgrad * _mdt_ov_v);

  double k3_Q[3][3];
  double k3_P[3][3];
  for (int k = 0; k < 3; k++)
    {
      double p_P_tmp(0);
      double vgrad_Q_tmp(0);
      for (int i = 0; i < 3; i++)
        {
          p_P_tmp += ray.p[i]*ray.P[i][k];
          vgrad_Q_tmp += vgrad[i]*ray.Q[i][k];
        }
      for (int i = 0; i < 3; i++)
        {
          k3_Q[i][k] = v_2*(ray.P[i][k] - 2*v_2*ray.p[i] * p_P_tmp) * _dt;
        }
       k3_P[0][k] = v_inv * (  v_inv * vgrad[0] * vgrad_Q_tmp - (vgradxx[0]*ray.Q[0][k] + vgradxy[2]*ray.Q[1][k] + vgradxy[1]*ray.Q[2][k])) * _dt;
       k3_P[1][k] = v_inv * (  v_inv * vgrad[1] * vgrad_Q_tmp - (vgradxy[2]*ray.Q[0][k] + vgradxx[1]*ray.Q[1][k] + vgradxy[0]*ray.Q[2][k])) * _dt;
       k3_P[2][k] = v_inv * (  v_inv * vgrad[2] * vgrad_Q_tmp - (vgradxy[1]*ray.Q[0][k] + vgradxy[0]*ray.Q[1][k] + vgradxx[2]*ray.Q[2][k])) * _dt;
    }

  v = VelModel->GetProperty(ray.x + k3_x, vgrad, vgradxx, vgradxy, ith);


  //std::cout << ray.x + k3_x << " : " << v << std::endl;
  
  _dt_v_2 = _dt * v*v;
  _mdt_ov_v = -_dt/v;
  const point3D<float> k4_x((ray.p + k3_p) * _dt_v_2);
  const point3D<float> k4_p(vgrad * _mdt_ov_v);

  double k4_Q[3][3];
  double k4_P[3][3];
  for (int k = 0; k < 3; k++)
    {
      double p_P_tmp(0);
      double vgrad_Q_tmp(0);
      for (int i = 0; i < 3; i++)
        {
          p_P_tmp += ray.p[i]*ray.P[i][k];
          vgrad_Q_tmp += vgrad[i]*ray.Q[i][k];
        }
      for (int i = 0; i < 3; i++)
        {
          k4_Q[i][k] = v_2*(ray.P[i][k] - 2*v_2*ray.p[i] * p_P_tmp) * _dt;
        }
       k4_P[0][k] = v_inv * (  v_inv * vgrad[0] * vgrad_Q_tmp - (vgradxx[0]*ray.Q[0][k] + vgradxy[2]*ray.Q[1][k] + vgradxy[1]*ray.Q[2][k])) * _dt;
       k4_P[1][k] = v_inv * (  v_inv * vgrad[1] * vgrad_Q_tmp - (vgradxy[2]*ray.Q[0][k] + vgradxx[1]*ray.Q[1][k] + vgradxy[0]*ray.Q[2][k])) * _dt;
       k4_P[2][k] = v_inv * (  v_inv * vgrad[2] * vgrad_Q_tmp - (vgradxy[1]*ray.Q[0][k] + vgradxy[0]*ray.Q[1][k] + vgradxx[2]*ray.Q[2][k])) * _dt;
    }

  ray.x = ray.x + ( k1_x + (k2_x + k3_x) * 2  + k4_x) * _1_ov_6;
  ray.p = ray.p + ( k1_p + (k2_p + k3_p) * 2  + k4_p) * _1_ov_6;
  ray.v = v;

  // renorm p
  float np = ray.p.norm();
  ray.p = ray.p / (v*np);

  for (int k = 0; k < 3; k++)
    {
      for (int i = 0; i < 3; i++)
        {
          ray.Q[i][k] = ray.Q[i][k] +  ( k1_Q[i][k] + (k2_Q[i][k] + k3_Q[i][k]) * 2  + k4_Q[i][k]) * _1_ov_6;
          ray.P[i][k] = ray.P[i][k] +  ( k1_P[i][k] + (k2_P[i][k] + k3_P[i][k]) * 2  + k4_P[i][k]) * _1_ov_6;
        }
    }
  // Dynamic Variables, 3rd component
    for (int i = 0; i < 3; i++)
      {
	  ray.Q[i][2] = ray.p[i];//((double)(ray.x[i] - x_tmp[i]))/_dt;
        ray.P[i][2] = ((double)(ray.p[i] - p_tmp[i]))/_dt;
      }
}



template<class VelModelType>
inline void isoTracingOperator<VelModelType>::Eval_Isotrop_at(const point3D<float>& x, const point3D<float>& p, const double Q[3][3], const double P[3][3], 
				     point3D<float>& k_x, point3D<float>& k_p, double k_Q[3][3], double k_P[3][3], const int ith)
{
    // Some Constants
    point3D<float> vgrad, vgradxx, vgradxy;
    double v = VelModel->GetProperty(x, vgrad, vgradxx, vgradxy, ith);

    point3D<float> A11grad, A11gradxx, A11gradxy;
    double A11 = v*v;
    A11grad = vgrad * 2*v;
    A11gradxx = point3D<float>(2*v*vgradxx[0] + 2*vgrad[0]*vgrad[0],
			       2*v*vgradxx[1] + 2*vgrad[1]*vgrad[1],
			       2*v*vgradxx[2] + 2*vgrad[2]*vgrad[2]);
    A11gradxy = point3D<float>(2*v*vgradxy[0] + 2*vgrad[1]*vgrad[2],
			       2*v*vgradxy[1] + 2*vgrad[2]*vgrad[0],
			       2*v*vgradxy[2] + 2*vgrad[0]*vgrad[1]);

    double A33 = v*v;
    point3D<float> A33grad = vgrad * 2*v;


    const double v_2 = v*v;
    const double v_inv = 1.0/v;

    double p1_2 = p[0]*p[0];
    double p2_2 = p[1]*p[1];
    double p3_2 = p[2]*p[2];
    double p1_plus_p2 = p1_2 + p2_2;
    double p1_plus_p2_2 = p1_plus_p2 * p1_plus_p2;
    double p_2 =  p1_plus_p2 + p3_2;
    double p3_4 = p3_2 * p3_2;

    // eval derivatives of G
    point3D<float> dG_dp_over_2;
    point3D<float> dG_dx_over_2;

    point3D<float> ddG_dx1_dx_over_2;
    point3D<float> ddG_dx2_dx_over_2;
    point3D<float> ddG_dx3_dx_over_2;

    point3D<float> ddG_dp1_dx_over_2;
    point3D<float> ddG_dp2_dx_over_2;
    point3D<float> ddG_dp3_dx_over_2;

    point3D<float> ddG_dx1_dp_over_2;
    point3D<float> ddG_dx2_dp_over_2;
    point3D<float> ddG_dx3_dp_over_2;

    point3D<float> ddG_dp1_dp_over_2;
    point3D<float> ddG_dp2_dp_over_2;
    point3D<float> ddG_dp3_dp_over_2;

    
   dG_dp_over_2 =  point3D<float> (p[0] * (A11), 
				    p[1] * (A11), 
				    p[2] * (A33) );

    dG_dx_over_2 =  point3D<float> ( ( A11grad[0] * p1_plus_p2 + A33grad[0] * p3_2) * 0.5,
				     ( A11grad[1] * p1_plus_p2 + A33grad[1] * p3_2) * 0.5,
				     ( A11grad[2] * p1_plus_p2 + A33grad[2] * p3_2) * 0.5 );

    ddG_dp1_dp_over_2 = point3D<float> ( A11,
					 0.0f,
					 0.0f );

    ddG_dp2_dp_over_2 = point3D<float> ( ddG_dp1_dp_over_2[1],
					 A11,
					 0.0f);

    ddG_dp3_dp_over_2 = point3D<float> ( ddG_dp1_dp_over_2[2],
					 ddG_dp2_dp_over_2[2],
					 A33 );

    for (int j = 0; j < 3; j++)
    {
	ddG_dp1_dx_over_2[j] = p[0] * ( A11grad[j] );
	ddG_dp2_dx_over_2[j] = p[1] * ( A11grad[j] );
	ddG_dp3_dx_over_2[j] = p[2] * ( A33grad[j] );
    }

    ddG_dx1_dx_over_2[0] = A11gradxx[0] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[0] + v*vgradxx[0]) * p3_2;
    ddG_dx1_dx_over_2[1] = A11gradxy[2] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[1] + v*vgradxy[2]) * p3_2;
    ddG_dx1_dx_over_2[2] = A11gradxy[1] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[2] + v*vgradxy[1]) * p3_2;

    ddG_dx2_dx_over_2[0] = A11gradxy[2] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[1] + v*vgradxy[2]) * p3_2;
    ddG_dx2_dx_over_2[1] = A11gradxx[1] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[1] + v*vgradxx[1]) * p3_2;
    ddG_dx2_dx_over_2[2] = A11gradxy[0] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[2] + v*vgradxy[0]) * p3_2;

    ddG_dx3_dx_over_2[0] = A11gradxy[1] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[2] + v*vgradxy[1]) * p3_2;
    ddG_dx3_dx_over_2[1] = A11gradxy[0] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[2] + v*vgradxy[0]) * p3_2;
    ddG_dx3_dx_over_2[2] = A11gradxx[2] * p1_plus_p2 + 2.0 * (vgrad[2]*vgrad[2] + v*vgradxx[2]) * p3_2;

    ddG_dx1_dp_over_2 = point3D<float> ( ddG_dp1_dx_over_2[0], ddG_dp2_dx_over_2[0], ddG_dp3_dx_over_2[0]);
    ddG_dx2_dp_over_2 = point3D<float> ( ddG_dp1_dx_over_2[1], ddG_dp2_dx_over_2[1], ddG_dp3_dx_over_2[1]);
    ddG_dx3_dp_over_2 = point3D<float> ( ddG_dp1_dx_over_2[2], ddG_dp2_dx_over_2[2], ddG_dp3_dx_over_2[2]);


    k_x = dG_dp_over_2;
    k_p  = dG_dx_over_2 * (-1.0f);


    
    for (int k = 0; k < 3; k++)
    {
	k_Q[0][k] = 0;
	k_Q[1][k] = 0;
	k_Q[2][k] = 0;
	
	k_P[0][k] = 0;
	k_P[1][k] = 0;
	k_P[2][k] = 0;
    }

    for (int i = 0; i < 3; i++)
    {
	for (int k = 0; k < 3; k++)
	{
	    k_Q[0][k] += (ddG_dp1_dx_over_2[i] * Q[i][k] + ddG_dp1_dp_over_2[i] * P[i][k]);
	    k_Q[1][k] += (ddG_dp2_dx_over_2[i] * Q[i][k] + ddG_dp2_dp_over_2[i] * P[i][k]);
	    k_Q[2][k] += (ddG_dp3_dx_over_2[i] * Q[i][k] + ddG_dp3_dp_over_2[i] * P[i][k]);
			 		   		  				      			  
	    k_P[0][k] -= (ddG_dx1_dx_over_2[i] * Q[i][k] + ddG_dx1_dp_over_2[i] * P[i][k]);
	    k_P[1][k] -= (ddG_dx2_dx_over_2[i] * Q[i][k] + ddG_dx2_dp_over_2[i] * P[i][k]);
	    k_P[2][k] -= (ddG_dx3_dx_over_2[i] * Q[i][k] + ddG_dx3_dp_over_2[i] * P[i][k]);
	}
    }

} 

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_IsotropicRungeKutta_dynamic(const double _dt, ray3D & ray, const int ith)
{
    const double _1_ov_6 = 1.0/6.0;

    point3D<float> vgrad, vgradxx, vgradxy;
    double v = VelModel->GetProperty(ray.x, vgrad, vgradxx, vgradxy, ith);

    point3D<float> k1_x, k2_x, k3_x, k4_x;
    point3D<float> k1_p, k2_p, k3_p, k4_p;

    double k1_Q[3][3];
    double k1_P[3][3];

    double k2_Q[3][3];
    double k2_P[3][3];

    double k3_Q[3][3];
    double k3_P[3][3];

    double k4_Q[3][3];
    double k4_P[3][3];

    double Q[3][3];
    double P[3][3];
    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k];
	    P[i][k] = ray.P[i][k];
        }

    point3D<float> x_tmp = ray.x;
    point3D<float> p_tmp = ray.p;

    Eval_Isotrop_at(ray.x, ray.p, Q, P, k1_x, k1_p, k1_Q, k1_P, ith);
   
    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + 0.5 * k1_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + 0.5 * k1_P[i][k] * (float)_dt;
        }
    Eval_Isotrop_at(ray.x + k1_x * 0.5 * (float)_dt, ray.p + k1_p * 0.5 * (float)_dt, Q, P, k2_x, k2_p, k2_Q, k2_P, ith);

    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + 0.5 * k2_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + 0.5 * k2_P[i][k] * (float)_dt;
        }
    Eval_Isotrop_at(ray.x + k2_x * 0.5 * (float)_dt, ray.p + k2_p * 0.5 * (float)_dt, Q, P, k3_x, k3_p, k3_Q, k3_P, ith);

    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + k3_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + k3_P[i][k] * (float)_dt;
        }
    Eval_Isotrop_at(ray.x + k3_x * (float)_dt, ray.p +  k3_p * (float)_dt, Q, P, k4_x, k4_p, k4_Q, k4_P, ith);

    ray.x = ray.x + ( k1_x + (k2_x + k3_x) * 2  + k4_x) * (float)_dt * _1_ov_6;
    ray.p = ray.p + ( k1_p + (k2_p + k3_p) * 2  + k4_p) * (float)_dt * _1_ov_6;
    ray.v = v;

    for (int k = 0; k < 3; k++)
    {
	for (int i = 0; i < 3; i++)
        {
	    ray.Q[i][k] = ray.Q[i][k] +  _dt * ( k1_Q[i][k] + (k2_Q[i][k] + k3_Q[i][k]) * 2  + k4_Q[i][k]) * _1_ov_6;
	    ray.P[i][k] = ray.P[i][k] +  _dt * ( k1_P[i][k] + (k2_P[i][k] + k3_P[i][k]) * 2  + k4_P[i][k]) * _1_ov_6;
        }
    }

    // Dynamic Variables, 3rd component
    for (int i = 0; i < 3; i++)
    {
	ray.Q[i][2] = (ray.p * ray.p) * ((double)(ray.x[i] - x_tmp[i]))/_dt;
	ray.P[i][2] = ((double)(ray.p[i] - p_tmp[i]))/_dt;
    }
}

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_RungeKutta(const double _dt, kinray3D* rays[4], const int ith){
  const double _1_ov_6 = 1.0/6.0;

  float v[4];
  point3D<float> x_vec[4];
  point3D<float> vgrad[4], vgradxx[4], vgradxy[4];
  for (int n = 0; n < 4; n++)
    x_vec[n] = rays[n]->x;
    
  VelModel->GetProperty(x_vec, v, vgrad, vgradxx, vgradxy, ith);


  double _dt_v_2[4];
  double _mdt_ov_v[4];
  point3D<float> k1_x[4], k1_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt * v[n]*v[n];
    _mdt_ov_v[n] = -_dt/v[n];
    k1_x[n] = rays[n]->p * _dt_v_2[n];
    k1_p[n] = vgrad[n] * _mdt_ov_v[n];
  }

  point3D<float> x_vec1[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec1[n] = x_vec[n] + k1_x[n]/2;
  }  
  VelModel->GetProperty(x_vec1, v, vgrad, ith);

  point3D<float> k2_x[4], k2_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt * v[n]*v[n];
    _mdt_ov_v[n] = -_dt/v[n];
    k2_x[n] = (rays[n]->p + k1_p[n]/2) * _dt_v_2[n];
    k2_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec2[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec2[n] = x_vec[n] + k2_x[n]/2;
  }
  VelModel->GetProperty(x_vec2, v, vgrad, ith);

  point3D<float> k3_x[4], k3_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt * v[n]*v[n];
    _mdt_ov_v[n] = -_dt/v[n];
    k3_x[n] = (rays[n]->p + k2_p[n]/2) * _dt_v_2[n];
    k3_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec3[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec3[n] = x_vec[n] + k3_x[n];
  }
  VelModel->GetProperty(x_vec3, v, vgrad, ith);

  point3D<float> k4_x[4], k4_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt * v[n]*v[n];
    _mdt_ov_v[n] = -_dt/v[n];
    k4_x[n] = (rays[n]->p + k3_p[n]) * _dt_v_2[n];
    k4_p[n] = vgrad[n] * _mdt_ov_v[n];
  }




  for (int n = 0; n < 4; n++)
    {
      rays[n]->x = rays[n]->x + ( k1_x[n] + (k2_x[n] + k3_x[n]) * 2  + k4_x[n]) * _1_ov_6;
      rays[n]->p = rays[n]->p + ( k1_p[n] + (k2_p[n] + k3_p[n]) * 2  + k4_p[n]) * _1_ov_6;
      rays[n]->v = v[n];
    }

  // renorm p
  for (int n = 0; n < 4; n++)
    {
      rays[n]->p = rays[n]->p / (v[n] * (rays[n]->p).norm() );
    } 

}

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_RungeKutta(const double _dt[4], kinray3D rays[4], const int ith){
  const double _1_ov_6 = 1.0/6.0;

  float v[4];
  point3D<float> x_vec[4];
  point3D<float> vgrad[4];
  for (int n = 0; n < 4; n++)
    x_vec[n] = rays[n].x;

  VelModel->GetProperty(x_vec, v, vgrad, ith);

  double _dt_v_2[4];
  double _mdt_ov_v[4];
  point3D<float> k1_x[4], k1_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k1_x[n] = rays[n].p * _dt_v_2[n];
    k1_p[n] = vgrad[n] * _mdt_ov_v[n];
  }

  point3D<float> x_vec1[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec1[n] = x_vec[n] + k1_x[n]/2;
  }
  VelModel->GetProperty(x_vec1, v, vgrad, ith);

  point3D<float> k2_x[4], k2_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k2_x[n] =(rays[n].p + k1_p[n]/2) * _dt_v_2[n];
    k2_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec2[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec2[n] = x_vec[n] + k2_x[n]/2;
  }
  VelModel->GetProperty(x_vec2, v, vgrad, ith);

  point3D<float> k3_x[4], k3_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k3_x[n] = (rays[n].p + k2_p[n]/2) * _dt_v_2[n];
    k3_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec3[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec3[n] = x_vec[n] + k3_x[n];
  }
  VelModel->GetProperty(x_vec3, v, vgrad, ith);

  point3D<float> k4_x[4], k4_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k4_x[n] = (rays[n].p + k3_p[n]) * _dt_v_2[n];
    k4_p[n] = vgrad[n] * _mdt_ov_v[n];
  }




  for (int n = 0; n < 4; n++)
  {
    rays[n].x = rays[n].x + ( k1_x[n] + (k2_x[n] + k3_x[n]) * 2  + k4_x[n]) * _1_ov_6;
    rays[n].p = rays[n].p + ( k1_p[n] + (k2_p[n] + k3_p[n]) * 2  + k4_p[n]) * _1_ov_6;
    rays[n].v = v[n];
  }


  // renorm p
  for (int n = 0; n < 4; n++)
  {
    rays[n].p = rays[n].p / (v[n] * (rays[n].p).norm() );
  }
}

template<class VelModelType>
inline void isoTracingOperator<VelModelType>::TraceTstep_RungeKutta(const double _dt[4], ray3D rays[4], const int ith){
  const double _1_ov_6 = 1.0/6.0;

  float v[4];
  point3D<float> x_vec[4];
  point3D<float> vgrad[4];
  for (int n = 0; n < 4; n++)
    x_vec[n] = rays[n].x;

  VelModel->GetProperty(x_vec, v, vgrad, ith);

  double _dt_v_2[4];
  double _mdt_ov_v[4];
  point3D<float> k1_x[4], k1_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k1_x[n] = rays[n].p * _dt_v_2[n];
    k1_p[n] = vgrad[n] * _mdt_ov_v[n];
  }

  point3D<float> x_vec1[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec1[n] = x_vec[n] + k1_x[n]/2;
  }
  VelModel->GetProperty(x_vec1, v, vgrad, ith);

  point3D<float> k2_x[4], k2_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k2_x[n] =(rays[n].p + k1_p[n]/2) * _dt_v_2[n];
    k2_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec2[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec2[n] = x_vec[n] + k2_x[n]/2;
  }
  VelModel->GetProperty(x_vec2, v, vgrad, ith);

  point3D<float> k3_x[4], k3_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k3_x[n] = (rays[n].p + k2_p[n]/2) * _dt_v_2[n];
    k3_p[n] = vgrad[n] * _mdt_ov_v[n];
  }
  point3D<float> x_vec3[4];
  for (int n = 0; n < 4; n++)
  {
    x_vec3[n] = x_vec[n] + k3_x[n];
  }
  VelModel->GetProperty(x_vec3, v, vgrad, ith);

  point3D<float> k4_x[4], k4_p[4];
  for (int n = 0; n < 4; n++)
  {
    _dt_v_2[n] = _dt[n] * v[n]*v[n];
    _mdt_ov_v[n] = -_dt[n]/v[n];
    k4_x[n] = (rays[n].p + k3_p[n]) * _dt_v_2[n];
    k4_p[n] = vgrad[n] * _mdt_ov_v[n];
  }




  for (int n = 0; n < 4; n++)
  {
    rays[n].x = rays[n].x + ( k1_x[n] + (k2_x[n] + k3_x[n]) * 2  + k4_x[n]) * _1_ov_6;
    rays[n].p = rays[n].p + ( k1_p[n] + (k2_p[n] + k3_p[n]) * 2  + k4_p[n]) * _1_ov_6;
    rays[n].v = v[n];
  }


  // renorm p
  for (int n = 0; n < 4; n++)
  {
    rays[n].p = rays[n].p / ( v[n] * (rays[n].p).norm() );
  }
}

/** No descriptions */
template<class VelModelType>
void isoTracingOperator<VelModelType>::TraceTstep(const double _dt, ray3D & ray, const double& v, const point3D<float>& vgrad, const int ith){
  const double dt_v2 = _dt * v*v;
  ray.x[0] += dt_v2 * ray.p[0];
  ray.x[1] += dt_v2 * ray.p[1];
  ray.x[2] += dt_v2 * ray.p[2];

  const double dt_v = _dt / v;
  ray.p[0] -= dt_v * vgrad[0];
  ray.p[1] -= dt_v * vgrad[1];
  ray.p[2] -= dt_v * vgrad[2];
}
/** */

template<class VelModelType>
void isoTracingOperator<VelModelType>::TraceDistance(ray3D* ray[4], const float l[4], const int t, float _tt[4], const int ith)
{
//  std::cout << "In TraceDistance\n";

  float v[4];
  double _dt_tmp[4];
  ray3D ray_tmp[4] = {*ray[0], *ray[1], *ray[2], *ray[3]};
  point3D<float> xvec[4];

  for (int i = 0; i < 4; i++)
  {
      //ray_tmp[i] = ray[i];
   xvec[i] = ray_tmp[i].x  + ray_tmp[i].p * (l[i]/(2.0 * (ray_tmp[i].p).norm() ));
  }

  VelModel->GetProperty(xvec, v, ith);
  for (int i = 0; i < 4; i++)
    _dt_tmp[i] = l[i]/v[i];

  TraceTstep_RungeKutta(_dt_tmp, ray_tmp, ith);

  for (int i = 0; i < 4; i++)
  {
    if (fabs(l[i]) > 1e-6)
      {
        const double _dt_new = (ray[i]->x-ray_tmp[i].x).norm() / l[i] * _dt_tmp[i];
        _tt[i] = t*dtstep_max + _dt_new;
      }
    else 
      _tt[i] = t*dtstep_max;
  }  
}

template<class VelModelType>
double isoTracingOperator<VelModelType>::TraceDistance(ray3D& ray, const double l, const int t, const int ith)
{
  if (fabs(l) > 1e-6)
    {
      ray3D ray_tmp = ray;

      point3D<float> dummy;
      float v = VelModel->GetProperty(ray.x + ray.p * (l/(2.0 * (ray.p).norm() )), dummy, ith);
      const double _dt = min(l/v, (double) dtstep_max);

      DRT(_dt,  ray_tmp, ith);

      const double _dt_new = min( (ray.x-ray_tmp.x).norm() /l * _dt, (double) dtstep_max);

      DRT(_dt_new, ray, ith);

      ray.detQ = (ray.Q[0][0]*ray.Q[1][1]*ray.Q[2][2] + ray.Q[0][1]*ray.Q[1][2]*ray.Q[2][0] + ray.Q[0][2]*ray.Q[1][0]*ray.Q[2][1] - 
                  ray.Q[0][0]*ray.Q[2][1]*ray.Q[1][2] - ray.Q[0][1]*ray.Q[1][0]*ray.Q[2][2] - ray.Q[0][2]*ray.Q[1][1]*ray.Q[2][0]);
      return t*dtstep_max + _dt_new;
    }
  else
    return t*dtstep_max;   //return t*dt*nt + _dt;
}

template<class VelModelType>
double isoTracingOperator<VelModelType>::TraceToSurface(ray3D& ray, const float finaldepth, const int t, const int ith)
{
    float dt_correction = 0;
    float dt_tmp = dtstep_max/2;
    while (dt_tmp > 0.0001f)
    {
	ray.Store();
	DRT(dt_tmp, ray, ith);
	if ( ray.x[2] >= finaldepth )
	{
	    ray.Restore();
	}
	else
	{
	    dt_correction += dt_tmp;
	}
	dt_tmp = dt_tmp / 2;
    }

    ray.detQ = (ray.Q[0][0]*ray.Q[1][1]*ray.Q[2][2] + ray.Q[0][1]*ray.Q[1][2]*ray.Q[2][0] + ray.Q[0][2]*ray.Q[1][0]*ray.Q[2][1] - 
		ray.Q[0][0]*ray.Q[2][1]*ray.Q[1][2] - ray.Q[0][1]*ray.Q[1][0]*ray.Q[2][2] - ray.Q[0][2]*ray.Q[1][1]*ray.Q[2][0]);

    return t*dtstep_max + dt_correction;
}

template<class VelModelType>
double isoTracingOperator<VelModelType>::TraceToPoint(ray3D& ray, const point3D<float>& DestPoint, const int ith)
{
    float distprod_old = 1.0f;
    float distprod_new = 1.0f;
    float distprod = 1.0f;
    float dt = 0.0f;
    float dt_step = dtstep_max/20;

    while( (distprod > 0) && (dt < dtstep_max))
    {
      ray.Store();
      DRT(dt_step, ray, ith);
	dt += dt_step;
	distprod_new = ((DestPoint - ray.x) * ray.p);
	distprod_old = ((DestPoint - ray.x_old) * ray.p_old);
	distprod = distprod_old*distprod_new;
    }
    dt = dt - fabs(distprod_new)/(fabs(distprod_old) + fabs(distprod_new)) * dt_step;
    return min(dt, dtstep_max);
}

template<class VelModelType>
float isoTracingOperator<VelModelType>::GetTT(const float dt, const int tstepindex)
{
    return tstepindex*dtstep_max + dt;
}

template<class VelModelType>
void isoTracingOperator<VelModelType>::TraceDistance(kinray3D& ray, const double l, const int ith)
{
  if (fabs(l) > 1e-6)
    {
      kinray3D ray_tmp = ray;

      point3D<float> dummy;
      float v = VelModel->GetProperty(ray.x + ray.p * (l/(2.0 * (ray.p).norm() )), dummy, ith);
      const double _dt = min(l/v, (double) dtstep_max);

      TraceTstep_RungeKutta(_dt, ray_tmp, ith);
      const double _dt_new = min( (ray.x-ray_tmp.x).norm() /l * _dt, (double) dtstep_max);

      TraceTstep_RungeKutta(_dt_new, ray, ith);
    }
}

template<class VelModelType>
void isoTracingOperator<VelModelType>::BindModels(const VelModelRep_t& _VModels)
{
    VelModel = _VModels.V;  
}

template<class VelModelType>
void isoTracingOperator<VelModelType>::GetVelocityAt(float& v, const point3D<float>& Point, const int ith)
{
    v = VelModel->GetProperty(Point, ith);
}


