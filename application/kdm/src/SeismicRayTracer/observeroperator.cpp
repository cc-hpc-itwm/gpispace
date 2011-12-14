/***************************************************************************
                          observeroperator.cpp  -  description
                             -------------------
    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


// #include "observeroperator.h"

template<class TracingOperator_T> 
ObserverOperator<TracingOperator_T>::ObserverOperator():Tracer(NULL),kmah(0),ith(0){
}
template<class TracingOperator_T> 
ObserverOperator<TracingOperator_T>::ObserverOperator(TracingOperator_T* TrOp, const int _ith):kmah(0), ith(_ith){
  Tracer = TrOp;
}
template<class TracingOperator_T> 
ObserverOperator<TracingOperator_T>::~ObserverOperator(){
}
template<class TracingOperator_T> 
void ObserverOperator<TracingOperator_T>::operator()(const Triangle* Tri, Receiver* Recv, const int& _Tstep)
{
  const RecSig Signal(SignIntpol(Tri, Recv, _Tstep));
  if (Signal.GetT() > 0)
      Recv->SaveSig(Signal, _Tstep);
}
template<class TracingOperator_T> 
void ObserverOperator<TracingOperator_T>::operator()(const Triangle* Tri, Receiver* Recv[4], const int& _Tstep)
{
  RecSig Signals[4];
  SignIntpol(Tri, Recv, _Tstep, Signals);  
  for (int i = 0; i < 4; i++)
    {
	if (Signals[i].GetT() > 0)
	    Recv[i]->SaveSig(Signals[i], _Tstep);
    }
}
template<class TracingOperator_T> 
void ObserverOperator<TracingOperator_T>::Init(const Triangle* Tri)
{
  X0_n = point3D<float>(Tri->points[0]->mainray.x[0], Tri->points[0]->mainray.x[1], Tri->points[0]->mainray.x[2]);
  X1_n = point3D<float>(Tri->points[1]->mainray.x[0], Tri->points[1]->mainray.x[1], Tri->points[1]->mainray.x[2]);
  X2_n = point3D<float>(Tri->points[2]->mainray.x[0], Tri->points[2]->mainray.x[1], Tri->points[2]->mainray.x[2]);

  X0_o = point3D<float>(Tri->points[0]->mainray.x_old[0], Tri->points[0]->mainray.x_old[1],Tri->points[0]->mainray.x_old[2]);
  X1_o = point3D<float>(Tri->points[1]->mainray.x_old[0], Tri->points[1]->mainray.x_old[1], Tri->points[1]->mainray.x_old[2]);
  X2_o = point3D<float>(Tri->points[2]->mainray.x_old[0], Tri->points[2]->mainray.x_old[1], Tri->points[2]->mainray.x_old[2]);
  
  mid_old = X0_o;
  mid_new = X0_n;
  for (int i = 0; i < 3; i++)
    {
      mid_old[i] = (mid_old[i] + X1_o[i] + X2_o[i])/3.0;
      mid_new[i] = (mid_new[i] + X1_n[i] + X2_n[i])/3.0;
    }

  //Normal of old wave face
//   n_old = VecProd(X1_o - X0_o, X2_o - X0_o);
  n_old = (X1_o - X0_o).VecProd(X2_o - X0_o);

  if (n_old.norm() > 1.0e-6)
    n_old = n_old / (n_old.norm());

  // Normal of new wave face
//   n_new = VecProd(X1_n - X0_n, X2_n - X0_n);
  n_new = (X1_n - X0_n).VecProd(X2_n - X0_n);

  if ( n_new.norm() > 1.0e-6)
  n_new = n_new / (n_new.norm());


  if ( (Tri->points[0]->mainray.kmah == Tri->points[1]->mainray.kmah) && (Tri->points[0]->mainray.kmah == Tri->points[2]->mainray.kmah))
    kmah = Tri->points[0]->mainray.kmah;
  else
    kmah = -1;
};
/** No descriptions */
template<class TracingOperator_T> 
bool ObserverOperator<TracingOperator_T>::RecvInTube(const Triangle* Tri, const Receiver* Recv){
  
  const point3D<float> P = point3D<float>(Recv->pos[0], Recv->pos[1], Recv->pos[2]);

  //if (kmah != -1)
    {

      //Check if receiver and new wave face are on the same side of the old wave face
      if ( (((P - mid_old) * n_old) * ((mid_new - mid_old) * n_old) > 0) &&
           (((P - mid_new) * n_new) * ((mid_old - mid_new) * n_new) > 0) )
        {
          //     const point3D<float> perp = P - mid_old - n_old * ((P - mid_old) * n_old);
          const float perp_2 = (P - mid_old)*(P - mid_old) - ((P - mid_old) * n_old)*((P - mid_old) * n_old);
          //std::cout << " perp : " << perp << std::endl;

          // calc circumcircle
          const float a = (X1_o - X0_o).norm(); //sqrt((X1_o - X0_o) * (X1_o - X0_o));
          const float b = (X2_o - X1_o).norm(); //sqrt((X2_o - X1_o) * (X2_o - X1_o));
          const float c = (X0_o - X2_o).norm(); //sqrt((X0_o - X2_o) * (X0_o - X2_o));
          const float s = (a+b+c)/2;
          const float A = sqrt(s*(s-a)*(s-b)*(s-c));
          const float R = (a*b*c)/(4*A);

          //    std::cout << "                              P   = " << P  << std::endl;
          //    std::cout << "                              prep= " << perp << std::endl;
          //    std::cout << "                              R   = " << R  << std::endl;
          //    std::cout << "                              X0  = " << X0_o << std::endl;
          //    std::cout << "                              X1  = " << X1_o << std::endl;
          //    std::cout << "                              X2  = " << X2_o << std::endl;
          const float fac = 1.2F;
          //     std::cout << "ObserverOperator::RecvInTube :\n";
          //     point3D<float> Curv = VecProd(VecProd(X0_o - X0_n, X1_o - X1_n), X2_o - X2_n) / (norm(X0_o - X0_n)*norm(X1_o - X1_n)*norm(X2_o - X2_n));
          //     std::cout << "Curv = " << Curv << std::endl;
          //     std::cout << "n_old = " << n_old << std::endl;
          //     std::cout << "n_new = " << n_new << std::endl;
          //     std::cout << sqrt(perp * perp) << " <? " << fac << "*" << R << " = " << fac*R << std::endl;
          return ((perp_2) < fac*fac*R*R);
          //     if ((perp_2) < fac*fac*R*R)
          //     {
          ////       std::cout << (Recv->pos - mid_old) * n_old << " " << (mid_new - mid_old) * n_old << std::endl;
          ////       std::cout << "a, b, c = " << a << ", " << b << ", " << c << std::endl;
          ////       std::cout << " R : " << R << std::endl;
          ////       std::cout << " sqrt(perp * perp) : " << sqrt(perp * perp) << std::endl;
          //       Ret = true;
          //     }
        }
    }
  return false;
}

