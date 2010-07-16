/***************************************************************************
                          outputclass.cpp  -  description
                             -------------------
    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "outputclass.h"

OutputClass::OutputClass(){
    PreFileName = NULL;
}
OutputClass::~OutputClass(){
    if ( PreFileName != NULL )
	delete[] PreFileName;
}
/** No descriptions */
void OutputClass::PrintRcv(const ReceiverGrid& RcvGrid, const point3D<float>& X0, const point3D<float>& X1)
{
  ReceiverGrid::iterator iter = RcvGrid.begin(X0, X1);

  for (; !iter.end(); ++iter)
  {
    Receiver* Rcv = (*iter);
    RecSig* pSig = Rcv->pSignal;
    std::cout << "Signals recorded with receiver at " << Rcv->pos << ": \n";
    while (pSig != &Rcv->Signals[g_MAXSIG-1])
    {
      pSig++;
      std::cout << "  t = " << pSig->GetT() << "\n";
//       std::cout << "  t = " << pSig->arrtime << ", A = " << pSig->Amp << "\n";
    }
  }
}

// void OutputClass::WriteRcv(const ReceiverGrid& RcvGrid, const int& SrcX, const int& SrcY)
// {
//   char* filename = new char[199];
//   sprintf(filename, "LZ_%d_%d.dat", SrcX, SrcY);
//   std::cout << "Writing run-times to file " << filename << std::endl;
  
//   std::ofstream outfile(filename, std::ios::binary);
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not open file " << filename << std::endl;

//   int Nx = RcvGrid.getNx();
//   int Ny = RcvGrid.getNy();
//   int Nz = RcvGrid.getNz();

//   outfile.write(reinterpret_cast<char*>(&Nx), sizeof(int));
//   outfile.write(reinterpret_cast<char*>(&Ny), sizeof(int));
//   outfile.write(reinterpret_cast<char*>(&Nz), sizeof(int));
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not write to file " << filename << std::endl;

//   for (int i = 0; i < Nx; i++)
//     for (int j = 0; j < Ny; j++)
//       for (int k = 0; k < Nz; k++)
//       {
//         Receiver* Rcv = RcvGrid.GetPointAt(i, j, k);
//         RecSig* pSig = Rcv->GetSig();
//         outfile.write(reinterpret_cast<char*>(&(pSig->GetT())), sizeof(float));
//       }
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not write to file " << filename << std::endl;

//   outfile.close();
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not close file " << filename << std::endl;
// }


void OutputClass::WriteAngSig(const AngSig* Signals, const int& n, const int& idir)
{
  char* filename = new char[199];
  sprintf(filename, "ANG_%d.dat", idir);
  std::cout << "Writing angle signals to file " << filename << std::endl;
  
  std::ofstream outfile(filename, std::ios::binary);
  if (outfile.fail())
    std::cerr << "Error in OutputClass::WriteRcv:  Could not open file " << filename << std::endl;

  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].arrtime), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].x), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].y), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].xdir), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].ydir), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].v_inv), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].detQ), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].kmah), sizeof(int));
  outfile.close();
}

void OutputClass::WriteAngSig(const AngSig* Signals, const int& n, const int& SrcX, const int& SrcY, const int& SrcZ)
{
  char* filename = new char[199];
  sprintf(filename, "ANG_%d_%d_%d.dat", SrcX, SrcY, SrcZ);
  std::cout << "Writing angle signals to file " << filename << std::endl;
  
  std::ofstream outfile(filename, std::ios::binary);
  if (outfile.fail())
    std::cerr << "Error in OutputClass::WriteRcv:  Could not open file " << filename << std::endl;

  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].arrtime), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].x), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].y), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].xdir), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].ydir), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].v_inv), sizeof(float));
  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].detQ), sizeof(float));
  for (int i = 0; i < n; i++)
    {
      float kmah = Signals[i].kmah;
      outfile.write(reinterpret_cast<const char*>(&kmah), sizeof(int));
    }

  for (int i = 0; i < n; i++)
    outfile.write(reinterpret_cast<const char*>(&Signals[i].z), sizeof(float));
  outfile.close();
}

