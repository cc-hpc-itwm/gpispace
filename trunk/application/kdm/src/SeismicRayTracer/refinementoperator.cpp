/***************************************************************************
                          refinementoperator.cpp  -  description
                             -------------------
    begin                : Wed Nov 16 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "refinementoperator.h"

template<class TracingOperator_T> 
RefinementOperator<TracingOperator_T>::RefinementOperator(){
  max_line = (int) 1e6;   // Dummy value for infinite line length, i.e. no interpolation
  min_line = 10;
  ith = 0;
}
template<class TracingOperator_T> 
RefinementOperator<TracingOperator_T>::RefinementOperator(const int& lmax, TracingOperator_T* TrOp, WFPtSrc* _Source, const int _ith){
  max_line = lmax;
  min_line = 10;
  Tracer = TrOp;
  Source = _Source;
  ith = _ith;
}
template<class TracingOperator_T> 
RefinementOperator<TracingOperator_T>::~RefinementOperator(){
}
/** No descriptions */
template<class TracingOperator_T> 
void RefinementOperator<TracingOperator_T>::operator () (triMem& _TList, trirayMem& _RList, const float& t){
  triMem* TList = &_TList;
  trirayMem* RList = &_RList;
  
  int notri = TList->size();

  for(int i = 0; i < notri; i++)
    if ( (*TList)[i].lifesign != Triangle::DELETED)
    {
      Subdivide(TList, RList, &((*TList)[i]), t);
    }  
}
//** No descriptions */
template<class TracingOperator_T> 
void RefinementOperator<TracingOperator_T>::operator () (triMem& _TList, trirayMem& _RList, Triangle* T, const float& t){
  triMem* TList = &_TList;
  trirayMem* RList = &_RList;

    if ( T->lifesign != Triangle::DELETED)
    {
      Subdivide(TList, RList, T, t);
    }
}
// void RefinementOperator::DeleteRay(Triangle* T, ray3D* r_old, ray3D* r_new, Triangle* n1_old, Triangle* n1_new, Triangle* n2_old, Triangle* n2_new)
// {
//   if (T != NULL)
//     {
//       if ( T->lifesign != Triangle::DELETED)
//         {
//           std::cout << T->id << "  " << T->lifesign << std::endl;
//           bool del = false;
//           for (int i = 0; i < 3; i++)
//             {
//               if (T->points[i] == r_old)
//                 {
//                   T->points[i] = r_new;
//                   for (int n = 0; n < 3; n++)
//                     {
//                       DeleteRay(T->neighs[n], r_old, r_new, n1_old, n1_new, n2_old, n2_new);
//                     }
// //                   const int i1 = (i+1)%3;
// //                   const int i2 = (i1+1)%3;

// //                   if ( T->neighs[i1] == n1_old)
// //                     T->neighs[i1] = n1_new;
// //                   if ( T->neighs[i1] == n2_old)
// //                     T->neighs[i1] = n2_new;
// //                   if ( T->neighs[i2] == n1_old)
// //                     T->neighs[i2] = n1_new;
// //                   if ( T->neighs[i2] == n2_old)
// //                     T->neighs[i2] = n2_new;
//                   del = true;
//                 }
//             }

//           if (T->id == 276)
//             std::cout << T->points[0] << " " << T->points[1] << " " << T->points[2] << std::endl;

// //           if (del)
// //             {
// //               for (int n = 0; n < 3; n++)
// //                 {
// //                   DeleteRay(T->neighs[n], r_old, r_new, n1_old, n1_new, n2_old, n2_new);
// //                 }
// //             }
//         }
//     }
// }

