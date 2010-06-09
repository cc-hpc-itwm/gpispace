/***************************************************************************
                          polyhedron.cpp  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "polyhedron.h"

PolyHedron::PolyHedron(){
}
PolyHedron::~PolyHedron(){
}
/** A PolyHedron with a minimum number of Nmin Verticees is generated. */
void PolyHedron::Init_N(const int& Nmin){

  Icosahedron Base;

  N_V = Base.N_V; N_F = Base.N_F;

  int n_it = 0;
  while (N_V < Nmin)
  {
    N_F = 4*N_F;
    N_V = (3*N_F + Base.N_V) / 6;
    n_it++;
  }

  Vertices.clear();
  Vertices.reserve(N_V);

  Vertices.resize(Base.N_V);
  for (int i = 0; i < Base.N_V; i++)
    Vertices[i] = Base.Vertices[i];

  Faces.clear();
  Faces.resize(N_F);

  for (int i = 0; i < Base.N_F; i++)
    Faces[ i << (2*n_it)] = Base.Faces[i];

  for (int it = 0; it < n_it; it++)
  {
    //std::cout << "new refinement to \n";
    int N_F_i = Base.N_F << (2*it);
    int N_V_i = (3*N_F_i + Base.N_V) / 6;

    int** new_vertices = new int*[ N_V_i ];
    for (int i = 0; i < N_V_i; i++)
    {
      new_vertices[i] = new int[i];
      for (int k = 0; k < i; k++)
        new_vertices[i][k] = -1;
    }

    // generate new vertices
    for (int i = 0; i < N_F_i; i++)
    {
      const int i_f = i << (2*(n_it - it));
      for (int j = 0; j < 3; j++)
      {
        int i0, i1;
        if ( Faces[i_f].v[j] < Faces[i_f].v[(j+1)%3])
        {
          i0 = Faces[i_f].v[j]; i1 = Faces[i_f].v[(j+1)%3];
        }
        else
        {
          i0 = Faces[i_f].v[(j+1)%3]; i1 = Faces[i_f].v[j];
        }

        if ( new_vertices[i1][i0] == -1)
        {
          new_vertices[i1][i0] = Vertices.size();
          const point3D<float> _p = FaceUtils.MidLine(Vertices[i0], Vertices[i1]);
          Vertices.push_back(_p);
        }
      }
    }

    // generate new faces
    for (int i = 0; i < N_F_i; i++)
    {
      const int i_f = i << (2*(n_it - it));
      const int i_f_d = 1 << (2*(n_it - it - 1));
      int i01, i10;
      if (Faces[i_f].v[0]<Faces[i_f].v[1])
      {
        i01 = Faces[i_f].v[0]; i10 = Faces[i_f].v[1];
      }
      else
      {
        i01 = Faces[i_f].v[1]; i10 = Faces[i_f].v[0];
      }
      int i02, i20;
      if (Faces[i_f].v[0]<Faces[i_f].v[2])
      {
        i02 = Faces[i_f].v[0]; i20 = Faces[i_f].v[2];
      }
      else
      {
        i02 = Faces[i_f].v[2]; i20 = Faces[i_f].v[0];
      }
      int i12, i21;
      if (Faces[i_f].v[1]<Faces[i_f].v[2])
      {
        i12 = Faces[i_f].v[1]; i21 = Faces[i_f].v[2];
      }
      else
      {
        i12 = Faces[i_f].v[2]; i21 = Faces[i_f].v[1];
      }

//      polygon new_face0;
//      new_face0.v[0] = new_vertices[i21][i12];
//      new_face0.v[1] = new_vertices[i10][i01];
//      new_face0.v[2] = new_vertices[i20][i02];
//      Faces[i_f + i_f_d] = new_face0;
//
//      polygon new_face1;
//      new_face1.v[1] = Faces[i_f].v[1];
//      new_face1.v[0] = new_vertices[i10][i01];
//      new_face1.v[2] = new_vertices[i21][i12];
//      Faces[i_f + 2*i_f_d] = new_face1;
//
//      polygon new_face2;
//      new_face2.v[2] = Faces[i_f].v[2];
//      new_face2.v[0] = new_vertices[i20][i02];
//      new_face2.v[1] = new_vertices[i21][i12];
//      Faces[i_f + 3*i_f_d] = new_face2;
//
//      polygon new_face3;
//      Faces[i_f].v[0] = Faces[i_f].v[0];
//      Faces[i_f].v[1] = new_vertices[i10][i01];
//      Faces[i_f].v[2] = new_vertices[i20][i02];

      polygon new_face0;
      new_face0.v[0] = Faces[i_f].v[0];
      new_face0.v[1] = new_vertices[i10][i01];
      new_face0.v[2] = new_vertices[i20][i02];
      Faces[i_f + i_f_d] = new_face0;

      polygon new_face1;
      new_face1.v[1] = Faces[i_f].v[1];
      new_face1.v[0] = new_vertices[i10][i01];
      new_face1.v[2] = new_vertices[i21][i12];
      Faces[i_f + 2*i_f_d] = new_face1;

      polygon new_face2;
      new_face2.v[2] = Faces[i_f].v[2];
      new_face2.v[0] = new_vertices[i20][i02];
      new_face2.v[1] = new_vertices[i21][i12];
      Faces[i_f + 3*i_f_d] = new_face2;

      polygon new_face3;
      Faces[i_f].v[0] = new_vertices[i21][i12];
      Faces[i_f].v[1] = new_vertices[i10][i01];
      Faces[i_f].v[2] = new_vertices[i20][i02];

    }

    for (int i = 0; i < N_V_i; i++)
      delete[] new_vertices[i];
    delete[] new_vertices;

    //std::cout << "Now we have N_F = " << N_F_i << ", N_V = " << N_V_i << "\n";

  }

  if ( Faces.size() != N_F)
    std::cerr << "Dimension mismatch for the faces: " << Faces.size() << " != " << N_F << std::endl;

  if ( Vertices.size() != N_V)
    std::cerr << "Dimension mismatch for the vertices: " << Vertices.size() << " != " << N_V << std::endl;

//  Vertices.reserve(N_V);
//  for (int i = 0; i < N_V; i++)
//    Vertices.push_back(Base.Vertices[i]);
//
//  Faces.reserve(N_F);
//  for (int i = 0; i < N_F; i++)
//    Faces.push_back(Base.Faces[i]);
//
//  const int N_F_Base = N_F;
//
//  while (N_V < Nmin)
//  {
//    std::cout << "new refinement\n";
//    const int N_V_old = N_V;
//    int** new_vertices = new int*[N_V];
//    for (int i = 0; i < N_V; i++)
//    {
//      new_vertices[i] = new int[i];
//      for (int k = 0; k < i; k++)
//        new_vertices[i][k] = -1;
//    }
//
//    // generate new vertices
//    for (int i = 0; i < N_F; i++)
//    {
//      for (int j = 0; j < 3; j++)
//      {
//        int i0, i1;
//        if ( Faces[i].v[j] < Faces[i].v[(j+1)%3])
//        {
//          i0 = Faces[i].v[j]; i1 = Faces[i].v[(j+1)%3];
//        }
//        else
//        {
//          i0 = Faces[i].v[(j+1)%3]; i1 = Faces[i].v[j];
//        }
//
//        if ( new_vertices[i1][i0] == -1)
//        {
//          new_vertices[i1][i0] = Vertices.size();
//          const point3D<> _p = FaceUtils.MidLine(Vertices[i0], Vertices[i1]);
//          Vertices.push_back(_p);
//          N_V++;
//        }
//      }
//    }
//
//    // generate new faces
//    for (int i = 0; i < N_F; i++)
//    {
//      int i01, i10;
//      if (Faces[i].v[0]<Faces[i].v[1])
//      {
//        i01 = Faces[i].v[0]; i10 = Faces[i].v[1];
//      }
//      else
//      {
//        i01 = Faces[i].v[1]; i10 = Faces[i].v[0];
//      }
//      int i02, i20;
//      if (Faces[i].v[0]<Faces[i].v[2])
//      {
//        i02 = Faces[i].v[0]; i20 = Faces[i].v[2];
//      }
//      else
//      {
//        i02 = Faces[i].v[2]; i20 = Faces[i].v[0];
//      }
//      int i12, i21;
//      if (Faces[i].v[1]<Faces[i].v[2])
//      {
//        i12 = Faces[i].v[1]; i21 = Faces[i].v[2];
//      }
//      else
//      {
//        i12 = Faces[i].v[2]; i21 = Faces[i].v[1];
//      }
//
//      polygon new_face0;
//      new_face0.v[0] = Faces[i].v[0];
//      new_face0.v[1] = new_vertices[i10][i01];
//      new_face0.v[2] = new_vertices[i20][i02];
//      Faces.push_back(new_face0);
//
//      polygon new_face1;
//      new_face1.v[1] = Faces[i].v[1];
//      new_face1.v[0] = new_vertices[i10][i01];
//      new_face1.v[2] = new_vertices[i21][i12];
//      Faces.push_back(new_face1);
//
//      polygon new_face2;
//      new_face2.v[2] = Faces[i].v[2];
//      new_face2.v[0] = new_vertices[i20][i02];
//      new_face2.v[1] = new_vertices[i21][i12];
//      Faces.push_back(new_face2);
//
//      polygon new_face3;
//      Faces[i].v[0] = new_vertices[i21][i12];
//      Faces[i].v[1] = new_vertices[i10][i01];
//      Faces[i].v[2] = new_vertices[i20][i02];
//
//    }
//    N_F = 4*N_F;
//
//    for (int i = 0; i < N_V_old; i++)
//      delete[] new_vertices[i];
//    delete[] new_vertices;
//
//    std::cout << "Now we have N_F = " << N_F << ", N_V = " << N_V << "\n";
//  }


  // clear the neighboring information
  for (int i = 0; i < Faces.size(); i++)
    for (int j = 0; j < 3; j++)
    {
      Faces[i].n[j] = -1;
    }

//  for (int i = 0; i < Faces.size(); i++)
//  {
//    std::cout << "polygon " << i << std::endl;
//    std::cout << Faces[i].v[0] << " " << Faces[i].v[1] << " " << Faces[i].v[2] << std::endl;
//    std::cout << Faces[i].n[0] << " " << Faces[i].n[1] << " " << Faces[i].n[2] << std::endl;
//  }


  for (int i = 0; i < N_F-1; i++)
    for (int j = 0; j < 3; j++)
    {
      if (Faces[i].n[j] != -1)
        continue;

      const int i1 = j;
      const int i2 = (j+1)%3;
      const int i3 = (j+2)%3;


      for (int k = i+1; k < N_F; k++)
      {
        for (int l = 0; l < 3; l++)
        {
          const int l1 = l;
          const int l2 = (l+1)%3;
          const int l3 = (l+2)%3;

          if (    ( (Faces[i].v[i2] == Faces[k].v[l1]) && (Faces[i].v[i3] == Faces[k].v[l2]) )
          || ( (Faces[i].v[i3] == Faces[k].v[l1]) && (Faces[i].v[i2] == Faces[k].v[l2]) ) )
          {
            Faces[i].n[i1] = k;
            Faces[k].n[l3] = i;

//            std::cout << "Setting Face[" << i << "].n[" << i1 << "] = " << k << std::endl;
//            std::cout << "      because " << Faces[i].v[i2] << " == " << Faces[k].v[l1] << " && " << Faces[i].v[i3] << " == " << Faces[k].v[l2] << std::endl;
//            std::cout << "      or      " << Faces[i].v[i3] << " == " << Faces[k].v[l1] << " && " << Faces[i].v[i2] << " == " << Faces[k].v[l2] << std::endl;
//
            l=3; k=N_F; // break;
          }
        }

      }
    }
//  for (int i = 0; i < Faces.size(); i++)
//  {
//    std::cout << "polygon " << i << std::endl;
//    std::cout << Faces[i].v[0] << " " << Faces[i].v[1] << " " << Faces[i].v[2] << std::endl;
//    std::cout << Faces[i].n[0] << " " << Faces[i].n[1] << " " << Faces[i].n[2] << std::endl;
//  }

  width = 2 * N_V / Base.N_F;
}