void OutputClass::WriteAngSig(const AngSig* Signals, const int& n, const int& SrcX, const int& SrcY, const int& SrcZ,
			      const int& NSrcY)
{
  char* filename = new char[199];
  sprintf(filename, "ANG_%d.dat", SrcZ);
  std::cout << "Writing angle signals to file " << filename << std::endl;
  
  FILE* outfile = fopen(filename, "rb+");
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not open file " << filename << std::endl;
  const unsigned long filepos = (SrcX * NSrcY + SrcY) * n * 9 * sizeof(float);
  fseek(outfile, filepos, SEEK_SET);

  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].arrtime), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].x), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].y), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].xdir), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].ydir), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].v_inv), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].detQ), sizeof(float), 1, outfile);
  for (int i = 0; i < n; i++)
    {
      float kmah = Signals[i].kmah;
      fwrite(reinterpret_cast<const char*>(&kmah), sizeof(int), 1, outfile);
    }

  for (int i = 0; i < n; i++)
    fwrite(reinterpret_cast<const char*>(&Signals[i].z), sizeof(float), 1, outfile);
  
  fclose(outfile);
}

void OutputClass::TouchAngSig(const int& SrcX, const int& SrcY, const int& SrcZ,
			      const int& NSrcY)
{
  char* filename = new char[199];
  sprintf(filename, "ANG_%d.dat", SrcZ);
  std::cout << "Writing angle signals to file " << filename << std::endl;
  
  FILE* outfile = fopen(filename, "wb");
//   if (outfile.fail())
//     std::cerr << "Error in OutputClass::WriteRcv:  Could not open file " << filename << std::endl;
  const float dummy = 0;
  fwrite(reinterpret_cast<const char*>(&dummy), sizeof(float), 1, outfile);
  fclose(outfile);
}

// void OutputClass::WriteRcv(const ReceiverGrid& RcvGrid, const char* FName)
// {
//   std::ofstream outfile(FName, std::ios::app | std::ios::binary);

//   float NoSig = -1;

//   int Nx = RcvGrid.getNx();
//   int Ny = RcvGrid.getNy();
//   int Nz = RcvGrid.getNz();

//   outfile.write(reinterpret_cast<char*>(&Nx), sizeof(int));
//   outfile.write(reinterpret_cast<char*>(&Ny), sizeof(int));
//   outfile.write(reinterpret_cast<char*>(&Nz), sizeof(int));

//   for (int i = 0; i < Nx; i++)
//     for (int j = 0; j < Ny; j++)
//       for (int k = 0; k < Nz; k++)
//       {
//         Receiver* Rcv = RcvGrid.GetPointAt(i, j, k);
//         RecSig* pSig = Rcv->pSignal;
//         if ( pSig == &Rcv->Signals[g_MAXSIG-1]) // no signal recorded
//           outfile.write(reinterpret_cast<char*>(&NoSig), sizeof(float));
//         else // write last signal
//         {
//           pSig++;
//           outfile.write(reinterpret_cast<char*>(&pSig->GetT()), sizeof(float));
//         }
//       }

