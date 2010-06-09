/***************************************************************************
                          vtitracingoperator.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


template<class VelModelType>
vtiTracingOperator<VelModelType>::vtiTracingOperator():VelModel(NULL),
						 VhModel(NULL),
						 ExModel(NULL)
{
    dtstep_max = 0;
}

template<class VelModelType>
vtiTracingOperator<VelModelType>::~vtiTracingOperator(){
}

/** operator () (const vtiTracingOperator& ) */
template<class VelModelType>
void vtiTracingOperator<VelModelType>::operator () (ray3D& ray, const int ith)
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

/** operator () (const vtiTracingOperator& ) */
template<class VelModelType>
void vtiTracingOperator<VelModelType>::operator () (ray3D* rays[4], const int ith)
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

/** No descriptions */
template<class VelModelType>
void vtiTracingOperator<VelModelType>::Init(const float& _dt){
  if (_dt <= 0)
    {
      std::cerr << "FATAL ERROR in vtiTracingOperator<VelModelType>::Init\n";
      std::cerr << "            The time interval _dt = " << _dt << std::endl;
      std::cerr << "            is <= 0.\n";
      exit(1);
    }
  dtstep_max = _dt;
}

/** No descriptions */
template<class VelModelType>
inline void vtiTracingOperator<VelModelType>::Eval_at(const point3D<float>& x, const point3D<float>& p, const double Q[3][3], const double P[3][3], 
				     point3D<float>& k_x, point3D<float>& k_p, double k_Q[3][3], double k_P[3][3], const int ith)
{
    // Some Constants
    point3D<float> vgrad, vgradxx, vgradxy;
    double v = VelModel->GetProperty(x, vgrad, vgradxx, vgradxy, ith);

    point3D<float> Exgrad, Exgradxx, Exgradxy;
    double Ex = ExModel->GetProperty(x, Exgrad, Exgradxx, Exgradxy, ith);

    point3D<float> Vhgrad, Vhgradxx, Vhgradxy;
    double Vh = VhModel->GetProperty(x, Vhgrad, Vhgradxx, Vhgradxy, ith);

    point3D<float> A11grad, A11gradxx, A11gradxy;
    double A11 = Vh*Vh;
    A11grad = Vhgrad * 2*Vh;
    A11gradxx[0] = Vhgradxx[0] * 2*Vh + 2 * Vhgradxx[0] * Vhgradxx[0];
    A11gradxx[1] = Vhgradxx[1] * 2*Vh + 2 * Vhgradxx[1] * Vhgradxx[1];
    A11gradxx[2] = Vhgradxx[2] * 2*Vh + 2 * Vhgradxx[2] * Vhgradxx[2];
    A11gradxy[0] = Vhgradxy[0] * 2*Vh + 2 * Vhgradxy[1] * Vhgradxy[2];
    A11gradxy[1] = Vhgradxy[1] * 2*Vh + 2 * Vhgradxy[2] * Vhgradxy[0];
    A11gradxy[2] = Vhgradxy[2] * 2*Vh + 2 * Vhgradxy[0] * Vhgradxy[1];

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
    double Ex_over_p4 = Ex / (p_2*p_2);
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

    dG_dp_over_2 =  point3D<float> (p[0] * (A11 + Ex_over_p4 * p3_4), 
				    p[1] * (A11 + Ex_over_p4 * p3_4), 
				    p[2] * (A33 + Ex_over_p4 * p1_plus_p2_2) );

    dG_dx_over_2 =  point3D<float> ( ( Exgrad[0] * p1_plus_p2 * p3_2 / p_2 + A11grad[0] * p1_plus_p2 + A33grad[0] * p3_2) * 0.5,
				     ( Exgrad[1] * p1_plus_p2 * p3_2 / p_2 + A11grad[1] * p1_plus_p2 + A33grad[1] * p3_2) * 0.5,
				     ( Exgrad[2] * p1_plus_p2 * p3_2 / p_2 + A11grad[2] * p1_plus_p2 + A33grad[2] * p3_2) * 0.5);

    ddG_dp1_dp_over_2 = point3D<float> ( A11 + Ex_over_p4 * p3_4 * (1.0f - 4.0f * p1_2 /  p_2),
					 -4.0f * Ex_over_p4 * p[0] * p[1] * p3_4 / p_2,
					 4.0f * Ex_over_p4 * p[0] * p[2] * p3_2 * ( 1.0f - p3_2 / p_2) );

    ddG_dp2_dp_over_2 = point3D<float> ( ddG_dp1_dp_over_2[1],
					 A11 + Ex_over_p4 * p3_4 * (1.0f - 4.0f * p2_2 /  p_2),
					 4.0f * Ex_over_p4 * p[1] * p[2] * p3_2 * ( 1.0f - p3_2 / p_2) );

    ddG_dp3_dp_over_2 = point3D<float> ( ddG_dp1_dp_over_2[2],
					 ddG_dp2_dp_over_2[2],
					 A33 + Ex_over_p4 * p1_plus_p2_2 * ( 1.0f - 4.0f * p3_2 /  p_2) );

    for (int j = 0; j < 3; j++)
    {
	ddG_dp1_dx_over_2[j] = p[0] * ( A11grad[j] + Exgrad[j] * p3_4 / (p_2*p_2));
	ddG_dp2_dx_over_2[j] = p[1] * ( A11grad[j] + Exgrad[j] * p3_4 / (p_2*p_2));
	ddG_dp3_dx_over_2[j] = p[2] * ( A33grad[j] + Exgrad[j] * p1_plus_p2_2 / (p_2*p_2));
    }

    ddG_dx1_dx_over_2[0] = Exgradxx[0] * p1_plus_p2 * p3_2 / p_2 + A11gradxx[0] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[0] + v*vgradxx[0]) * p3_2;
    ddG_dx1_dx_over_2[1] = Exgradxy[2] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[2] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[1] + v*vgradxy[2]) * p3_2;
    ddG_dx1_dx_over_2[2] = Exgradxy[1] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[1] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[2] + v*vgradxy[1]) * p3_2;

    ddG_dx2_dx_over_2[0] = Exgradxy[2] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[2] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[1] + v*vgradxy[2]) * p3_2;
    ddG_dx2_dx_over_2[1] = Exgradxx[1] * p1_plus_p2 * p3_2 / p_2 + A11gradxx[1] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[1] + v*vgradxx[1]) * p3_2;
    ddG_dx2_dx_over_2[2] = Exgradxy[0] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[0] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[2] + v*vgradxy[0]) * p3_2;

    ddG_dx3_dx_over_2[0] = Exgradxy[1] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[1] * p1_plus_p2 + 2.0 * (vgrad[0]*vgrad[2] + v*vgradxy[1]) * p3_2;
    ddG_dx3_dx_over_2[1] = Exgradxy[0] * p1_plus_p2 * p3_2 / p_2 + A11gradxy[0] * p1_plus_p2 + 2.0 * (vgrad[1]*vgrad[2] + v*vgradxy[0]) * p3_2;
    ddG_dx3_dx_over_2[2] = Exgradxx[2] * p1_plus_p2 * p3_2 / p_2 + A11gradxx[2] * p1_plus_p2 + 2.0 * (vgrad[2]*vgrad[2] + v*vgradxx[2]) * p3_2;

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
inline void vtiTracingOperator<VelModelType>::TraceTstep_AnisotropicRungeKutta_dynamic(const double _dt, ray3D & ray, const int ith)
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

    Eval_at(ray.x, ray.p, Q, P, k1_x, k1_p, k1_Q, k1_P, ith);

    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + 0.5 * k1_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + 0.5 * k1_P[i][k] * (float)_dt;
        }
    Eval_at(ray.x + k1_x * 0.5 * (float)_dt, ray.p + k1_p * 0.5 * (float)_dt, Q, P, k2_x, k2_p, k2_Q, k2_P, ith);

    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + 0.5 * k2_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + 0.5 * k2_P[i][k] * (float)_dt;
        }
    Eval_at(ray.x + k2_x * 0.5 * (float)_dt, ray.p + k2_p * 0.5 * (float)_dt, Q, P, k3_x, k3_p, k3_Q, k3_P, ith);

    for (int i = 0; i < 3; i++)
	for (int k = 0; k < 3; k++)
        {
	    Q[i][k] = ray.Q[i][k] + k3_Q[i][k] * (float)_dt;
	    P[i][k] = ray.P[i][k] + k3_P[i][k] * (float)_dt;
        }
    Eval_at(ray.x + k3_x * (float)_dt, ray.p +  k3_p * (float)_dt, Q, P, k4_x, k4_p, k4_Q, k4_P, ith);

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
double vtiTracingOperator<VelModelType>::TraceToSurface(ray3D& ray, const float finaldepth, const int t, const int ith)
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
double vtiTracingOperator<VelModelType>::TraceToPoint(ray3D& ray, const point3D<float>& DestPoint, const int ith)
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
float vtiTracingOperator<VelModelType>::GetTT(const float dt, const int tstepindex)
{
    return tstepindex*dtstep_max + dt;
}

template<class VelModelType>
void vtiTracingOperator<VelModelType>::BindModels(const VelModelRep_t& VModels)
{
    VelModel = VModels.V;  
    VhModel = VModels.Vh;
    ExModel = VModels.Ex;
}

template<class VelModelType>
void vtiTracingOperator<VelModelType>::GetVelocityAt(vti_velocity& vti, const point3D<float>& Point, const int ith)
{
    vti.v = VelModel->GetProperty(Point, ith);
    vti.A11 = VhModel->GetProperty(Point, ith)*this->VhModel->GetProperty(Point, ith);
    vti.Ex = ExModel->GetProperty(Point, ith);
}