/** No descriptions */
template<class TracingOperator_T> 
RecSig ObserverOperator<TracingOperator_T>::SignIntpol(const Triangle* Tri, Receiver* Recv, const int& _t){

  // P position of receiver
  const point3D<float> P = point3D<float>(Recv->pos[0], Recv->pos[1],Recv->pos[2]);

  point3D<float> n_proj((Tri->points[0]->mainray.p_old + Tri->points[1]->mainray.p_old + Tri->points[2]->mainray.p_old)/3.0);
  const float mynorm = n_proj.norm();
  n_proj = n_proj/mynorm;
  // Distance d of old wave face
  const float d = X0_o * n_old;

  // Distance of Receiver to the old face
  float l_r = (P * n_old - d)/(n_old*n_proj);

  // Projection of Receiver into face
  const point3D<float> P_proj = P - n_proj * l_r;

  int axis;
  if ( fabs(n_old[0]) > fabs(n_old[1]) )
    if ( fabs(n_old[0]) > fabs(n_old[2]) )
      axis = 0;
    else
      axis = 2;
  else
    if ( fabs(n_old[1]) > fabs(n_old[2]) )
      axis = 1;
    else
      axis = 2;
    
  int uaxis = (axis + 1)%3;
  int vaxis = (uaxis + 1)%3;
  const float dx10 = (Tri->points[1]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
  const float dy10 = (Tri->points[1]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
  const float dx20 = (Tri->points[2]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
  const float dy20 = (Tri->points[2]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
  const float t2 = ( (Tri->points[0]->mainray.x_old[uaxis] - P_proj[uaxis])*dy10 + (P_proj[vaxis] - Tri->points[0]->mainray.x_old[vaxis])*dx10 ) / ( dy20*dx10 - dx20*dy10);
  const float t1 = ( (Tri->points[0]->mainray.x_old[vaxis] - P_proj[vaxis])*dx20 + (P_proj[uaxis] - Tri->points[0]->mainray.x_old[uaxis])*dy20 ) / ( dy20*dx10 - dx20*dy10);
  const float t0 = 1 - t2 - t1;
  const float t_norm = 1.0f;//sqrt(t0*t0 + t1*t1 + t2*t2);

  if ( (t0 < -0.1) || (t0 > 1.1f) || (t1 < -0.1) || (t1 > 1.1f) || (t2 < -0.1) || (t2 > 1.1f))
    return RecSig();

  // Generate interpolated ray at P_proj from the old rays
  point3D<float> p_interpol = ( Tri->points[0]->mainray.p_old * t0 + Tri->points[1]->mainray.p_old * t1 + Tri->points[2]->mainray.p_old * t2)/t_norm;
  const float v_interpol = (Tri->points[0]->mainray.v_old * t0 + Tri->points[1]->mainray.v_old * t1 + Tri->points[2]->mainray.v_old * t2)/t_norm;
  const float np_interpol = p_interpol.norm();
  p_interpol = p_interpol / (v_interpol*np_interpol);

//**************
  float phi_int, theta_int;

  point3D<float> Pvrtc[6];
  Pvrtc[0]=Tri->points[0]->mainray.x;
  Pvrtc[1]=Tri->points[0]->mainray.x_old;
  Pvrtc[2]=Tri->points[1]->mainray.x;
  Pvrtc[3]=Tri->points[1]->mainray.x_old;
  Pvrtc[4]=Tri->points[2]->mainray.x;
  Pvrtc[5]=Tri->points[2]->mainray.x_old;

  point3D<float> PvrtcVals[6];

  PvrtcVals[0]=PvrtcVals[1]= point3D <float>
  		          (sin(Tri->points[0]->mainray.StartDir.theta)*cos(Tri->points[0]->mainray.StartDir.phi),
  		           sin(Tri->points[0]->mainray.StartDir.theta)*sin(Tri->points[0]->mainray.StartDir.phi),
  		           cos(Tri->points[0]->mainray.StartDir.theta));

 PvrtcVals[2]=PvrtcVals[3]= point3D <float>
  		          (sin(Tri->points[1]->mainray.StartDir.theta)*cos(Tri->points[1]->mainray.StartDir.phi),
  		           sin(Tri->points[1]->mainray.StartDir.theta)*sin(Tri->points[1]->mainray.StartDir.phi),
  		           cos(Tri->points[1]->mainray.StartDir.theta));

 PvrtcVals[4]=PvrtcVals[5]= point3D <float>
  		          (sin(Tri->points[2]->mainray.StartDir.theta)*cos(Tri->points[2]->mainray.StartDir.phi),
  		           sin(Tri->points[2]->mainray.StartDir.theta)*sin(Tri->points[2]->mainray.StartDir.phi),
  		           cos(Tri->points[2]->mainray.StartDir.theta));


 point3D<float> P_int = IDW_Interpol_p3D(P, Pvrtc, PvrtcVals, 6);
 
 P_int = P_int/P_int.norm();

 phi_int =  atan(P_int[1]/P_int[0]);
 theta_int = acos(P_int[2]/(P_int.norm()));

 const Spherical spher_interpol(phi_int,theta_int);
//********
//   const Spherical spher_interpol( (Tri->points[0]->mainray.StartDir.phi + Tri->points[1]->mainray.StartDir.phi + Tri->points[2]->mainray.StartDir.phi)/3.0,(Tri->points[0]->mainray.StartDir.theta + Tri->points[1]->mainray.StartDir.theta + Tri->points[2]->mainray.StartDir.theta)/3.0);


  const point3D<float> P_proj_tmp = point3D<float>(P_proj[0], P_proj[1], P_proj[2]);

  ray3D MidRay(P_proj_tmp, p_interpol, v_interpol, spher_interpol, kmah);
  for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
      {
 	  MidRay.Q[i][j] =  ( Tri->points[0]->mainray.Q_old[i][j] * t0 + Tri->points[1]->mainray.Q_old[i][j] * t1 + Tri->points[2]->mainray.Q_old[i][j] * t2)/t_norm;
 	  MidRay.P[i][j] =  ( Tri->points[0]->mainray.P_old[i][j] * t0 + Tri->points[1]->mainray.P_old[i][j] * t1 + Tri->points[2]->mainray.P_old[i][j] * t2)/t_norm;
      }
  const float dt_tracer = Tracer->TraceToPoint(MidRay, P, ith);
  // Get full travel time
  const float TT = Tracer->GetTT(dt_tracer, _t - 1); 
  // Trace Triangle to intermediate wave front
  ray3D ray0_tmp = Tri->points[0]->mainray;
  ray3D ray1_tmp = Tri->points[1]->mainray;
  ray3D ray2_tmp = Tri->points[2]->mainray;
  ray0_tmp.Restore();
  ray1_tmp.Restore();
  ray2_tmp.Restore();
  Tracer->TraceTstep_RungeKutta_dynamic(dt_tracer, ray0_tmp, ith);  
  Tracer->TraceTstep_RungeKutta_dynamic(dt_tracer, ray1_tmp, ith);  
  Tracer->TraceTstep_RungeKutta_dynamic(dt_tracer, ray2_tmp, ith);  

  // Determine interpolated slowness
  const float detQ = (MidRay.Q[0][0]*MidRay.Q[1][1]*MidRay.Q[2][2] + MidRay.Q[0][1]*MidRay.Q[1][2]*MidRay.Q[2][0] + MidRay.Q[0][2]*MidRay.Q[1][0]*MidRay.Q[2][1] - 
		      MidRay.Q[0][0]*MidRay.Q[2][1]*MidRay.Q[1][2] - MidRay.Q[0][1]*MidRay.Q[1][0]*MidRay.Q[2][2] - MidRay.Q[0][2]*MidRay.Q[1][1]*MidRay.Q[2][0]);

  // Trace parallel rays to intermediate wave front
  kinray3D dxray0_tmp = Tri->points[0]->dxray;
  kinray3D dxray1_tmp = Tri->points[1]->dxray;
  kinray3D dxray2_tmp = Tri->points[2]->dxray;
  dxray0_tmp.Restore();
  dxray1_tmp.Restore();
  dxray2_tmp.Restore();
  Tracer->TraceTstep_RungeKutta(dt_tracer, dxray0_tmp, ith);
  Tracer->TraceTstep_RungeKutta(dt_tracer, dxray1_tmp, ith);
  Tracer->TraceTstep_RungeKutta(dt_tracer, dxray2_tmp, ith);

  kinray3D dyray0_tmp = Tri->points[0]->dyray;
  kinray3D dyray1_tmp = Tri->points[1]->dyray;
  kinray3D dyray2_tmp = Tri->points[2]->dyray;
  dyray0_tmp.Restore();
  dyray1_tmp.Restore();
  dyray2_tmp.Restore();
  Tracer->TraceTstep_RungeKutta(dt_tracer, dyray0_tmp, ith);
  Tracer->TraceTstep_RungeKutta(dt_tracer, dyray1_tmp, ith);
  Tracer->TraceTstep_RungeKutta(dt_tracer, dyray2_tmp, ith);

  // Interpolate parrallel rays to determination point of MidRay
  kinray3D dxray_interp = Interpolate_to_point( MidRay.x, dxray0_tmp, dxray1_tmp, dxray2_tmp);
  kinray3D dyray_interp = Interpolate_to_point( MidRay.x, dyray0_tmp, dyray1_tmp, dyray2_tmp);

  // Determine numerical derivative
  const point3D<float> dPdx = dxray_interp.p - MidRay.p;
  const point3D<float> dPdy = dyray_interp.p - MidRay.p;


  return RecSig(TT, MidRay.v, -MidRay.p[0], -MidRay.p[1], -MidRay.p[2], 
  		dPdx[0], dPdx[1], dPdx[2], 
  		dPdy[0], dPdy[1], dPdy[2], 
  		sqrt(fabs(detQ)), MidRay.StartDir, P_int);

// //   // Generate interpolated ray at P_proj from the old rays
// //   point3D<float> p_interpol = ( Tri->points[0]->mainray.p_old * t0 + Tri->points[1]->mainray.p_old * t1 + Tri->points[2]->mainray.p_old * t2)/t_norm;
// //   const float v_interpol = (Tri->points[0]->mainray.v_old * t0 + Tri->points[1]->mainray.v_old * t1 + Tri->points[2]->mainray.v_old * t2)/t_norm;
// //   const float np_interpol = norm(p_interpol);
// //   p_interpol = p_interpol / (v_interpol*np_interpol);


// //   const Spherical spher_interpol( (Tri->points[0]->mainray.StartDir.phi + Tri->points[1]->mainray.StartDir.phi + Tri->points[2]->mainray.StartDir.phi)/3.0,
// //                                   (Tri->points[0]->mainray.StartDir.theta + Tri->points[1]->mainray.StartDir.theta + Tri->points[2]->mainray.StartDir.theta)/3.0);
// //   //const double Amp = (Tri->points[0]->A + Tri->points[1]->A + Tri->points[2]->A)/3.0;

// //   const point3D<float> P_proj_tmp = point3D<float>(P_proj[0], P_proj[1], P_proj[2]);

// //   ray3D MidRay(P_proj_tmp, p_interpol, v_interpol, spher_interpol, kmah);
 
// //   for (int i = 0; i < 3; i++)
// //       for (int j = 0; j < 3; j++)
// //       {
// // 	  MidRay.Q[i][j] =  ( Tri->points[0]->mainray.Q_old[i][j] * t0 + Tri->points[1]->mainray.Q_old[i][j] * t1 + Tri->points[2]->mainray.Q_old[i][j] * t2)/t_norm;
// // 	  MidRay.P[i][j] =  ( Tri->points[0]->mainray.P_old[i][j] * t0 + Tri->points[1]->mainray.P_old[i][j] * t1 + Tri->points[2]->mainray.P_old[i][j] * t2)/t_norm;
// //       }
// //  //MidRay.A = Amp;

// //   // and trace it the distance l_r
// //   //std::cout << "Trace the distance with my Tracer : " << Tracer << std::endl;

// //   const float dt = Tracer->TraceDistance(MidRay, fabs(l_r), _t-1, ith);
// //   const float detQ = (MidRay.Q[0][0]*MidRay.Q[1][1]*MidRay.Q[2][2] + MidRay.Q[0][1]*MidRay.Q[1][2]*MidRay.Q[2][0] + MidRay.Q[0][2]*MidRay.Q[1][0]*MidRay.Q[2][1] - 
// // 		      MidRay.Q[0][0]*MidRay.Q[2][1]*MidRay.Q[1][2] - MidRay.Q[0][1]*MidRay.Q[1][0]*MidRay.Q[2][2] - MidRay.Q[0][2]*MidRay.Q[1][1]*MidRay.Q[2][0]);


// //   kinray3D dxray_interp = Interpolate_to_point(Recv->pos, Tri->points[0]->dxray, Tri->points[1]->dxray, Tri->points[2]->dxray);
// //   kinray3D dyray_interp = Interpolate_to_point(Recv->pos, Tri->points[0]->dyray, Tri->points[1]->dyray, Tri->points[2]->dyray);

// //   const point3D<float> dPdx = dxray_interp.p;// - MidRay.p;
// //   const point3D<float> dPdy = dyray_interp.p;// - MidRay.p;

// // //   return RecSig(dt, MidRay.v, -MidRay.p[0], -MidRay.p[1], -MidRay.p[2], 
// // // 		dxray_interp.p[0], dxray_interp.p[1], dxray_interp.p[2], 
// // // 		dyray_interp.p[0], dyray_interp.p[1], dyray_interp.p[2], 
// // // 		sqrt(fabs(detQ)));
// //    return RecSig(dt, MidRay.v, -MidRay.p[0], -MidRay.p[1], -MidRay.p[2], 
// //  		dPdx[0], dPdx[1], dPdx[2], 
// //  		dPdy[0], dPdy[1], dPdy[2], 
// //  		sqrt(fabs(detQ)));
}
/** No descriptions */
template<class TracingOperator_T> 
RecSig ObserverOperator<TracingOperator_T>::SignIntpol_old(const Triangle* Tri, Receiver* Recv, const int& _t){

  // P position of receiver
  const point3D<float> P = point3D<float>(Recv->pos[0], Recv->pos[1],Recv->pos[2]);

  // Distance d of old wave face
  const float d = X0_o * n_old;

  // Distance of Receiver to the old face
  float l_r = P * n_old - d;

  // Projection of Receiver into face
  const point3D<float> P_proj = P - n_old * l_r;

  int axis;
  if ( fabs(n_old[0]) > fabs(n_old[1]) )
    if ( fabs(n_old[0]) > fabs(n_old[2]) )
      axis = 0;
    else
      axis = 2;
  else
    if ( fabs(n_old[1]) > fabs(n_old[2]) )
      axis = 1;
    else
      axis = 2;
    
  int uaxis = (axis + 1)%3;
  int vaxis = (uaxis + 1)%3;
  const float dx10 = (Tri->points[1]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
  const float dy10 = (Tri->points[1]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
  const float dx20 = (Tri->points[2]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
  const float dy20 = (Tri->points[2]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
  const float t2 = ( (Tri->points[0]->mainray.x_old[uaxis] - P_proj[uaxis])*dy10 + (P_proj[vaxis] - Tri->points[0]->mainray.x_old[vaxis])*dx10 ) / ( dy20*dx10 - dx20*dy10);
  const float t1 = ( (Tri->points[0]->mainray.x_old[vaxis] - P_proj[vaxis])*dx20 + (P_proj[uaxis] - Tri->points[0]->mainray.x_old[uaxis])*dy20 ) / ( dy20*dx10 - dx20*dy10);
  const float t0 = 1 - t2 - t1;
  const float t_norm = 1.0f;//sqrt(t0*t0 + t1*t1 + t2*t2);

  if ( (t0 < -0.1) || (t0 > 1.1f) || (t1 < -0.1) || (t1 > 1.1f) || (t2 < -0.1) || (t2 > 1.1f))
       return RecSig();

  ray3D ray0_tmp = Tri->points[0]->mainray;
  ray3D ray1_tmp = Tri->points[1]->mainray;
  ray3D ray2_tmp = Tri->points[2]->mainray;
  ray0_tmp.Restore();
  ray1_tmp.Restore();
  ray2_tmp.Restore();
  const float dt0 = Tracer->TraceDistance(ray0_tmp, fabs(l_r), _t-1, ith);  
  const float dt1 = Tracer->TraceDistance(ray1_tmp, fabs(l_r), _t-1, ith);  
  const float dt2 = Tracer->TraceDistance(ray2_tmp, fabs(l_r), _t-1, ith);  
  if ( (dt0 < 0) || ( dt1 < 0) || (dt2 < 0))
      return RecSig();
  const float t_interpol = dt0*t0 + dt1*t1 + dt2*t2;
  const float v_interpol = (ray0_tmp.v * t0 + ray1_tmp.v * t1 + ray2_tmp.v * t2)/t_norm;
  point3D<float> x_interpol = ( ray0_tmp.x * t0 + ray1_tmp.x * t1 + ray2_tmp.x * t2)/t_norm;
  point3D<float> p_interpol = ( ray0_tmp.p * t0 + ray1_tmp.p * t1 + ray2_tmp.p * t2)/t_norm;
  const float np_interpol = p_interpol.norm();
  p_interpol = p_interpol / (v_interpol*np_interpol);
  float Q_interpol[3][3];
  for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
      {
 	  Q_interpol[i][j] =  ( ray0_tmp.Q[i][j] * t0 + ray1_tmp.Q[i][j] * t1 + ray2_tmp.Q[i][j] * t2)/t_norm;
      }

  const float detQ_interpol = (Q_interpol[0][0]*Q_interpol[1][1]*Q_interpol[2][2] + Q_interpol[0][1]*Q_interpol[1][2]*Q_interpol[2][0] + Q_interpol[0][2]*Q_interpol[1][0]*Q_interpol[2][1] - 
 		      Q_interpol[0][0]*Q_interpol[2][1]*Q_interpol[1][2] - Q_interpol[0][1]*Q_interpol[1][0]*Q_interpol[2][2] - Q_interpol[0][2]*Q_interpol[1][1]*Q_interpol[2][0]);
  const Spherical StartDir_interpol( ( ray0_tmp.StartDir.phi * t0 + ray1_tmp.StartDir.phi * t1 + ray2_tmp.StartDir.phi * t2)/t_norm, ( ray0_tmp.StartDir.theta * t0 + ray1_tmp.StartDir.theta * t1 + ray2_tmp.StartDir.theta * t2)/t_norm);

  kinray3D dxray0_tmp = Tri->points[0]->dxray;
  kinray3D dxray1_tmp = Tri->points[1]->dxray;
  kinray3D dxray2_tmp = Tri->points[2]->dxray;
  dxray0_tmp.Restore();
  dxray1_tmp.Restore();
  dxray2_tmp.Restore();
  Tracer->TraceDistance(dxray0_tmp, fabs(l_r), ith);
  Tracer->TraceDistance(dxray1_tmp, fabs(l_r), ith);
  Tracer->TraceDistance(dxray2_tmp, fabs(l_r), ith);

  kinray3D dyray0_tmp = Tri->points[0]->dyray;
  kinray3D dyray1_tmp = Tri->points[1]->dyray;
  kinray3D dyray2_tmp = Tri->points[2]->dyray;
  dyray0_tmp.Restore();
  dyray1_tmp.Restore();
  dyray2_tmp.Restore();
  Tracer->TraceDistance(dyray0_tmp, fabs(l_r), ith);
  Tracer->TraceDistance(dyray1_tmp, fabs(l_r), ith);
  Tracer->TraceDistance(dyray2_tmp, fabs(l_r), ith);
  kinray3D dxray_interp = Interpolate_to_point( x_interpol, dxray0_tmp, dxray1_tmp, dxray2_tmp);
  kinray3D dyray_interp = Interpolate_to_point( x_interpol, dyray0_tmp, dyray1_tmp, dyray2_tmp);
  const point3D<float> dPdx = dxray_interp.p - p_interpol;
  const point3D<float> dPdy = dyray_interp.p - p_interpol;

  return RecSig(t_interpol, v_interpol, -p_interpol[0],  -p_interpol[1], -p_interpol[2], 
  		dPdx[0], dPdx[1], dPdx[2], 
  		dPdy[0], dPdy[1], dPdy[2], 
  		sqrt(fabs(detQ_interpol)),
		StartDir_interpol);


// //   // Generate interpolated ray at P_proj from the old rays
// //   point3D<float> p_interpol = ( Tri->points[0]->mainray.p_old * t0 + Tri->points[1]->mainray.p_old * t1 + Tri->points[2]->mainray.p_old * t2)/t_norm;
// //   const float v_interpol = (Tri->points[0]->mainray.v_old * t0 + Tri->points[1]->mainray.v_old * t1 + Tri->points[2]->mainray.v_old * t2)/t_norm;
// //   const float np_interpol = norm(p_interpol);
// //   p_interpol = p_interpol / (v_interpol*np_interpol);


// //   const Spherical spher_interpol( (Tri->points[0]->mainray.StartDir.phi + Tri->points[1]->mainray.StartDir.phi + Tri->points[2]->mainray.StartDir.phi)/3.0,
// //                                   (Tri->points[0]->mainray.StartDir.theta + Tri->points[1]->mainray.StartDir.theta + Tri->points[2]->mainray.StartDir.theta)/3.0);
// //   //const double Amp = (Tri->points[0]->A + Tri->points[1]->A + Tri->points[2]->A)/3.0;

// //   const point3D<float> P_proj_tmp = point3D<float>(P_proj[0], P_proj[1], P_proj[2]);

// //   ray3D MidRay(P_proj_tmp, p_interpol, v_interpol, spher_interpol, kmah);
 
// //   for (int i = 0; i < 3; i++)
// //       for (int j = 0; j < 3; j++)
// //       {
// // 	  MidRay.Q[i][j] =  ( Tri->points[0]->mainray.Q_old[i][j] * t0 + Tri->points[1]->mainray.Q_old[i][j] * t1 + Tri->points[2]->mainray.Q_old[i][j] * t2)/t_norm;
// // 	  MidRay.P[i][j] =  ( Tri->points[0]->mainray.P_old[i][j] * t0 + Tri->points[1]->mainray.P_old[i][j] * t1 + Tri->points[2]->mainray.P_old[i][j] * t2)/t_norm;
// //       }
// //  //MidRay.A = Amp;

// //   // and trace it the distance l_r
// //   //std::cout << "Trace the distance with my Tracer : " << Tracer << std::endl;

// //   const float dt = Tracer->TraceDistance(MidRay, fabs(l_r), _t-1, ith);
// //   const float detQ = (MidRay.Q[0][0]*MidRay.Q[1][1]*MidRay.Q[2][2] + MidRay.Q[0][1]*MidRay.Q[1][2]*MidRay.Q[2][0] + MidRay.Q[0][2]*MidRay.Q[1][0]*MidRay.Q[2][1] - 
// // 		      MidRay.Q[0][0]*MidRay.Q[2][1]*MidRay.Q[1][2] - MidRay.Q[0][1]*MidRay.Q[1][0]*MidRay.Q[2][2] - MidRay.Q[0][2]*MidRay.Q[1][1]*MidRay.Q[2][0]);


// //   kinray3D dxray_interp = Interpolate_to_point(Recv->pos, Tri->points[0]->dxray, Tri->points[1]->dxray, Tri->points[2]->dxray);
// //   kinray3D dyray_interp = Interpolate_to_point(Recv->pos, Tri->points[0]->dyray, Tri->points[1]->dyray, Tri->points[2]->dyray);

// //   const point3D<float> dPdx = dxray_interp.p;// - MidRay.p;
// //   const point3D<float> dPdy = dyray_interp.p;// - MidRay.p;

// // //   return RecSig(dt, MidRay.v, -MidRay.p[0], -MidRay.p[1], -MidRay.p[2], 
// // // 		dxray_interp.p[0], dxray_interp.p[1], dxray_interp.p[2], 
// // // 		dyray_interp.p[0], dyray_interp.p[1], dyray_interp.p[2], 
// // // 		sqrt(fabs(detQ)));
// //    return RecSig(dt, MidRay.v, -MidRay.p[0], -MidRay.p[1], -MidRay.p[2], 
// //  		dPdx[0], dPdx[1], dPdx[2], 
// //  		dPdy[0], dPdy[1], dPdy[2], 
// //  		sqrt(fabs(detQ)));
}
/** No descriptions */
template<class TracingOperator_T> 
void ObserverOperator<TracingOperator_T>::SignIntpol(const Triangle* Tri, Receiver* Recv[4], const int& _t, RecSig Signals[4]){

  // Distance d of old wave face
  const float d = X0_o * n_old;

  float l_r[4];
  bool valid[4];
  ray3D* MidRay[4];

  point3D<float> P, P_proj, P_proj_tmp, P_n, Q, p_interpol;
  for (int i = 0; i < 4; i++)
    {
      // P position of receiver
      P = point3D<float>(Recv[i]->pos[0], Recv[i]->pos[1], Recv[i]->pos[2]);


      // Distance of Receiver to the old face
      l_r[i] = (P * n_old) - d;

      // Projection of Receiver into face
      P_proj = P - n_old * l_r[i];
      l_r[i] = fabs(l_r[i]);
      // Intersection of the line X0-P_proj with X1-X2 is calculated
      // as the intersection of the plane build by the points X0,P and the normal n_old
      // with X1-X2

//       // Normal of the triangle P, X0 and old face normal
//       P_n = VecProd( P - X0_o, n_old);
//       const float P_n_l = sqrt(P_n[0]*P_n[0] + P_n[1]*P_n[1] + P_n[2]*P_n[2]);
//       P_n = P_n / P_n_l;

//       // Intersection Q of plane normal to x_n and line points[1] -- points[2]
//       Q = (X1_o * ( (X2_o-P)*P_n ) - X2_o * ( (X1_o-P)*P_n )) / ( (X2_o-X1_o)*P_n);


//       // Barycentric coordinates from Distance points[1] -- Q:
//       const float t2 = ((P - X1_o) * P_n) / ((X2_o - X1_o) * P_n);
//       const float t1 = 1-t2;
//       const float t0 = norm(P_proj - Q) / norm(P_proj - X0_o);
//       const float t_norm = t0+1;


      int axis;
      if ( fabs(n_old[0]) > fabs(n_old[1]) )
        if ( fabs(n_old[0]) > fabs(n_old[2]) )
          axis = 0;
        else
          axis = 2;
      else
        if ( fabs(n_old[1]) > fabs(n_old[2]) )
          axis = 1;
        else
          axis = 2;
    
      int uaxis = (axis + 1)%3;
      int vaxis = (uaxis + 1)%3;
      const float dx10 = (Tri->points[1]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
      const float dy10 = (Tri->points[1]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
      const float dx20 = (Tri->points[2]->mainray.x_old[uaxis] - Tri->points[0]->mainray.x_old[uaxis]);
      const float dy20 = (Tri->points[2]->mainray.x_old[vaxis] - Tri->points[0]->mainray.x_old[vaxis]);
      const float t2 = ( (Tri->points[0]->mainray.x_old[uaxis] - P_proj[uaxis])*dy10 + (P_proj[vaxis] - Tri->points[0]->mainray.x_old[vaxis])*dx10 ) / ( dy20*dx10 - dx20*dy10);
      const float t1 = ( (Tri->points[0]->mainray.x_old[vaxis] - P_proj[vaxis])*dx20 + (P_proj[uaxis] - Tri->points[0]->mainray.x_old[uaxis])*dy20 ) / ( dy20*dx10 - dx20*dy10);
      const float t0 = 1 - t2 - t1;
      const float t_norm = 1.0f;//sqrt(t0*t0 + t1*t1 + t2*t2);



      // Generate interpolated ray at P_proj from the old rays
      p_interpol = ( Tri->points[0]->mainray.p_old * t0 + Tri->points[1]->mainray.p_old * t1 + Tri->points[2]->mainray.p_old * t2)/t_norm;
      const float v_interpol = (Tri->points[0]->mainray.v_old * t0 + Tri->points[1]->mainray.v_old * t1 + Tri->points[2]->mainray.v_old * t2)/t_norm;
      const float np_interpol = p_interpol.norm();
      p_interpol = p_interpol / (v_interpol*np_interpol);
      

      const Spherical spher_interpol( (Tri->points[0]->mainray.StartDir.phi + Tri->points[1]->mainray.StartDir.phi + Tri->points[2]->mainray.StartDir.phi)/3.0,
                                      (Tri->points[0]->mainray.StartDir.theta + Tri->points[1]->mainray.StartDir.theta + Tri->points[2]->mainray.StartDir.theta)/3.0);
      //const double Amp = (Tri->points[0]->A + Tri->points[1]->A + Tri->points[2]->A)/3.0;

      P_proj_tmp = point3D<float>(P_proj[0], P_proj[1], P_proj[2]);

//       if ( (t0 < 0) || (t0 > 1.0f) || (t1 < 0) || (t1 > 1.0f) || (t2 < 0) || (t2 > 1.0f))
//       {
// 	  MidRay[i] = new ray3D(Tri->points[0]->x_old, Tri->points[0]->p_old, Tri->points[0]->v, Tri->points[0]->StartDir,  Tri->points[0]->kmah);
// 	  valid[i] = false;
//       }
//       else
      {
	  MidRay[i] = new ray3D(P_proj_tmp, p_interpol, v_interpol, spher_interpol, kmah);
	  for (int iq = 0; iq < 3; iq++)
	      for (int jq = 0; jq < 3; jq++)
	      {
		  MidRay[i]->Q[iq][jq] =  ( Tri->points[0]->mainray.Q_old[iq][jq] * t0 + Tri->points[1]->mainray.Q_old[iq][jq] * t1 + Tri->points[2]->mainray.Q_old[iq][jq] * t2)/t_norm;
		  MidRay[i]->P[iq][jq] =  ( Tri->points[0]->mainray.P_old[iq][jq] * t0 + Tri->points[1]->mainray.P_old[iq][jq] * t1 + Tri->points[2]->mainray.P_old[iq][jq] * t2)/t_norm;
	      }

	  valid[i] = true;
      }
    }


  // and trace it the distance l_r
  //std::cout << "Trace the distance with my Tracer : " << Tracer << std::endl;
  float dt[4];
  Tracer->TraceDistance(MidRay, l_r, _t-1, dt, ith);

  for (int i = 0; i < 4; i++)
    {
      if (valid[i])
      {
	  const float detQ = (MidRay[i]->Q[0][0]*MidRay[i]->Q[1][1]*MidRay[i]->Q[2][2] 
			      + MidRay[i]->Q[0][1]*MidRay[i]->Q[1][2]*MidRay[i]->Q[2][0] 
			      + MidRay[i]->Q[0][2]*MidRay[i]->Q[1][0]*MidRay[i]->Q[2][1] 
			      - MidRay[i]->Q[0][0]*MidRay[i]->Q[2][1]*MidRay[i]->Q[1][2] 
			      - MidRay[i]->Q[0][1]*MidRay[i]->Q[1][0]*MidRay[i]->Q[2][2] 
			      - MidRay[i]->Q[0][2]*MidRay[i]->Q[1][1]*MidRay[i]->Q[2][0]);

	  Signals[i] = RecSig(dt[i], MidRay[i]->v, -MidRay[i]->p[0], -MidRay[i]->p[1], -MidRay[i]->p[3], sqrt(fabs(detQ)), MidRay[i]->StartDir);
      }
      else
	  Signals[i] = RecSig();	  
      delete MidRay[i];
    }
}
/** No descriptions */
template<class TracingOperator_T> 
bool ObserverOperator<TracingOperator_T>::RecvInBox(const Triangle* Tri, const Receiver* Recv){
  const point3D<float>& X0_o = Tri->points[0]->mainray.x_old;
  const point3D<float>& X1_o = Tri->points[1]->mainray.x_old;
  const point3D<float>& X2_o = Tri->points[2]->mainray.x_old;

  const point3D<float>& X0_n = Tri->points[0]->mainray.x;
  const point3D<float>& X1_n = Tri->points[1]->mainray.x;
  const point3D<float>& X2_n = Tri->points[2]->mainray.x;

  point3D<float> Xmax = X0_o;
  point3D<float> Xmin = X0_o;

  //  for (int i = 0; i < 3; i++)
  //  {
  //    Xmax[i] = std::max(Xmax[i], X1_o[i]);
  //    Xmin[i] = std::min(Xmin[i], X1_o[i]);
  //  }
  //
  //  for (int i = 0; i < 3; i++)
  //  {
  //    Xmax[i] = std::max(Xmax[i], X2_o[i]);
  //    Xmin[i] = std::min(Xmin[i], X2_o[i]);
  //  }
  //
  //  for (int i = 0; i < 3; i++)
  //  {
  //    Xmax[i] = std::max(Xmax[i], X0_n[i]);
  //    Xmin[i] = std::min(Xmin[i], X0_n[i]);
  //  }
  //
  //  for (int i = 0; i < 3; i++)
  //  {
  //    Xmax[i] = std::max(Xmax[i], X1_n[i]);
  //    Xmin[i] = std::min(Xmin[i], X1_n[i]);
  //  }
  //
  //  for (int i = 0; i < 3; i++)
  //  {
  //    Xmax[i] = std::max(Xmax[i], X2_n[i]);
  //    Xmin[i] = std::min(Xmin[i], X2_n[i]);
  //  }

  //  for (int i = 0; i < 3; i++)
  //  {
  //    if ( X1_o[i] > Xmax[i])
  //      Xmax[i] = X1_o[i];
  //    else
  //      if ( X1_o[i] < Xmin[i])
  //        Xmin[i] = X1_o[i];
  //
  //    if ( X2_o[i] > Xmax[i])
  //      Xmax[i] = X2_o[i];
  //    else
  //      if ( X2_o[i] < Xmin[i])
  //        Xmin[i] = X2_o[i];
  //
  //    if ( X0_n[i] > Xmax[i])
  //      Xmax[i] = X0_n[i];
  //    else
  //      if ( X0_n[i] < Xmin[i])
  //        Xmin[i] = X0_n[i];
  //
  //    if ( X1_n[i] > Xmax[i])
  //      Xmax[i] = X1_n[i];
  //    else
  //      if ( X1_n[i] < Xmin[i])
  //        Xmin[i] = X1_n[i];
  //
  //    if ( X2_n[i] > Xmax[i])
  //      Xmax[i] = X2_n[i];
  //    else
  //      if ( X2_n[i] < Xmin[i])
  //        Xmin[i] = X2_n[i];
  //  }

  //  bool gtmax = true;
  //  bool gtmin = true;
  //
  //  const point3D<>& RcvPos = Recv->pos;
  //
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X0_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X0_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X1_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X1_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X2_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X2_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X0_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X0_n[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X1_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X1_n[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X2_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X2_n[i]);
  //  }
  //
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X0_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X0_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X1_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X1_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X2_o[i]);
  //    gtmin = gtmin && (RcvPos[i] < X2_o[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X0_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X0_n[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X1_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X1_n[i]);
  //  }
  //  for (int i = 0; i < 2; i++)
  //  {
  //    gtmax = gtmax && (RcvPos[i] > X2_n[i]);
  //    gtmin = gtmin && (RcvPos[i] < X2_n[i]);
  //  }


  const point3D<float>& RcvPos = Recv->pos;

  for (int i = 0; i < 2; i++)
    {
      bool gtmax = true;
      bool gtmin = true;

      gtmax = gtmax && (RcvPos[i] > X0_o[i]);
      gtmax = gtmax && (RcvPos[i] > X1_o[i]);
      gtmax = gtmax && (RcvPos[i] > X2_o[i]);
      gtmax = gtmax && (RcvPos[i] > X0_n[i]);
      gtmax = gtmax && (RcvPos[i] > X1_n[i]);
      gtmax = gtmax && (RcvPos[i] > X2_n[i]);

      if (gtmax)
        return false;

      gtmin = gtmin && (RcvPos[i] < X0_o[i]);
      gtmin = gtmin && (RcvPos[i] < X1_o[i]);
      gtmin = gtmin && (RcvPos[i] < X2_o[i]);
      gtmin = gtmin && (RcvPos[i] < X0_n[i]);
      gtmin = gtmin && (RcvPos[i] < X1_n[i]);
      gtmin = gtmin && (RcvPos[i] < X2_n[i]);

      if (gtmin)
        return false;
    }

  return true;
}

template<class TracingOperator_T> 
kinray3D ObserverOperator<TracingOperator_T>::Interpolate_to_point(const point3D<float>& Rec_pos, const kinray3D& ray0, const kinray3D& ray1, const kinray3D& ray2)
{
    // normal of old wave front
//  point3D<float> normal = VecProd(ray1.x_old - ray0.x_old, ray2.x_old - ray0.x_old);
    point3D<float> normal = (ray1.x_old - ray0.x_old).VecProd(ray2.x_old - ray0.x_old);

    if (normal.norm() > 1.0e-6)
	normal = normal / ( normal.norm() );

    // Distance d of old wave face
    const float d =  ray0.x_old * normal;

    // Distance of Receiver to the old face
    float l_r = Rec_pos * normal - d;

    // Projection of Receiver into face
    const point3D<float> P_proj = Rec_pos - normal * l_r;

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
  const float dx10 = (ray1.x_old[uaxis] - ray0.x_old[uaxis]);
  const float dy10 = (ray1.x_old[vaxis] - ray0.x_old[vaxis]);
  const float dx20 = (ray2.x_old[uaxis] - ray0.x_old[uaxis]);
  const float dy20 = (ray2.x_old[vaxis] - ray0.x_old[vaxis]);
  const float t2 = ( (ray0.x_old[uaxis] - P_proj[uaxis])*dy10 + (P_proj[vaxis] - ray0.x_old[vaxis])*dx10 ) / ( dy20*dx10 - dx20*dy10);
  const float t1 = ( (ray0.x_old[vaxis] - P_proj[vaxis])*dx20 + (P_proj[uaxis] - ray0.x_old[uaxis])*dy20 ) / ( dy20*dx10 - dx20*dy10);
  const float t0 = 1 - t2 - t1;
  const float t_norm = 1.0f;//sqrt(t0*t0 + t1*t1 + t2*t2);

  // Generate interpolated ray at P_proj from the old rays
  point3D<float> p_interpol = ( ray0.p_old * t0 + ray1.p_old * t1 + ray2.p_old * t2)/t_norm;
  const float v_interpol = (ray0.v_old * t0 + ray1.v_old * t1 + ray2.v_old * t2)/t_norm;
  const float np_interpol = p_interpol.norm();
  p_interpol = p_interpol / (v_interpol*np_interpol);


  const Spherical spher_interpol( (ray0.StartDir.phi + ray1.StartDir.phi + ray2.StartDir.phi)/3.0,
                                  (ray0.StartDir.theta + ray1.StartDir.theta + ray2.StartDir.theta)/3.0);

  kinray3D Ray_interp(P_proj, p_interpol, v_interpol, spher_interpol);
  if (l_r < 0)
  {
      Ray_interp.p[0] = -Ray_interp.p[0];
      Ray_interp.p[1] = -Ray_interp.p[1];
      Ray_interp.p[2] = -Ray_interp.p[2];
      Tracer->TraceDistance(Ray_interp, fabs(l_r), ith);
      Ray_interp.p[0] = -Ray_interp.p[0];
      Ray_interp.p[1] = -Ray_interp.p[1];
      Ray_interp.p[2] = -Ray_interp.p[2];
  }
  else
      Tracer->TraceDistance(Ray_interp, fabs(l_r), ith);

  return Ray_interp;
}

template<class TracingOperator_T> 
point3D<float> ObserverOperator<TracingOperator_T>::IDW_Interpol_p3D(point3D<float> pX, point3D<float> *P, point3D<float> *Pval, int nP)
{
  point3D<float> nom(0,0,0);
  float denom=0., dtmp;
  for(int i=0;i<nP;i++){
	dtmp = (pX-P[i]).norm();
	if(dtmp>1e-8){
	dtmp = 1./dtmp;
	nom = nom + Pval[i]*dtmp;
	denom += dtmp;
	}else return Pval[i];
  }
   return nom/denom;
}

template<class TracingOperator_T> 
float ObserverOperator<TracingOperator_T>::IDW_Interpol(point3D<float> pX, point3D<float> *P, float *Pval, int nP)
{
  float nom=0.,denom=0., dtmp;
  for(int i=0;i<nP;i++){
	dtmp = (pX-P[i]).norm();
	if(dtmp>1e-8){
	dtmp = 1./dtmp;
	nom += dtmp * Pval[i];
	denom += dtmp;
	}else return Pval[i];
  }
   return nom/denom;
}
