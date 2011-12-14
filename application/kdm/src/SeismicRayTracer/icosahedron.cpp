/***************************************************************************
                          icosahedron.cpp  -  description
                             -------------------
    begin                : Wed Dec 7 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "icosahedron.h"

Icosahedron::Icosahedron(){
  N_V = 12;
  N_F = 20;
  
  // define some usefull constants;
  const double PI_5 = 0.628319;
  const double sin_w = 0.894427; const double cos_w = 0.44721;

  const double sin_PI_5 = sin(PI_5); const double cos_PI_5 = cos(PI_5);
  const double sin_2_PI_5 = sin(2*PI_5); const double cos_2_PI_5 = cos(2*PI_5);
  const double sin_3_PI_5 = sin(3*PI_5); const double cos_3_PI_5 = cos(3*PI_5);
  const double sin_4_PI_5 = sin(4*PI_5); const double cos_4_PI_5 = cos(4*PI_5);

  // Init the Vertices
  Vertices[0] = point3D<float>(0, 0, 1);

  Vertices[1] = point3D<float>( cos_PI_5 * sin_w, -sin_PI_5 * sin_w, cos_w);
  Vertices[2] = point3D<float>( cos_PI_5 * sin_w, sin_PI_5 * sin_w, cos_w);
  Vertices[3] = point3D<float>( cos_3_PI_5 * sin_w, sin_3_PI_5 * sin_w, cos_w);
  Vertices[4] = point3D<float>( -sin_w, 0, cos_w);
  Vertices[5] = point3D<float>( cos_3_PI_5 * sin_w, -sin_3_PI_5 * sin_w, cos_w);


  Vertices[6] = point3D<float>( sin_w, 0, -cos_w);
  Vertices[7] = point3D<float>( cos_2_PI_5 * sin_w, sin_2_PI_5 * sin_w, -cos_w);
  Vertices[8] = point3D<float>( cos_4_PI_5 * sin_w, sin_4_PI_5 * sin_w, -cos_w);
  Vertices[9] = point3D<float>( cos_4_PI_5 * sin_w, -sin_4_PI_5 * sin_w, -cos_w);
  Vertices[10] = point3D<float>( cos_2_PI_5 * sin_w, -sin_2_PI_5 * sin_w, -cos_w);

  Vertices[11] = point3D<float>(0, 0, -1);

  // Normalize the vertices, just to be sure ...
  for (int i = 0; i < N_V; i++)
  {
    double norm = sqrt(Vertices[i][0]*Vertices[i][0] + Vertices[i][1]*Vertices[i][1] + Vertices[i][2]*Vertices[i][2]);
    for (int k = 0; k < 3; k++)
      Vertices[i][k] = Vertices[i][k] / norm;
  }

  // Initialize the Faces
  Faces[0].v[0] = 0; Faces[0].v[1] = 1; Faces[0].v[2] = 2;
  Faces[1].v[0] = 0; Faces[1].v[1] = 2; Faces[1].v[2] = 3;
  Faces[2].v[0] = 0; Faces[2].v[1] = 3; Faces[2].v[2] = 4;
  Faces[3].v[0] = 0; Faces[3].v[1] = 4; Faces[3].v[2] = 5;
  Faces[4].v[0] = 0; Faces[4].v[1] = 5; Faces[4].v[2] = 1;
  Faces[5].v[0] = 1; Faces[5].v[1] = 2; Faces[5].v[2] = 6;
  Faces[6].v[0] = 2; Faces[6].v[1] = 3; Faces[6].v[2] = 7;
  Faces[7].v[0] = 3; Faces[7].v[1] = 4; Faces[7].v[2] = 8;
  Faces[8].v[0] = 4; Faces[8].v[1] = 5; Faces[8].v[2] = 9;
  Faces[9].v[0] = 5; Faces[9].v[1] = 1; Faces[9].v[2] = 10;
  Faces[10].v[0] = 6; Faces[10].v[1] = 7; Faces[10].v[2] = 2;
  Faces[11].v[0] = 7; Faces[11].v[1] = 8; Faces[11].v[2] = 3;
  Faces[12].v[0] = 8; Faces[12].v[1] = 9; Faces[12].v[2] = 4;
  Faces[13].v[0] = 9; Faces[13].v[1] = 10; Faces[13].v[2] = 5;
  Faces[14].v[0] = 10; Faces[14].v[1] = 6; Faces[14].v[2] = 1;
  Faces[15].v[0] = 6; Faces[15].v[1] = 7; Faces[15].v[2] = 11;
  Faces[16].v[0] = 7; Faces[16].v[1] = 8; Faces[16].v[2] = 11;
  Faces[17].v[0] = 8; Faces[17].v[1] = 9; Faces[17].v[2] = 11;
  Faces[18].v[0] = 9; Faces[18].v[1] = 10; Faces[18].v[2] = 11;
  Faces[19].v[0] = 10; Faces[19].v[1] = 6; Faces[19].v[2] = 11;


  // Initialize the neighbors
  for (int i = 0; i < N_F-1; i++)
  {
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
  }
  
}
Icosahedron::~Icosahedron(){
}
