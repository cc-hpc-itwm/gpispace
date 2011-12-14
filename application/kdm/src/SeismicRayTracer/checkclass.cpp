/***************************************************************************
                          checkclass.cpp  -  description
                             -------------------
    begin                : Tue Jan 24 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "checkclass.h"

CheckClass::CheckClass(){
}
CheckClass::~CheckClass(){
}
/** No descriptions */
bool CheckClass::CheckTriangleList(triMem& TriangleList){

  bool MisRet = false;
  std::cout << "// Check TriangleList start\n";
  int number = 0;
  triMem::iterator iter_end(TriangleList.end());
  for (triMem::iterator iter(TriangleList.begin()); iter != iter_end; ++iter)
  {
    number++;
  }
  if (number !=  TriangleList.size())
  {
    std::cout << "Size of TriangleList differs from iterator iter_end :\n";
    std::cout << " Size = " << TriangleList.size() << " != iter_end =^ " << number << std::endl;
  }


  for (int i = 0; i < TriangleList.size(); i++)
  {
    bool mismatch = false;
    if (TriangleList[i].lifesign != Triangle::DELETED)
    {
      for ( int i_n = 0; i_n < 3; i_n++)
      {
        Triangle* n0 = TriangleList[i].neighs[i_n];
        if (n0 != NULL)
        {
          int i_n_base = -1;
          for (int i_nb = 0; i_nb < 3; i_nb++)
          {
            if (n0->neighs[i_nb] == &(TriangleList[i]))
            {
              i_n_base = i_nb;
              break;
            }
          }

          if ( i_n_base == -1)
          {
            mismatch = true;
            MisRet = true;            
          }
          else
          {
            if (    ( (n0->points[ (i_n_base + 1)%3 ] == TriangleList[i].points[ (i_n + 1)%3 ])
                 && ( n0->points[ (i_n_base + 2)%3 ] == TriangleList[i].points[ (i_n + 2)%3 ]))
                 || ( (n0->points[ (i_n_base + 1)%3 ] == TriangleList[i].points[ (i_n + 2)%3 ])
                 && ( n0->points[ (i_n_base + 2)%3 ] == TriangleList[i].points[ (i_n + 1)%3 ]))  )
//             || ( (TriangleList[n0].points[ (i_n_base + 1)%3 ] == TriangleList[i].points[ (n0 + 1)%3 ])
//                 && ( TriangleList[n0].points[ (i_n_base + 2)%3 ] == TriangleList[i].points[ (n0 + 2)%3 ]))
//             || ( (TriangleList[n0].points[ (i_n_base + 1)%3 ] == TriangleList[i].points[ (n0 + 1)%3 ])
//                 && ( TriangleList[n0].points[ (i_n_base + 2)%3 ] == TriangleList[i].points[ (n0 + 2)%3 ])) )
            {
              mismatch = false; 
            }
            else
            {
              mismatch = true;
              MisRet = true;
            }
          }
         }
       }
    }

    if (mismatch)
    {
      std::cout << "Found mismatch: Triangle index " << i << "\n";
      std::cout << "Triangle points: " << TriangleList[i].points[0] << ", "
                                       << TriangleList[i].points[1] << ", "
                                       << TriangleList[i].points[2] << std::endl;
      std::cout << "Triangle neighs: " << TriangleList[i].neighs[0] << ", "
                                       << TriangleList[i].neighs[1] << ", "
                                       << TriangleList[i].neighs[2] << std::endl;

      Triangle* n0 = TriangleList[i].neighs[0];
      if (n0 != NULL){
      std::cout << "Neighbor 0 : Index " << n0 << std::endl;
      std::cout << "Triangle points: " << n0->points[0] << ", "
                                       << n0->points[1] << ", "
                                       << n0->points[2] << std::endl;
      std::cout << "Triangle neighs: " << n0->neighs[0] << ", "
                                       << n0->neighs[1] << ", "
                                       << n0->neighs[2] << std::endl;
      }

      Triangle* n1 = TriangleList[i].neighs[1];
      if (n1 != NULL){
      std::cout << "Neighbor 1 : Index " << n1 << std::endl;
      std::cout << "Triangle points: " << n1->points[0] << ", "
                                       << n1->points[1] << ", "
                                       << n1->points[2] << std::endl;
      std::cout << "Triangle neighs: " << n1->neighs[0] << ", "
                                       << n1->neighs[1] << ", "
                                       << n1->neighs[2] << std::endl;
      }

      Triangle* n2 = TriangleList[i].neighs[2];
      if (n2 != NULL){
      std::cout << "Neighbor 2 : Index " << n2 << std::endl;
      std::cout << "Triangle points: " << n2->points[0] << ", "
                                       << n2->points[1] << ", "
                                       << n2->points[2] << std::endl;
      std::cout << "Triangle neighs: " << n2->neighs[0] << ", "
                                       << n2->neighs[1] << ", "
                                       << n2->neighs[2] << std::endl;
      }
    }

  }
  std::cout << "// Check TriangleList end\n";
  return MisRet;
}
/** No descriptions */
void CheckClass:: CheckMemoryAccess(){

#define DIMENSION 250  

  int Nx = DIMENSION;
  int Ny = DIMENSION;
  int Nz = DIMENSION;

  // allocate memory
  //MemMan3D_malloc<VelGridPoint> My_MemMan(Nx, Ny, Nz);
  //MemMan3D_vector<VelGridPoint> My_MemMan(Nx, Ny, Nz);
  //MemMan3D_stlvector<VelGridPoint> My_MemMan(Nx, Ny, Nz);
  MemMan3D_bucket<VelGridPoint> My_MemMan(Nx, Ny, Nz);

  // init memory for the velocity field.
  // Velocity field points are referenced by integer indicees
  for (int k = 0; k < Nz; k++)
    for (int j = 0; j < Ny; j++)
      for (int i = 0; i < Nx; i++)
       {
        point3D<float> P(i, j, k);
        double mypoint_data = 1;//sqrt(i*i + j*j + k*k);
        VelGridPoint mypoint(P, mypoint_data);
        My_MemMan.InitPoint(i, j, k, mypoint);
      }

  // access memory
  std::cout << "Starting to access the Memory\n" << std::flush;

  Timer mytimer;

  double add = 0;
  for (int k = 0; k < Nz; k+=1)
    for (int j = 0; j < Ny; j+=1)
      for (int i = 0; i < Nx; i+=1)
      {
         double myoint_data = My_MemMan.GetPointAt(i,j,k)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i+1)%Nx,(j)%Ny,(k)%Nz)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i+Nx)%Nx,(j)%Ny,(k)%Nz)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i)%Nx,(j+1)%Ny,(k)%Nz)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i)%Nx,(j+Ny)%Ny,(k)%Nz)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i)%Nx,(j)%Ny,(k+1)%Nz)->vel;
         add += myoint_data;
         myoint_data = My_MemMan.GetPointAt((i)%Nx,(j)%Ny,(k+Nz)%Nz)->vel;
         add += myoint_data;
       }

  double tout = mytimer.GetTime();
  std::cout << "add = " << add << std::endl;
  std::cout << "This took " << tout << " seconds\n";
}

void CheckClass::CheckVelocityField(PropModel<VelGridPointBSpline>& VF)
{
#define DIMENSION 250

  int Nx = DIMENSION;
  int Ny = DIMENSION;
  int Nz = DIMENSION;

  float x0, y0, z0;
  float dx, dy, dz;
  x0 = -250; y0 = -250, z0 = -100;
  dx = 500.0 / (Nx-1); dy = 500.0 / (Ny-1), dz = 1000.0 / (Nz-1);

  for (int k = 0; k < DIMENSION-1; k++)
    for (int j = 0; j < DIMENSION-1; j++)
      for (int i = 0; i < DIMENSION-1; i++)
       {
        float mypoint_data = (1000 + 0.01 * sqrt((double) i*i + j*j + k*k));
        float x = x0 + i*dx;
        float y = y0 + j*dy;
        float z = z0 + k*dz;

        point3D<float> xp(x, y, z);
        float v = VF.GetProperty(xp);
        if ( fabs( (v-mypoint_data)/(v+mypoint_data)) > 1e-3 )
          std::cout << i << " " << j << " " << k << " : " << v << " != " << mypoint_data << std::endl;
        }
}