//   outfile.close();
// }
void OutputClass::WriteRcvSegY(const ReceiverGrid& RcvGrid, const char* FName)
{
  std::ofstream outfile(FName, std::ios::app | std::ios::binary);

  float NoSig = -1;

  int Nx = RcvGrid.getNx();
  int Ny = RcvGrid.getNy();
  int Nz = RcvGrid.getNz();

  point3D<float> X0 = RcvGrid.getX0();
  point3D<float> dx = RcvGrid.getdx();

  float* buffer = new float[Nz];
  SegYHeader Header = {};
  Header.ns = Nz;
  Header.dt = (int) dx[2];
  int index = 0;
  for (int i = 0; i < Nx; i++)
    for (int j = 0; j < Ny; j++)
      {
        Header.tracl = index;
        Header.tracr = index;
        index++;
        Header.sx = i+1;
        Header.sy = j+1;

        Header.gx = static_cast<int>(X0[0] + i * dx[0]);
        Header.gy = static_cast<int>(X0[1] + j * dx[1]);

        outfile.write(reinterpret_cast<char*>(&Header), sizeof(SegYHeader));
        for (int k = 0; k < Nz; k++)
          {
            Receiver* Rcv = RcvGrid.GetPointAt(i, j, k);
            RecSig* pSig = Rcv->pSignal;
            if ( pSig == &Rcv->Signals[g_MAXSIG-1]) // no signal recorded
              buffer[Nz-k-1] = NoSig;
            else // write last signal
              {
                pSig++;
                buffer[Nz-k-1] = pSig->GetT();
              }

          }
        outfile.write(reinterpret_cast<char*>(buffer), Nz*sizeof(float));
      }

  delete[] buffer;
  outfile.close();
}
void OutputClass::WriteDim(const grid3D& SrcGrid, const ReceiverGrid& RcvGrid)
{
  char* filename = new char[199];
  sprintf(filename, "LZ.txt");

  std::ofstream outfile(filename, std::ios::ate);

  outfile << "# Dimensioning of the Surface grid of Sources/Receiver\n";
  outfile << "SrcGridX0 " << SrcGrid.getx0() << std::endl;
  outfile << "SrcGridY0 " << SrcGrid.gety0() << std::endl;
  outfile << "SrcGridZ0 " << SrcGrid.getz0() << std::endl;
  outfile << std::endl;
  outfile << "SrcGriddx " << SrcGrid.getNx() << std::endl;
  outfile << "SrcGriddy " << SrcGrid.getNx() << std::endl;
  outfile << "SrcGriddy " << SrcGrid.getNx() << std::endl;
  outfile << std::endl;
  outfile << "SrcGridNx " << SrcGrid.getNx() << std::endl;
  outfile << "SrcGridNy " << SrcGrid.getNy() << std::endl;
  outfile << "SrcGridNz " << SrcGrid.getNz() << std::endl;

  outfile << "\n\n# Dimensioning of the Volume grid\n";

  const point3D<float> VelGridX0(RcvGrid.getX0());
  outfile << "VolGridX0 " << VelGridX0[0] << std::endl;
  outfile << "VolGridY0 " << VelGridX0[1] << std::endl;
  outfile << "VolGridZ0 " << VelGridX0[2] << std::endl;
  outfile << std::endl;
  const point3D<float> VelGriddx(RcvGrid.getdx());
  outfile << "VolGriddx " << VelGriddx[0] << std::endl;
  outfile << "VolGriddy " << VelGriddx[1] << std::endl;
  outfile << "VolGriddz " << VelGriddx[2] << std::endl;
  outfile << std::endl;
  outfile << "VolGridNx " << RcvGrid.getNx() << std::endl;
  outfile << "VolGridNy " << RcvGrid.getNy() << std::endl;
  outfile << "VolGridNz " << RcvGrid.getNz() << std::endl;

  outfile.close();
}

void OutputClass::PrintVelocityField(PropModel<VelGridPointBSpline>& VF)
{
#define DIMENSION 100

  int Nx = DIMENSION;
  int Ny = DIMENSION;
  int Nz = DIMENSION;

  double x0, y0, z0;
  double dx, dy, dz;
  x0 = -250; y0 = -250, z0 = -250;
  dx = 500.0 / (Nx-1); dy = 500.0 / (Ny-1), dz = 250.0 / (Nz-1);

  for (int i = 0; i < DIMENSION-1; i+=3)
  {
    for (int j = 0; j < DIMENSION-1; j+=3)
    {
      for (int k = 0; k < DIMENSION-1; k+=3)
       {
        float x = x0 + i*dx;
        float y = y0 + j*dy;
        float z = z0 + k*dz;

        point3D<float> xp(x, y, z);
        point3D<float> vgrad;
        float v = VF.GetProperty(xp, vgrad);
        std::cout << x << " " << y << " " << z << " " << v << " " << vgrad << std::endl;
        }
      std::cout << std::endl << std::endl;
    }
    std::cout << std::endl << std::endl;
  }
}