/** A PolyHedron with a given angle resolution is generated. */
void PolyHedron::Init(const float& dang, const float& beta)
{
  std::cerr << "Sorry. A PolyHedron with given angle resolution not implemented yet!\n";
};
/** A PolyHedron Cap from a PolyHedron with a minimum number of Nmin Verticees pointing into  direction X with opening angle beta is generated. */
void PolyHedron::Init(const int& Nmin, const point3D<float>& X, const float& beta)
{
  Icosahedron Base;

  N_V = Base.N_V; N_F = Base.N_F;

  int n_it = 0;
  while (N_V < Nmin)
  {
    N_F = 4*N_F;
    N_V = (3*N_F + Base.N_V) / 6;
    n_it++;
  }

  Vertices.clear();
  Vertices.reserve(N_V);

  Vertices.resize(Base.N_V);
  for (int i = 0; i < Base.N_V; i++)
    Vertices[i] = Base.Vertices[i];

  Faces.clear();
  Faces.resize(N_F);

  for (int i = 0; i < Base.N_F; i++)
    Faces[ i << (2*n_it)] = Base.Faces[i];

  for (int it = 0; it < n_it; it++)
  {
    //std::cout << "new refinement to \n";
    int N_F_i = Base.N_F << (2*it);
    int N_V_i = (3*N_F_i + Base.N_V) / 6;

    int** new_vertices = new int*[ N_V_i ];
    for (int i = 0; i < N_V_i; i++)
    {
      new_vertices[i] = new int[i];
      for (int k = 0; k < i; k++)
        new_vertices[i][k] = -1;
    }

    // generate new vertices
    for (int i = 0; i < N_F_i; i++)
    {
      const int i_f = i << (2*(n_it - it));
      for (int j = 0; j < 3; j++)
      {
        int i0, i1;
        if ( Faces[i_f].v[j] < Faces[i_f].v[(j+1)%3])
        {
          i0 = Faces[i_f].v[j]; i1 = Faces[i_f].v[(j+1)%3];
        }
        else
        {
          i0 = Faces[i_f].v[(j+1)%3]; i1 = Faces[i_f].v[j];
        }

        if ( new_vertices[i1][i0] == -1)
        {
          new_vertices[i1][i0] = Vertices.size();
          const point3D<float> _p = FaceUtils.MidLine(Vertices[i0], Vertices[i1]);
          Vertices.push_back(_p);
        }
      }
    }

    // generate new faces
    for (int i = 0; i < N_F_i; i++)
    {
      const int i_f = i << (2*(n_it - it));
      const int i_f_d = 1 << (2*(n_it - it - 1));
      int i01, i10;
      if (Faces[i_f].v[0]<Faces[i_f].v[1])
      {
        i01 = Faces[i_f].v[0]; i10 = Faces[i_f].v[1];
      }
      else
      {
        i01 = Faces[i_f].v[1]; i10 = Faces[i_f].v[0];
      }
      int i02, i20;
      if (Faces[i_f].v[0]<Faces[i_f].v[2])
      {
        i02 = Faces[i_f].v[0]; i20 = Faces[i_f].v[2];
      }
      else
      {
        i02 = Faces[i_f].v[2]; i20 = Faces[i_f].v[0];
      }
      int i12, i21;
      if (Faces[i_f].v[1]<Faces[i_f].v[2])
      {
        i12 = Faces[i_f].v[1]; i21 = Faces[i_f].v[2];
      }
      else
      {
        i12 = Faces[i_f].v[2]; i21 = Faces[i_f].v[1];
      }

      polygon new_face0;
      new_face0.v[0] = Faces[i_f].v[0];
      new_face0.v[1] = new_vertices[i10][i01];
      new_face0.v[2] = new_vertices[i20][i02];
      Faces[i_f + i_f_d] = new_face0;

      polygon new_face1;
      new_face1.v[1] = Faces[i_f].v[1];
      new_face1.v[0] = new_vertices[i10][i01];
      new_face1.v[2] = new_vertices[i21][i12];
      Faces[i_f + 2*i_f_d] = new_face1;

      polygon new_face2;
      new_face2.v[2] = Faces[i_f].v[2];
      new_face2.v[0] = new_vertices[i20][i02];
      new_face2.v[1] = new_vertices[i21][i12];
      Faces[i_f + 3*i_f_d] = new_face2;

      polygon new_face3;
      Faces[i_f].v[0] = new_vertices[i21][i12];
      Faces[i_f].v[1] = new_vertices[i10][i01];
      Faces[i_f].v[2] = new_vertices[i20][i02];

    }

    for (int i = 0; i < N_V_i; i++)
      delete[] new_vertices[i];
    delete[] new_vertices;

    //std::cout << "Now we have N_F = " << N_F_i << ", N_V = " << N_V_i << "\n";

  }

  if ( Faces.size() != N_F)
    std::cerr << "Dimension mismatch for the faces: " << Faces.size() << " != " << N_F << std::endl;

  if ( Vertices.size() != N_V)
    std::cerr << "Dimension mismatch for the vertices: " << Vertices.size() << " != " << N_V << std::endl;

// delete Vertices and Faces that are outside of the opening angle
  int inew = 0;
  int* NewVIndex = new int[N_V];
  point3D<float>* New_Vertices = new point3D<float>[N_V];  
  if ( beta >= 0)
    {
      for (int i = 0; i < N_V; i++)
        {
          if (acos(Vertices[i][2]) <= beta)
            {
              New_Vertices[inew] = Vertices[i];
              NewVIndex[i] = inew;
              inew++;
            }
          else // mark Vertex as deleted
            NewVIndex[i] = -1;
        }
    }
  else
    {
      for (int i = 0; i < N_V; i++)
        {
          if ((_PI - acos(Vertices[i][2])) <= -beta)
            {
              New_Vertices[inew] = Vertices[i];
              NewVIndex[i] = inew;
              inew++;
            }
          else // mark Vertex as deleted
            NewVIndex[i] = -1;
        }
    }

  N_V = inew;
  Vertices.resize(N_V);
  // copy New_Vertices to Vertices and rotate (0,0,1) to X
  const float Dxyz = sqrt(X[0]*X[0] + X[1]*X[1]  + X[2]*X[2]);
  const float Dxy = sqrt(X[0]*X[0] + X[1]*X[1]);
  float cphi;
  float sphi;
  if (Dxy > 1e-4)
  {
    cphi = X[0]/Dxy;
    sphi = X[1]/Dxy;
  }
  else
  {
    cphi = 1;
    sphi = 0;
  }
  const float ctht = Dxy/Dxyz;
  const float stht = X[2]/Dxyz;

//  std::cout << "phi: " << cphi << ", " << sphi << std::endl;
//  std::cout << "tht: " << ctht << ", " << stht << std::endl;
//  std::cout << "new z-axis X = " << X << std::endl;
//  std::cout << " =! " << -Dxyz*stht << " , 0, " << Dxyz*ctht << std::endl;
//  std::cout << "Rotations:\n";
//  std::cout << cphi*stht << " , " << sphi*stht << " , " << -ctht <<"\n";
//  std::cout << -sphi << " , " << cphi << " , 0\n";
//  std::cout << cphi*ctht << " , " << sphi*ctht << " , " << stht <<"\n";
  for (int i = 0; i < N_V; i++)
  {
    Vertices[i][0] =  cphi*stht*New_Vertices[i][0] + -sphi*New_Vertices[i][1]+ cphi*ctht*New_Vertices[i][2];
    Vertices[i][1] =  sphi*stht*New_Vertices[i][0] + cphi*New_Vertices[i][1] + sphi*ctht*New_Vertices[i][2];
    Vertices[i][2] =  -ctht*New_Vertices[i][0]                           + stht*New_Vertices[i][2];
  }
  delete[] New_Vertices;



  inew = 0;
  int* NewFIndex = new int[N_F];
  polygon* New_Faces = new polygon[N_F];
  for (int i = 0; i < N_F; i++)
  {
     if ( (NewVIndex[Faces[i].v[0]] != -1) && (NewVIndex[Faces[i].v[1]] != -1) && (NewVIndex[Faces[i].v[2]] != -1) )
     {
       New_Faces[inew] = Faces[i];
       NewFIndex[i] = inew;

       New_Faces[inew].v[0] = NewVIndex[Faces[i].v[0]];
       New_Faces[inew].v[1] = NewVIndex[Faces[i].v[1]];
       New_Faces[inew].v[2] = NewVIndex[Faces[i].v[2]];

       inew++;
     }
  }

  N_F = inew;
  Faces.resize(N_F);
  for (int i = 0; i < N_F; i++)
    Faces[i] = New_Faces[i];
  delete[] New_Faces;  

  delete[] NewFIndex;
  delete[] NewVIndex;

// generate the neighboring information
  for (int i = 0; i < Faces.size(); i++)
    for (int j = 0; j < 3; j++)
    {
      Faces[i].n[j] = -1;
    }


  for (int i = 0; i < N_F-1; i++)
    for (int j = 0; j < 3; j++)
    {
      if (Faces[i].n[j] != -1)
        continue;

      const int i1 = j;
      const int i2 = (j+1)%3;
      const int i3 = (j+2)%3;


      for (int k = i+1; k < N_F; k++)
      {
        for (int l = 0; l < 3; l++)
        {
          const int l1 = l;
          const int l2 = (l+1)%3;
          const int l3 = (l+2)%3;

          if (    ( (Faces[i].v[i2] == Faces[k].v[l1]) && (Faces[i].v[i3] == Faces[k].v[l2]) )
          || ( (Faces[i].v[i3] == Faces[k].v[l1]) && (Faces[i].v[i2] == Faces[k].v[l2]) ) )
          {
            Faces[i].n[i1] = k;
            Faces[k].n[l3] = i;

            l=3; k=N_F; // break;
          }
        }

      }
    }

  width = 2 * N_V / Base.N_F;

}

  /** A PolyHedron Cap from a PolyHedron with a given angle resolution pointing into  direction X with opening angle beta is generated. */