//** No descriptions */
template<class TracingOperator_T> 
int RefinementOperator<TracingOperator_T>::Subdivide(triMem* TList, trirayMem* RList, Triangle* T, const float& t){
  int ld[3];
  int ldn = CheckDistances(*T, ld);

//std::cout << "Triangle " << T->id << " has neighbors ";
// for (int in = 0; in < 3; in++)
//     {
//       if ( T->neighs[in] != NULL)
//         std::cout << T->neighs[in]->id;
//       else 
//         std::cout << "NULL";
//       std::cout << "  ";
//     }
//   std::cout << "\n";

//   if ( ldn == -1)
//     {
//       const int i0 = ld[0];
//       const int i1 = (i0+1)%3;
//       const int i2 = (i1+1)%3;

//       ray3D* r_old = T->points[i1];
//       ray3D* r_new = T->points[i2];
//       int n0;
//       for (int n = 0; n < 3; n++)
//         {
//           if ( T->neighs[n] != NULL)
//             if ( (T->neighs[n]->points[0] == r_old && T->neighs[n]->points[1] == r_new) 
//                  || (T->neighs[n]->points[1] == r_old && T->neighs[n]->points[0] == r_new) 
//                  || (T->neighs[n]->points[0] == r_old && T->neighs[n]->points[2] == r_new) 
//                  || (T->neighs[n]->points[2] == r_old && T->neighs[n]->points[0] == r_new) 
//                  || (T->neighs[n]->points[1] == r_old && T->neighs[n]->points[2] == r_new) 
//                  || (T->neighs[n]->points[2] == r_old && T->neighs[n]->points[1] == r_new) )
//               {
//                 n0 = n;
//               }
//         }
//       std::cout << "n0 = " << n0 << " =? " << i0 << std::endl;
//       Triangle* n1_old = T;
//       Triangle* n1_new = T->neighs[i1];
//       Triangle* n2_old = T->neighs[n0];
//       Triangle* n2_new;
//       int i2_new = -1;
//       if (T->neighs[n0] != NULL)
//         {
//           for (int ir = 0; ir < 3; ir++)
//             if (T->neighs[n0]->points[ir] == r_old)
//               i2_new = ir;
//         }
//       if (i2_new != -1)
//         n2_new = T->neighs[n0]->neighs[i2_new];
//       else 
//         n2_new = NULL;
//       std::cout << "i0 = " << i0 << ", i1 = " << i1 << ", i2 = " << i2 << std::endl;
//       std::cout << "r_old = " << r_old << ", r_new = " << r_new << std::endl;
//       std::cout << "i2_new = " << i2_new << std::endl;
//       if (n1_new != NULL)
//         std::cout << "n1_new = " << n1_new->id << std::endl;
//       else
//         std::cout << "n1_new = NULL\n";
//       if (n2_new != NULL)
//         std::cout << "n2_new = " << n2_new->id << std::endl;
//       else
//         std::cout << "n2_new = NULL\n";

//       //std::cout << n1_old->id << " to " << n1_new->id << std::endl;
//       //std::cout << n2_old->id << " to " << n2_new->id << std::endl;
//       DeleteRay(T, r_old, r_new, n1_old, n1_new, n2_old, n2_new);

//       for (int iin = 0; iin < 3; iin++)
//         if ( T->neighs[iin] != NULL )
//           {
//             std::cout << "Triangle " << T->neighs[iin]->id << " has neighbors ";
//             for (int in = 0; in < 3; in++)
//               {
//                 if ( T->neighs[iin]->neighs[in] != NULL)
//                   std::cout << T->neighs[iin]->neighs[in]->id;
//                 else 
//                   std::cout << "NULL";
//                 std::cout << "  ";
//               }
//             std::cout << "\n";
//           }

//       if ( T->neighs[i1] != NULL )
//         for (int in = 0; in < 3; in++)
//           if (T->neighs[i1]->neighs[in] == T)
//             {
//               std::cout << "changing to " << T->neighs[i2]->id << "\n";
//               T->neighs[i1]->neighs[in] = T->neighs[i2];
//               std::cout << T->neighs[i1]->neighs[in]->id << std::endl;
//             }
//       if ( T->neighs[i2] != NULL )
//         for (int in = 0; in < 3; in++)
//           if (T->neighs[i2]->neighs[in] == T)
//             {
//               std::cout << "changing to " << T->neighs[i1]->id << "\n";
//               T->neighs[i2]->neighs[in] = T->neighs[i1];
//               std::cout << T->neighs[i2]->neighs[in]->id << std::endl;
//             }

//       for (int iin = 0; iin < 3; iin++)
//         if ( T->neighs[iin] != NULL )
//           {
//             std::cout << "Triangle " << T->neighs[iin]->id << " has neighbors ";
//             for (int in = 0; in < 3; in++)
//               {
//                 if ( T->neighs[iin]->neighs[in] != NULL)
//                   std::cout << T->neighs[iin]->neighs[in]->id;
//                 else 
//                   std::cout << "NULL";
//                 std::cout << "  ";
//               }
//             std::cout << "\n";
//           }



//       if ( T->neighs[i0] != NULL)
//         {
//           int ii0, ii1, ii2;
//           for (int ir = 0; ir < 3; ir++)
//             if ( T->neighs[i0]->neighs[ir] == T)
//               {
//                 ii0 = ir;
//                 ii1 = (ii0+1)%3;
//                 ii2 = (ii1+1)%3;
//               }
//           if ( T->neighs[i0]->neighs[ii1] != NULL )
//             for (int in = 0; in < 3; in++)
//               if (T->neighs[i0]->neighs[ii1]->neighs[in] == T->neighs[i0])
//                 T->neighs[i0]->neighs[ii1]->neighs[in] = T->neighs[i0]->neighs[ii2];
//           if ( T->neighs[i0]->neighs[ii2] != NULL )
//             for (int in = 0; in < 3; in++)
//               if (T->neighs[i0]->neighs[ii2]->neighs[in] == T->neighs[i0])
//                 T->neighs[i0]->neighs[ii2]->neighs[in] = T->neighs[i0]->neighs[ii1];
//         }

//       DeleteTri(n2_old);
//       if (n2_old != NULL)
//         std::cout << "Triangle " << n2_old->id << " has been deleted\n";
//       DeleteTri(T);
//       std::cout << "Triangle " << T->id << " has been deleted\n";
//     }

//  else if (ldn > 0)
  if (ldn > 0)
  {
//    if (ldn != 3)
//    {
      const int i0 = ld[0];
      const int i1 = (i0+1)%3;
      const int i2 = (i1+1)%3;

      // generate new ray ...
      triray3D newray = InterpolateRay(*(T->points[i1]), *(T->points[i2]), t);
      newray.mainray.status = T->points[i1]->mainray.status;
      // ... and store it
      //int newray_index = RList->size();
      
      //cout << "newray_index = " << newray_index << endl;
      //(*RList).push_back(newray);
      //ray3D* newray_pointer = &(*RList)[newray_index];
      triray3D* newray_pointer = (*RList).push_back(newray);
      
      // generate new sub-triangle ...
      int newT1_index = TList->size();
      Triangle newT1(newT1_index, T->points[i1], newray_pointer, T->points[i0],
                     T, T->neighs[i2], T->neighs[i0]);

      newT1.status = T->status;
      // ... and store it                     
      //cout << "newT1_index = " << newT1_index << endl;
      //(*TList).push_back(newT1);
      //Triangle* newT1_pointer = &(*TList)[newT1_index];
      Triangle* newT1_pointer = (*TList).push_back(newT1);

      // Tell neighbor triangle to have new triangle as neighbor
      if ((T->neighs[i2] != NULL) && (T->neighs[i2]->lifesign != Triangle::DELETED))
      {
        for (int i = 0; i < 3; i++)
        {
          if ( T->neighs[i2]->neighs[i] == T )
          {
            T->neighs[i2]->neighs[i] = newT1_pointer; //&((*TList)[newT1_index]);
            break;
          }
        }
      }

      // Change source triangle to be the other part
      T->neighs[i2] = newT1_pointer; //&((*TList)[newT1_index]);
      T->points[i1] = newray_pointer; //&((*RList)[newray_index]);

      // generate new sub-triangle of neighbor
      if ( (T->neighs[i0] != NULL) && (T->neighs[i0]->lifesign != Triangle::DELETED))
      {
        Triangle* T_neigh = T->neighs[i0];
        int i1_neigh = -1, i2_neigh = -1, i0_neigh = -1;
        for (int i = 0; i < 3; i++)
        {
          if (T_neigh->points[i] == newT1.points[0])
            i1_neigh = i;
          if (T_neigh->points[i] == T->points[i2])
            i2_neigh = i;
        }
        for (int i = 0; i < 3; i++)
        {
          if ( (i != i1_neigh) && (i != i2_neigh) )
          {
            i0_neigh = i;
            break;
          }
        }  
        int newT2_index = TList->size();
        Triangle newT2(newT2_index,
                       T_neigh->points[i1_neigh],
                       newray_pointer, //&((*RList)[newray_index]),
                       T_neigh->points[i0_neigh],
                       T->neighs[i0],
                       T_neigh->neighs[i2_neigh],
                       newT1_pointer); //&((*TList)[newT1_index]));

        newT2.status = T_neigh->status;
        //cout << "newT2_index = " << newT2_index << endl;
      // ... and store it
      
      //(*TList).push_back(newT2);
      //Triangle* newT2_pointer = &(*TList)[newT2_index];//.push_back(newT2);
      Triangle* newT2_pointer = (*TList).push_back(newT2);

      // Tell neighbor triangle to have new triangle as neighbor
        for (int i = 0; i < 3; i++)
        {
          if ( T_neigh->neighs[i2_neigh] != NULL)
          {
            if ( T_neigh->neighs[i2_neigh]->neighs[i] == T->neighs[i0] )
            {
              T_neigh->neighs[i2_neigh]->neighs[i] = newT2_pointer; //&(*TList)[newT2_index];
              break;
            }
          }
        }
      // Change source triangle to be the other part
        T_neigh->neighs[i2_neigh] = newT2_pointer; //&(*TList)[newT2_index];
        T_neigh->points[i1_neigh] = newray_pointer; //&(*RList)[newray_index];

      // Set newT2 triangle to be newT1's neighbor
        newT1_pointer->neighs[2] = newT2_pointer; //&(*TList)[newT2_index];
//        (*TList)[newT1_index].neighs[2] = &(*TList)[newT2_index];
      }

      //cout << "Subdivide(newT1_index)\n" << flush;
      //Subdivide(newT1_index);
      //cout << "Subdivide(iT)\n" << flush;
      //Subdivide(iT);
//    }

  }
  return 0;
}
/** No descriptions */
template<class TracingOperator_T> 
int RefinementOperator<TracingOperator_T>::CheckDistances(const Triangle& T, int* ld){
  int ldn = 0;

  int d01 = 0; int d12 = 0; int d20 = 0;
//  std::cout << (T.points[0]) << " " << std::flush;
//  std::cout << (T.points[1]) << " " << std::flush;
//  std::cout << (T.points[2]) << std::endl << std::flush;
//
//  std::cout << (*(T.points[0])).status << (*(T.points[1])).status << (*(T.points[2])).status << std::endl << std::flush;
  if ( ((*(T.points[0])).mainray.status == T.status)
    && ((*(T.points[1])).mainray.status == T.status)
    && ((*(T.points[2])).mainray.status == T.status))
  {
//  cout <<  ((*RList)[T.points[0]]).x[0] - ((*RList)[T.points[1]]).x[0] << endl;

    if (T.points[0]->mainray.history == T.points[1]->mainray.history)
      // && (T.points[0]->kmah == T.points[1]->kmah))
    {
      const point3D<float> p_diff = T.points[0]->mainray.x - T.points[1]->mainray.x;
      d01 = static_cast<int>(sqrt( p_diff[0]*p_diff[0] + p_diff[1]*p_diff[1] + p_diff[2]*p_diff[2]));
    }

    if (T.points[1]->mainray.history == T.points[2]->mainray.history)
      // && (T.points[1]->kmah == T.points[2]->kmah))
    {
      const point3D<float> p_diff = T.points[1]->mainray.x - T.points[2]->mainray.x;
      d12 = static_cast<int>(sqrt( p_diff[0]*p_diff[0] + p_diff[1]*p_diff[1] + p_diff[2]*p_diff[2]));
    }

    if (T.points[2]->mainray.history == T.points[0]->mainray.history)
      // && (T.points[2]->kmah == T.points[0]->kmah))
    {
      const point3D<float> p_diff = T.points[2]->mainray.x - T.points[0]->mainray.x;
      d20 = static_cast<int>(sqrt( p_diff[0]*p_diff[0] + p_diff[1]*p_diff[1] + p_diff[2]*p_diff[2]));
    }


    if ( ((d01 < min_line) || (d12 < min_line) || (d20 < min_line))
         && ( 2*(d01+d12+d20) > 3*min_line) )
      {
//         std::cout << "Some distances are smaller than min_line\n";
//         std::cout << d01 << " " << d12 << " " << d20 << std::endl;
        ldn = -1;
        if ( (d01 < min_line) && (d01 <= d12) && (d01 <= d20) )
          ld[0] = 2;
        else if ( (d12 < min_line) && (d12 <= d01) && (d12 <= d20) )
          ld[0] = 0;
        else if ( (d20 < min_line) && (d20 <= d12) && (d20 <= d01) )
          ld[0] = 1;
        return ldn;
      }

    if ( (d01 > max_line) && (d01 >= d12) && (d01 >= d20) )
    {
      ldn = 1;
      ld[0] = 2;
    }
    else
      if ( (d12 > max_line) && (d12 >= d01) && (d12 >= d20) )
      {
        ldn = 1;
        ld[0] = 0;
      }
      else
        if ( (d20 > max_line) && (d20 >= d01) && (d20 >= d12) )
        {
          ldn = 1;
          ld[0] = 1;
        }
  }


//  std::cout << "CheckDistances done\n" << std::flush;
  return ldn;
}
/** No descriptions */
template<class TracingOperator_T> 
ray3D RefinementOperator<TracingOperator_T>::InterpolateRay(const ray3D& r1, const ray3D& r2){

  const double x0 = (r1.x[0] + r2.x[0])/2;
  const double x1 = (r1.x[1] + r2.x[1])/2;
  const double x2 = (r1.x[2] + r2.x[2])/2;

  const point3D<float> x( x0, x1, x2);

  const double v =  (r1.v + r2.v)/2;
  const double p0 = (r1.p[0] + r2.p[0])/2;
  const double p1 = (r1.p[1] + r2.p[1])/2;
  const double p2 = (r1.p[2] + r2.p[2])/2;

  point3D<float> p( p0, p1, p2);
  p = p/(p.norm() * v);


//   const Spherical Dir( (r1.StartDir.phi + r2.StartDir.phi) / 2.0,
//                        (r1.StartDir.theta + r2.StartDir.theta) / 2.0);
  const Spherical Dir( atan2(sin(r1.StartDir.phi)+sin(r2.StartDir.phi),cos(r1.StartDir.phi)+cos(r2.StartDir.phi)),                      (r1.StartDir.theta + r2.StartDir.theta) / 2.0);

  const int kmah = r1.kmah;

  ray3D newRay(x, p, v, Dir, kmah);

  for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
        newRay.Q[i][j] = (r1.Q[i][j] + r2.Q[i][j]) / 2.0;
      for (int j = 0; j < 2; j++)
        newRay.P[i][j] = (r1.P[i][j] + r2.P[i][j]) / 2.0;
    }

  newRay.detQ = (r1.detQ + r2.detQ) / 2.0;
  newRay.detQ_old = (r1.detQ + r2.detQ_old) / 2.0;
        
  return newRay;
}
/** No descriptions */
template<class TracingOperator_T> 
kinray3D RefinementOperator<TracingOperator_T>::InterpolateRay(const kinray3D& r1, const kinray3D& r2){

  const double x0 = (r1.x[0] + r2.x[0])/2;
  const double x1 = (r1.x[1] + r2.x[1])/2;
  const double x2 = (r1.x[2] + r2.x[2])/2;

  const point3D<float> x( x0, x1, x2);

  const double v =  (r1.v + r2.v)/2;
  const double p0 = (r1.p[0] + r2.p[0])/2;
  const double p1 = (r1.p[1] + r2.p[1])/2;
  const double p2 = (r1.p[2] + r2.p[2])/2;

  point3D<float> p( p0, p1, p2);
  p = p/(p.norm() * v);

//   const Spherical Dir( (r1.StartDir.phi + r2.StartDir.phi) / 2.0,
//                        (r1.StartDir.theta + r2.StartDir.theta) / 2.0);
  const Spherical Dir( atan2(sin(r1.StartDir.phi)+sin(r2.StartDir.phi),cos(r1.StartDir.phi)+cos(r2.StartDir.phi)),                      (r1.StartDir.theta + r2.StartDir.theta) / 2.0);

  kinray3D newRay(x, p, v, Dir);

  return newRay;
}
/** No descriptions */
template<class TracingOperator_T> 
triray3D RefinementOperator<TracingOperator_T>::InterpolateRay(const triray3D& r1, const triray3D& r2, const float& t)
{
//   Spherical Dir( (r1.mainray.StartDir.phi + r2.mainray.StartDir.phi) / 2.0,
//                        (r1.mainray.StartDir.theta + r2.mainray.StartDir.theta) / 2.0);
  Spherical Dir( atan2(sin(r1.mainray.StartDir.phi)+sin(r2.mainray.StartDir.phi),cos(r1.mainray.StartDir.phi)+cos(r2.mainray.StartDir.phi)), (r1.mainray.StartDir.theta + r2.mainray.StartDir.theta) / 2.0);

  if ( fabs(r1.mainray.StartDir.theta) < 1e-6)
    Dir.phi = r2.mainray.StartDir.phi;
  if ( fabs(r2.mainray.StartDir.theta) < 1e-6)
    Dir.phi = r1.mainray.StartDir.phi;

  const int kmah = r1.mainray.kmah;

  triray3D newRay = triray3D(Source->CreateRay(Dir));

  if (acos((cos(r1.mainray.StartDir.phi)*cos(r2.mainray.StartDir.phi)+sin(r1.mainray.StartDir.phi)*sin(r2.mainray.StartDir.phi))*sin(r1.mainray.StartDir.theta)*sin(r2.mainray.StartDir.theta)+cos(r1.mainray.StartDir.theta)*cos(r2.mainray.StartDir.theta))*180./M_PI > 0.01)
/*
  if ( (fabs(Dir.phi - r1.mainray.StartDir.phi) > 1e-3) || (fabs(Dir.theta - r1.mainray.StartDir.theta) > 1e-3) )*/
    {

      Tracer->TraceT(newRay, t, ith);

      if ( ((newRay.mainray.x - r1.mainray.x).norm() > max_line) || ((newRay.mainray.x - r2.mainray.x).norm() > max_line))
        {
          ray3D newmainRay = InterpolateRay(r1.mainray ,  r2.mainray);
          kinray3D newdxRay = InterpolateRay(r1.dxray ,  r2.dxray);
          kinray3D newdyRay = InterpolateRay(r1.dyray ,  r2.dyray);

	  newRay.mainray = newmainRay;
	  newRay.dxray = newdxRay;
	  newRay.dyray = newdyRay;
        }
    }
  else
    {
	ray3D newmainRay = InterpolateRay(r1.mainray ,  r2.mainray);
	kinray3D newdxRay = InterpolateRay(r1.dxray ,  r2.dxray);
	kinray3D newdyRay = InterpolateRay(r1.dyray ,  r2.dyray);
	
	newRay.mainray = newmainRay;
	newRay.dxray = newdxRay;
	newRay.dyray = newdyRay;
    }

  return newRay;
}
/** No descriptions */
template<class TracingOperator_T> 
void RefinementOperator<TracingOperator_T>::DeleteTri(Triangle* T)
{
  if ( T != NULL)
    {
      T->lifesign = Triangle::DELETED;
      for (int i = 0; i < 3; i++)
        {
          Triangle* NT = T->neighs[i];
          if (NT != NULL)
            {
              for (int j = 0; j < 3; j++)
                {
                  if (NT->neighs[j] == T)
                    {
                      NT->neighs[j] = NULL;
                      break;
                    }
                }
            }
        }
    }
}