void OutputClass::PrintTriangulation(triMem& TList, rayMem& rArray, int n, int Tstep)
{
  std::cout << "Writing Triangulation ...";

  char* filename = new char[199];
  if (Tstep == -1)
    sprintf(filename, "Triang.dat");
  else
    sprintf(filename, "Triang_%d.dat", Tstep);

  std::ofstream outfile(filename, std::ios::ate);

  //TList.InitRun();
  triMem::iterator iter_end(TList.end());
  for (triMem::iterator iter(TList.begin()); iter != iter_end; ++iter)
  {
    //if (_tri == 3441)
    {
      Triangle* Tri = &(*iter);
      if (Tri->lifesign != Triangle::DELETED)
      {
        ray3D* p0 = &(Tri->points[0]->mainray);
        ray3D* p1 = &(Tri->points[1]->mainray);
        ray3D* p2 = &(Tri->points[2]->mainray);
        outfile << "  " << p0->x[0] << "  " << p0->x[1] << "  " << p0->x[2]
                << "  " << p0->StartDir.phi << "  " << p0->StartDir.theta << std::endl;
        outfile << "  " << p1->x[0] << "  " << p1->x[1] << "  " << p1->x[2]
                << "  " << p1->StartDir.phi << "  " << p1->StartDir.theta << std::endl;
        outfile << "  " << p2->x[0] << "  " << p2->x[1] << "  " << p2->x[2]
                << "  " << p2->StartDir.phi << "  " << p2->StartDir.theta << std::endl;
        outfile << "  " << p0->x[0] << "  " << p0->x[1] << "  " << p0->x[2]
                << "  " << p0->StartDir.phi << "  " << p0->StartDir.theta << std::endl;

        outfile << "\n\n";
      }
    }
  }


//  for (int i = 0; i < n; i++)
//  {
//    ray3D* p0, *p1, *p2;
//    p0 = TList[i].points[0];
//    p1 = TList[i].points[1];
//    p2 = TList[i].points[2];
//    outfile << "  " << p0->x[0] << "  " << p0->x[1] << "  " << p0->x[2] << endl;
//    outfile << "  " << p1->x[0] << "  " << p1->x[1] << "  " << p1->x[2] << endl;
//    outfile << "  " << p2->x[0] << "  " << p2->x[1] << "  " << p2->x[2] << endl;
//    outfile << "  " << p0->x[0] << "  " << p0->x[1] << "  " << p0->x[2] << endl;
//
//    outfile << endl << endl;;
//  }
  outfile.close();

  std::cout << " done\n";

}

void OutputClass::PrintRay(ray3D* ray)
{
  std::cout << "Ray : " << ray->x << " " << ray->p << "\n";
}