void PolyHedron::Init(const float& dang, const point3D<float>& X, const float& beta) {
  // each ray shall cover a spherical cap with radius 'dang' degree
  // --> space angle  2*pi*(1-cos(dang*180/pi)
  // dividing the total sphere (4*pi) by this caps gives the number of Vertices
  Init((int)round(2./(1.-cos(dang))), X, beta);
};

void PolyHedron::GetVertices(point3D<float>* _vert, const int& _N_V){
  for (int i = 0; i < _N_V; i++)
    _vert[i] = Vertices[i];
}

void PolyHedron::GetFaces(polygon* _faces, const int& _N_F){
  for (int i = 0; i < _N_F; i++)
    _faces[i] = Faces[i];
}

void PolyHedron::Output()
{
  for (int i = 0; i < Faces.size(); i++)
  {
    std::cout << i << " : ";
    for (int j = 0; j < 3; j++)
      std::cout << Faces[i].v[j] << " ";
    std::cout << std::endl << i << " : ";
    for (int j = 0; j < 3; j++)
      std::cout << Faces[i].n[j] << " ";
    std::cout << std::endl << std::endl;
  }
}
/** No descriptions */
void PolyHedron::AddPoint(const int&i, const point3D<float>& p){

  const int p_index = Vertices.size();
  std::cout << "p_index = " << p_index << std::endl;
  Vertices.push_back(p);
  N_V++;

  polygon face0;
  int face0_index = Faces.size();
  Faces.push_back(face0);
  std::cout << "face0_index = " << face0_index << std::endl;

  polygon face1;
  int face1_index = Faces.size();
  Faces.push_back(face1);
  std::cout << "face1_index = " << face1_index << std::endl;

  Faces[face0_index].v[0] = p_index;
  Faces[face0_index].v[1] = Faces[i].v[1];
  Faces[face0_index].v[2] = Faces[i].v[2];

  Faces[face0_index].n[0] = Faces[i].n[0];
  Faces[face0_index].n[1] = face1_index;
  Faces[face0_index].n[2] = i;

  int n0_index = SideIndex( Faces[face0_index].v[1],  Faces[face0_index].v[2],  Faces[face0_index].n[0]);
  Faces[Faces[face0_index].n[0]].n[n0_index] = face0_index;


  Faces[face1_index].v[0] = Faces[i].v[0];
  Faces[face1_index].v[1] = p_index;
  Faces[face1_index].v[2] = Faces[i].v[2];

  Faces[face1_index].n[0] = face0_index;
  Faces[face1_index].n[1] = Faces[i].n[1];
  Faces[face1_index].n[2] = i;

  int n1_index = SideIndex(Faces[face1_index].v[0], Faces[face1_index].v[2], Faces[face1_index].n[1]);
  if (n1_index == -1)
    std::cerr << "Side " << Faces[face1_index].v[0] << " -- " << Faces[face1_index].v[2]
         << " has not been found in " << Faces[face1_index].n[1] << " : "
         << Faces[Faces[face1_index].n[1]].v[0] << " " << Faces[Faces[face1_index].n[1]].v[1] << " " << Faces[Faces[face1_index].n[1]].v[2] << std::endl;
  Faces[Faces[face1_index].n[1]].n[n1_index] = face1_index;


  Faces[i].v[2] = p_index;
  Faces[i].n[0] = face0_index;
  Faces[i].n[1] = face1_index;

  N_F += 2;
}

int PolyHedron::SideIndex(const int& v1, const int& v2, const int& n0)
{
  int ni = -1;
  for (int l = 0; l < 3; l++)
  {
    int l1 = l;
    int l2 = (l+1)%3;
    int l3 = (l+2)%3;

    if (     ( (v1 == Faces[n0].v[l1]) && (v2 == Faces[n0].v[l2]) )
          || ( (v2 == Faces[n0].v[l1]) && (v1 == Faces[n0].v[l2]) ) )
     {
       ni = l3;
       break;
     }
   }

   return ni;
}