void OutputClass::PrintJob(const TracingJob& Job)
{
  std::cout << "Migration Job has been initialized with:\n\n";
  std::cout << "Volume          : " << Job.X0Vol << " --- " << Job.X1Vol << std::endl;
  std::cout << "Velocity Grid   : " << Job.X0Vel << " --- " << Job.NVel << " Step " << Job.dxVel << std::endl;
  std::cout << "Source Grid     : " << Job.X0Src << " --- " << Job.NSrc << " Step " << Job.dxSrc << std::endl;
  std::cout << "Receiver Grid   : " << Job.X0Rcv << " --- " << Job.NRcv << " Step " << Job.dxRcv << std::endl;
  std::cout << std::endl;
  std::cout << "Max t step           : " << Job.g_MAXTSTEP << std::endl;
  std::cout << "t step size          : " << Job.g_TSTEPSIZE << std::endl;
  std::cout << "Initial angle discr  : " << Job.g_InitAngle.v << std::endl;
  std::cout << "Max triangle length  : " << Job.g_REF_LEN << std::endl;

  // Corner coordinates of the Volume
  std::cout << "X0Volx " << Job.X0Vol[0] << std::endl;
  std::cout << "X0Voly " << Job.X0Vol[1] << std::endl;
  std::cout << "X0Volz " << Job.X0Vol[2] << std::endl;

  std::cout << "X1Volx " << Job.X1Vol[0] << std::endl;
  std::cout << "X1Voly " << Job.X1Vol[1] << std::endl;
  std::cout << "X1Volz " << Job.X1Vol[2] << std::endl;

// Corners and resolution of the Velocity Model
  std::cout << "X0Velx " << Job.X0Vel[0] << std::endl;
  std::cout << "X0Vely " << Job.X0Vel[1] << std::endl;
  std::cout << "X0Velz " << Job.X0Vel[2] << std::endl;

  std::cout << "NxVel " << Job.NVel[0] << std::endl;
  std::cout << "NyVel " << Job.NVel[1] << std::endl;
  std::cout << "NzVel " << Job.NVel[2] << std::endl;

  std::cout << "dxVel " << Job.dxVel[0] << std::endl;
  std::cout << "dyVel " << Job.dxVel[1] << std::endl;
  std::cout << "dzVel " << Job.dxVel[2] << std::endl;

// Corners and distances of the grid of Sources
  std::cout << "X0Srcx " << Job.X0Src[0] << std::endl;
  std::cout << "X0Srcy " << Job.X0Src[1] << std::endl;
  std::cout << "X0Srcz " << Job.X0Src[2] << std::endl;

  std::cout << "NxSrc " << Job.NSrc[0] << std::endl;
  std::cout << "NySrc " << Job.NSrc[1] << std::endl;
  std::cout << "NzSrc " << Job.NSrc[2] << std::endl;

  std::cout << "dxSrc " << Job.dxSrc[0] << std::endl;
  std::cout << "dySrc " << Job.dxSrc[1] << std::endl;
  std::cout << "dzSrc " << Job.dxSrc[2] << std::endl;

// Corners and distances of the grid of Receivers
  std::cout << "X0Rcvx " << Job.X0Rcv[0] << std::endl;
  std::cout << "X0Rcvy " << Job.X0Rcv[1] << std::endl;
  std::cout << "X0Rcvz " << Job.X0Rcv[2] << std::endl;

  std::cout << "NxRcv " << Job.NRcv[0] << std::endl;
  std::cout << "NyRcv " << Job.NRcv[1] << std::endl;
  std::cout << "NzRcv " << Job.NRcv[2] << std::endl;

  std::cout << "dxRcv " << Job.dxRcv[0] << std::endl;
  std::cout << "dyRcv " << Job.dxRcv[1] << std::endl;
  std::cout << "dzRcv " << Job.dxRcv[2] << std::endl;

// Initial ray distribution per source and max lenght for refinement
  std::cout << "RAY_MIN " << Job.g_InitAngle.v << std::endl;
  std::cout << "REF_LEN " << Job.g_REF_LEN << std::endl;
  
// Time step size
  std::cout << "MAXTSTEP " << Job.g_MAXTSTEP << std::endl;
  std::cout << "TSTEPSIZE " << Job.g_TSTEPSIZE << std::endl;
}

// int OutputClass::SetDirectory(const char* DirName, const char* PreFix)
// {
//     int ierr = 0;
//     if (DirName != NULL)
//     {
// 	int len = strlen(DirName);
// 	if (PreFix != NULL)
// 	    len += strlen(PreFix);

// 	if ( len > 198 )
// 	{
// 	    
// 	    ierr = 1;
// 	}
// 	else
// 	{
// 	    if (PreFileName != NULL)
// 		delete PreFileName;
// 	    PreFileName = new char[len+3];
// 	    if ((PreFix != NULL) && (strlen(PreFix) > 0))
// 		sprintf(PreFileName, "%s/%s_", DirName, PreFix);
// 	    else
// 		sprintf(PreFileName, "%s/", DirName);
// 	}
	
//     }
//     return ierr;
// }
