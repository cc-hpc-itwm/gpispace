/***************************************************************************
                          migrationfilehandler.cpp  -  description
                             -------------------
    begin                : Mon Apr 3 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "migrationfilehandler.h"

MigrationFileHandler::MigrationFileHandler():index(1){
    endianess = TestByteOrder();
}
MigrationFileHandler::~MigrationFileHandler(){
}
void MigrationFileHandler::Write(const char* FName, const grid3D& G, const MVOL_TYPE M, const FILE_MODE MODE)
{
  std::ofstream MigFile(FName, std::ios::ate | std::ios::binary);
  if (MigFile.fail())
  {
    std::cerr << "ERROR in MigrationFileHandler::Write Could not open file " << FName << std::flush << std::endl;
    exit(1);
  }

  const int Nx(G.getNx());
  const int Ny(G.getNy());
  const int Nz(G.getNz());

  

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader SegHead = {};

      float* tmp = new float [Nz];
      SegHead.ns = (short) Nz;
      SegHead.dt = (short) (G.getdz() * 1e3);
      SegHead.trid = (short) 1;

      if (endianess != BIGENDIAN)
	{
	  swap_bytes(&SegHead.ns, 1, sizeof(short));
	  swap_bytes(&SegHead.dt, 1, sizeof(short));
	  swap_bytes(&SegHead.trid, 1, sizeof(short));
	}

      for (int iy = 0; iy < Ny; iy++)
      {
        for (int ix = 0; ix < Nx; ix++)
        {
          SegHead.tracl = index;
          SegHead.tracr = iy;
	  SegHead.cdp = ix;
          index++;

	  if (endianess != BIGENDIAN)
	    {
	      swap_bytes(&SegHead.tracl, 1, sizeof(int));
	      swap_bytes(&SegHead.tracr, 1, sizeof(int));
	      swap_bytes(&SegHead.cdp, 1, sizeof(int));
	    }

          for (int iz = 0; iz <Nz; iz++)
          {
            tmp[Nz-iz-1] = M[(ix*Ny+iy)*Nz+iz];
            //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
          }

	  float2ibm(&tmp[0], Nz, endianess);


          MigFile.write((char*) &SegHead, sizeof(SegYHeader));
          MigFile.write((char*) &tmp[0], Nz*sizeof(float));
        }
      }
      delete[] tmp;

     break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE not implemented yet!\n";
      break;
    }
  }
  MigFile.close();
  index = 1;
}

int MigrationFileHandler::TouchOffsetGather(const MigrationJob& Job, const char* FName, const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE, const float initval)
{
  FILE* MigFile = fopen(FName, "w");
  if (MigFile == NULL)
  {
      
      return -1;
  }

  

  unsigned long fileoffset = 0;

  Acq_geometry<float> Geom(Job.geom);

  switch(MODE)
  {
      case SEGY_BIGENDIAN:
      {
	  SegYEBCHeader EBCHeader = {};
	  fwrite((char*) EBCHeader.desc, sizeof(SegYEBCHeader), 1, MigFile);

	  SegYBHeader BHeader;
	  BHeader.hns = Job.MigVol.nz;
	  BHeader.hdt = (short) (Job.MigVol.dz * 1e3);
	  if (endianess != BIGENDIAN)
	    {
	      swap_bytes((void*)&BHeader.hns, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.hdt, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.format, 1, sizeof(short));
	    }
	  fwrite((char*) &BHeader, sizeof(SegYBHeader), 1, MigFile);

	  fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      default:
	  
	  break;
  };

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader Header = {};

      float* tmp = new float [Job.MigVol.nz];
      Header.ns = (short) Job.MigVol.nz;
      Header.dt = (short) (Job.MigVol.dz * 1e3);
      Header.trid = (short) 1;

      if (endianess != BIGENDIAN)
	{
	  swap_bytes(&Header.dt, 1, sizeof(short));
	  swap_bytes(&Header.ns, 1, sizeof(short));
	  swap_bytes(&Header.trid, 1, sizeof(short));
	}

      for (int iy = 0; iy < Job.MigVol.ny_inlines; iy++)
      {
        for (int ix = 0; ix < Job.MigVol.nx_xlines; ix++)
        {
	    Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
	    Point2d<float> P_MOD;
	    P_MOD.x[0] = ix * Job.MigVol.dx_between_xlines;
	    P_MOD.x[1] = iy * Job.MigVol.dy_between_inlines;
	    Point2d<float> P = P_WORLD_0 + Geom.MODxy_to_WORLDxy(P_MOD) - Geom.inlxl_to_WORLDxy(Job.geom.first_inline_num, Job.geom.first_xline_num);
	    int il, xl;
	    Geom.WORLDxy_to_inlxl(P.x[0], P.x[1], &il, &xl);
	    Header.sx = static_cast<int>(P.x[0]);
	    Header.sy = static_cast<int>(P.x[1]);
	    Header.gx = static_cast<int>(P.x[0]);
	    Header.gy = static_cast<int>(P.x[1]);
	    
	    if (endianess != BIGENDIAN)
	    {
		swap_bytes((void*)&Header.sx, 1, sizeof(int));
		swap_bytes((void*)&Header.sy, 1, sizeof(int));
		swap_bytes((void*)&Header.gx, 1, sizeof(int));
		swap_bytes((void*)&Header.gy, 1, sizeof(int));
	    }

	    for (int ioff = N0Off; ioff < N0Off + NOff; ioff++)
	    {
		unsigned long pos = ((iy * Job.MigVol.nx_xlines + ix) * NtotOff + ioff ) * (Job.MigVol.nz * sizeof(float) + sizeof(SegYHeader));
		fseek(MigFile, fileoffset + pos, SEEK_SET);

		Header.tracl = iy*Job.MigVol.nx_xlines + ix; //ix;
		Header.tracr = il; //ix*Job.MigVol.ny_inlines + iy;
		Header.cdp = xl;
		Header.offset = ioff;
		Header.delrt = static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz));

		index++;
		if (endianess != BIGENDIAN)
		  {
		    swap_bytes(&Header.tracl, 1, sizeof(int));
		    swap_bytes(&Header.tracr, 1, sizeof(int));
		    swap_bytes(&Header.cdp, 1, sizeof(int));
		    swap_bytes((void*)&Header.offset, 1, sizeof(int));
		    swap_bytes((void*)&Header.delrt, 1, sizeof(short));
		  }

		for (int iz = 0; iz <Job.MigVol.nz; iz++)
		{
		    tmp[Job.MigVol.nz-iz-1] = initval;
		    //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
		}

		float2ibm(&tmp[0], Job.MigVol.nz, endianess);


		fwrite((char*) &Header, sizeof(SegYHeader), 1, MigFile);
		fwrite((char*) &tmp[0], sizeof(float), Job.MigVol.nz, MigFile);
	    }
        }
      }
      delete[] tmp;

     break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE not implemented yet!\n";
      break;
    }
  }
  fclose(MigFile);
  index = 1;
  return 0;
}

int MigrationFileHandler::WriteOffsetClass(const char* FName, const grid3D& G, const MVOL_TYPE M, const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE)
{
  FILE* MigFile = fopen(FName, "rb+");
  if (MigFile == NULL)
  {
      
      return -1;
  }

  const int Nx(G.getNx());
  const int Ny(G.getNy());
  const int Nz(G.getNz());

  

  unsigned long fileoffset = 0;

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
      float* tmp = new float [Nz];

      for (int iy = 0; iy < Ny; iy++)
      {
        for (int ix = 0; ix < Nx; ix++)
        {
	    for (int ioff = N0Off; ioff < N0Off + NOff; ioff++)
	    {
		unsigned long pos = ((iy * Nx + ix) * NtotOff + ioff ) * (Nz * sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader);
		fseek(MigFile, fileoffset + pos, SEEK_SET);

		for (int iz = 0; iz <Nz; iz++)
		{
		    tmp[Nz-iz-1] = M[(ix*Ny+iy)*Nz+iz];
		    //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
		}

		float2ibm(&tmp[0], Nz, endianess);

		fwrite((char*) &tmp[0], sizeof(float), Nz, MigFile);
	    }
        }
      }
      delete[] tmp;

     break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE not implemented yet!\n";
      break;
    }
  }
  fclose(MigFile);
  index = 1;
  return 0;
}

int MigrationFileHandler::WriteOffsetClass(const char* FName, const grid3D& G, const MVOL_TYPE M,
					   const int N0x, const int Nx,
					   const int N0y, const int Ny,
					   const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE)
{
  FILE* MigFile = fopen(FName, "rb+");
  if (MigFile == NULL)
  {
      
      return -1;
  }

  const int Ntotx(G.getNx());
  const int Ntoty(G.getNy());
  const int Ntotz(G.getNz());

  
  unsigned long fileoffset = 0;

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
      float* tmp = new float [Ntotz];

      for (int iy = N0y; iy < N0y + Ny; iy++)
      {
        for (int ix = N0x; ix < N0x + Nx; ix++)
        {
	    for (int ioff = N0Off; ioff < N0Off + NOff; ioff++)
	    {
		unsigned long pos = ((iy * Ntotx + ix) * NtotOff + ioff ) * (Ntotz * sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader);
		fseek(MigFile, fileoffset + pos, SEEK_SET);

		for (int iz = 0; iz <Ntotz; iz++)
		{
		    tmp[Ntotz-iz-1] = M[(ix*Ntoty+iy)*Ntotz+iz];
		    //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
		}

		float2ibm(&tmp[0], Ntotz, endianess);

		fwrite((char*) &tmp[0], sizeof(float), Ntotz, MigFile);
	    }
        }
      }
      delete[] tmp;

     break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE not implemented yet!\n";
      break;
    }
  }
  fclose(MigFile);
  index = 1;
  return 0;
}

int MigrationFileHandler::WriteOffsetClassForSubVol(const char* FName, const grid3D& G, const MVOL_TYPE M,
					   const int N0x, const int Nx,
					   const int N0y, const int Ny,
					   const int N0Off, const int NOff, const int NtotOff, const FILE_MODE MODE)
{
  FILE* MigFile = fopen(FName, "rb+");
  if (MigFile == NULL)
  {
      
      return -1;
  }

  const int Ntotx(G.getNx());
  const int Ntoty(G.getNy());
  const int Ntotz(G.getNz());

  
  unsigned long fileoffset = 0;

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
      float* tmp = new float [Ntotz];

      for (int iy = N0y; iy < N0y + Ny; iy++)
      {
        for (int ix = N0x; ix < N0x + Nx; ix++)
        {
	    for (int ioff = N0Off; ioff < N0Off + NOff; ioff++)
	    {
		unsigned long pos = ((iy * Ntotx + ix) * NtotOff + ioff ) * (Ntotz * sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader);
		fseek(MigFile, fileoffset + pos, SEEK_SET);

		for (int iz = 0; iz <Ntotz; iz++)
		{
		    tmp[Ntotz-iz-1] = M[((ix-N0x)*Ntoty+(iy-N0y))*Ntotz+iz];
		    //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
		}

		float2ibm(&tmp[0], Ntotz, endianess);

		fwrite((char*) &tmp[0], sizeof(float), Ntotz, MigFile);
	    }
        }
      }
      delete[] tmp;

     break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE not implemented yet!\n";
      break;
    }
  }
  fclose(MigFile);
  index = 1;
  return 0;
}

void MigrationFileHandler::WritePart(const char* FName, const grid3D& G,
                                     point3D<int> N0, point3D<int> Ntot,
                                     const int N0Offx, const int NtotOffx,
                                     const MVOL_TYPE M, const FILE_MODE MODE)
{
  FILE* MigFile = fopen(FName, "rb+");
//   if (MigFile.fail())
//   {
//     std::cerr << "ERROR in MigrationFileHandler::WritePart Could not open file " << FName << std::flush << std::endl;
//     exit(1);
//   }

  const int Nx(G.getNx());
  const int Ny(G.getNy());
  const int Nz(G.getNz());

  


      SegYHeader SegHead = {};

      float* tmp = new float [Nz];
      SegHead.ns = (short) Nz;
      SegHead.dt = (short) (G.getdz() * 1e3);

      for (int iy = 0; iy < Ny; iy++)
      {
        for (int ix = 0; ix < Nx; ix++)
        {
          SegHead.tracl = index;
          SegHead.tracr = iy;//index;
	  SegHead.cdp = ix;

          SegHead.sx = static_cast<int>(G.getx0() + ix * G.getdx());
          SegHead.gx = static_cast<int>(G.getx0() + ix * G.getdx());

          SegHead.sy = static_cast<int>(G.gety0() + iy * G.getdy());
          SegHead.gy = static_cast<int>(G.gety0() + iy * G.getdy());

          index++;

          for (int iz = 0; iz <Nz; iz++)
          {
            tmp[Nz-iz-1] = M[(ix*Ny+iy)*Nz+iz];
            //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
          }

          //unsigned long pos = ((N0[0] + ix) * Ntot[1] + N0[1] + iy) * (Nz * sizeof(float) + sizeof(SegYHeader));
          unsigned long pos = (((N0[1] + iy) * Ntot[0] + N0[0] + ix) * NtotOffx + N0Offx) * (Nz * sizeof(float) + sizeof(SegYHeader));
          //std::cout << pos << std::endl;
          fseek(MigFile, pos, SEEK_SET);
          fwrite((char*) &SegHead, sizeof(SegYHeader), 1, MigFile);
          fwrite((char*) &tmp[0], sizeof(float), Nz, MigFile);

        }
      }
      delete[] tmp;


  fclose(MigFile);
  index = 1;
}



void MigrationFileHandler::WriteCutY(const char* FName, const int& iy, const grid3D& G, const MVOL_TYPE M, const FILE_MODE MODE)
{
  std::ofstream MigFile(FName, std::ios::ate | std::ios::binary);
  if (MigFile.fail())
    std::cerr << "Error in MigrationFileHandler::WriteCutY: Could not open file " << FName << std::endl;

  const int Nx(G.getNx());
  const int Ny(G.getNy());
  const int Nz(G.getNz());

  std::cout << "Writing CutY to File " << FName << " with Nx=" << Nx << ", Nz=" << Nz << std::endl;
  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader SegHead = {};
      float* tmp = new float [Nz];
      SegHead.ns = (short) Nz;
      SegHead.dt = (short) (G.getdz() * 1e3);
      SegHead.trid = (short) 1;
      swap_bytes(&SegHead.ns, 1, sizeof(short));
      swap_bytes(&SegHead.dt, 1, sizeof(short));
      swap_bytes(&SegHead.trid, 1, sizeof(short));

      for (int ix = 0; ix < Nx; ix++)
      {
        SegHead.tracl = index;
        SegHead.tracr = 1;
	SegHead.cdp = ix;
        index++;
        swap_bytes(&SegHead.tracl, 1, sizeof(int));
        swap_bytes(&SegHead.tracr, 1, sizeof(int));
        swap_bytes(&SegHead.cdp, 1, sizeof(int));

        for (int iz = 0; iz <Nz; iz++)
        {
          tmp[Nz-iz-1] = M[(ix*Ny+iy)*Nz+iz];
          //tmp[Nz-iz-1] = *M.GetPointAt(ix,iy,iz);
        }
        swap_bytes(&tmp[0], Nz, sizeof(float));

        MigFile.write((char*) &SegHead, sizeof(SegYHeader));
        MigFile.write((char*) &tmp[0], Nz*sizeof(float));
        if (MigFile.fail())
          std::cerr << "Error in MigrationFileHandler::WriteCutY: Could not write to file " << FName << std::endl;
      }
      delete[] tmp;
      break;
    }

    default:
    {
      std::cerr << "SEGFILE_MODE " << MODE << " not implemented yet!\n";
      break;
    }
  }

  MigFile.close();
  index = 1;
}
void MigrationFileHandler::WriteMoveOut(const char* FName, const int& Nz, const float& dz, float* M, const FILE_MODE MODE)
{
  std::ofstream MigFile(FName, std::ios::app | std::ios::binary);

  switch(MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader SegHead = {};
      float* tmp = new float [Nz];
      SegHead.ns = (short) Nz;
      SegHead.dt = (short) (dz * 1e3);
      SegHead.trid = (short) 1;
      swap_bytes(&SegHead.ns, 1, sizeof(short));
      swap_bytes(&SegHead.dt, 1, sizeof(short));
      swap_bytes(&SegHead.trid, 1, sizeof(short));

      SegHead.tracl = index;
      SegHead.tracr = index;
      index++;
      swap_bytes(&SegHead.tracl, 1, sizeof(int));
      swap_bytes(&SegHead.tracr, 1, sizeof(int));

      for (int iz = 0; iz <Nz; iz++)
      {
        tmp[iz] = M[iz];
      }
      swap_bytes(&tmp[0], Nz, sizeof(float));

      MigFile.write((char*) &SegHead, sizeof(SegYHeader));
      MigFile.write((char*) &tmp[0], Nz*sizeof(float));

      delete[] tmp;
      break;
    }
    default:
    {
      std::cerr << "SEGFILE_MODE " << MODE << " not implemented yet!\n";
      break;
    }
  }

  MigFile.close();
}

int MigrationFileHandler::WriteAngGather(float **Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume)
{
  std::ofstream Output(FileName, std::ios::binary);
  if (Output.fail())
  {
      
      return -1;
  }

  Acq_geometry<float> Geom(Job.geom);

  switch(Job.MigFileMode)
  {
      case SEGY_BIGENDIAN:
      {
	  SegYEBCHeader EBCHeader = {};
	  Output.write( (char*) EBCHeader.desc,  sizeof(SegYEBCHeader));

	  SegYBHeader BHeader;
	  BHeader.hns = BoxVolume.NBz*BoxVolume.nz;
	  BHeader.hdt = static_cast<unsigned short>(BoxVolume.dz * 1000);
	  if (endianess != BIGENDIAN)
	    {
	      swap_bytes((void*)&BHeader.hns, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.hdt, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.format, 1, sizeof(short));
	    }
	  Output.write( (char*) &BHeader,  sizeof(SegYBHeader));
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      default:
	  
	  break;
  };

  float* buffer = new float[BoxVolume.NBz*BoxVolume.nz];
  int count = 0;
  const unsigned long traclmax = BoxVolume.NBy*BoxVolume.ny * BoxVolume.NBx*BoxVolume.nx * Job.N_GATHER;

  int tracr = 0;
  for (int iby = 0; iby < BoxVolume.NBy; iby++)
    {
      for (int iy = 0; iy < BoxVolume.ny; iy++)
	{
	    tracr++;
	    int cdp = 0;
	    for (int ibx = 0; ibx < BoxVolume.NBx; ibx++)
	    {
		int boxindex = ibx * BoxVolume.NBy + iby;

		for (int ix = 0; ix < BoxVolume.nx; ix++)
		{
		    cdp++;
		    for (int ipo = 0; ipo < Job.N_GATHER; ipo++)
		    {
			SegYHeader Header = {};

			Header.ns =  BoxVolume.NBz*BoxVolume.nz;
			Header.dt = static_cast<unsigned short>(BoxVolume.dz * 1000);
			Header.trid = (short) 1;
			Header.tracl = traclmax;
			Header.tracr = tracr;
			Header.cdp = cdp;
			Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
			Point2d<float> P_MOD;
			P_MOD.x[0] = (ibx*BoxVolume.nx+ix) * Job.MigVol.dx_between_xlines;
			P_MOD.x[1] = (iby*BoxVolume.ny+iy) * Job.MigVol.dy_between_inlines;
			Point2d<float> P = P_WORLD_0 + Geom.MODxy_to_WORLDxy(P_MOD) - Geom.inlxl_to_WORLDxy(Job.geom.first_inline_num, Job.geom.first_xline_num);
//			Point2d<float> P = Geom.inlxl_to_WORLDxy((iby*BoxVolume.ny+iy), (ibx*BoxVolume.nx+ix));
			Header.sx = static_cast<int>(P.x[0]); //Job.X0[0] + (ibx*BoxVolume.nx+ix)*Job.dx[0];
			Header.sy = static_cast<int>(P.x[1]); //Job.X0[1] + (iby*BoxVolume.ny+iy)*Job.dx[1];
			Header.gx = static_cast<int>(P.x[0]); //Job.X0[0] + (ibx*BoxVolume.nx+ix)*Job.dx[0];
			Header.gy = static_cast<int>(P.x[1]); //Job.X0[1] + (iby*BoxVolume.ny+iy)*Job.dx[1];
			Header.offset = static_cast<short>(10 * (Job.phi_open_start.v + ipo * (Job.phi_open_end.v - Job.phi_open_start.v)/ (Job.N_GATHER - 1)));
			Header.delrt = static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz));
			count++;

			for (int ibz = 0; ibz < BoxVolume.NBz; ibz++)
			{
			    for (int iz = 0; iz < BoxVolume.nz; iz++)
			    {
				const int index = (((ibz * Job.N_GATHER + ipo) * BoxVolume.nx + ix) * BoxVolume.ny + iy) * BoxVolume.nz + iz;

				/// Have to swap z direction :-(
				buffer[BoxVolume.NBz*BoxVolume.nz - (ibz * BoxVolume.nz + iz) - 1] =  Result[boxindex][index];
			    }
			}

			switch(Job.MigFileMode)
			{
			    case SEGY_BIGENDIAN:
			    {
			      if (endianess != BIGENDIAN)
				{
				  swap_bytes((void*)&Header.tracl, 1, sizeof(int));
				  swap_bytes((void*)&Header.tracr, 1, sizeof(int));
				  swap_bytes((void*)&Header.cdp, 1, sizeof(int));
				  swap_bytes((void*)&Header.sx, 1, sizeof(int));
				  swap_bytes((void*)&Header.sy, 1, sizeof(int));
				  swap_bytes((void*)&Header.gx, 1, sizeof(int));
				  swap_bytes((void*)&Header.gy, 1, sizeof(int));
				  swap_bytes((void*)&Header.offset, 1, sizeof(int));
				  swap_bytes((void*)&Header.ns, 1, sizeof(unsigned short));
				  swap_bytes((void*)&Header.dt, 1, sizeof(unsigned short));
				  swap_bytes((void*)&Header.trid, 1, sizeof(short));
				  swap_bytes((void*)&Header.delrt, 1, sizeof(short));
				}
			      float2ibm(buffer, BoxVolume.NBz*BoxVolume.nz, endianess);
			      break;
			    }
			    case SU_LITENDIAN:
			    {
			      if (endianess != LITENDIAN)
				{
				  swap_bytes((void*)&Header.tracl, 1, sizeof(int));
				  swap_bytes((void*)&Header.tracr, 1, sizeof(int));
				  swap_bytes((void*)&Header.cdp, 1, sizeof(int));
				  swap_bytes((void*)&Header.sx, 1, sizeof(int));
				  swap_bytes((void*)&Header.sy, 1, sizeof(int));
				  swap_bytes((void*)&Header.gx, 1, sizeof(int));
				  swap_bytes((void*)&Header.gy, 1, sizeof(int));
				  swap_bytes((void*)&Header.offset, 1, sizeof(int));
				  swap_bytes((void*)&Header.ns, 1, sizeof(unsigned short));
				  swap_bytes((void*)&Header.dt, 1, sizeof(unsigned short));
				  swap_bytes((void*)&Header.trid, 1, sizeof(short));
				  swap_bytes((void*)&Header.delrt, 1, sizeof(short));

				  swap_bytes(buffer, BoxVolume.NBz*BoxVolume.nz, sizeof(float));
				}
				break;
			    }
			    default:
				
				break;
			};

			Output.write( (char*) &Header,  sizeof(SegYHeader));
			Output.write( (char*) buffer, BoxVolume.NBz*BoxVolume.nz*sizeof(float));
		    }
		}
	    }
	}
    }
  delete[] buffer;
  if (Output.fail())
  {
      
      return -1;
  }

 Output.close();
 if (Output.fail())
 {
      
      return -1;
 }
 return 0;
}

int MigrationFileHandler::TouchAngGather(const AngleMigrationJob& Job, const char* FileName, const float initval)
{
  int ierr = 0;

  SeismicFileWriter Writer;
  std::ofstream HeaderOutput;
  std::ofstream BinaryOutput;
  if (Job.MigFileMode == SVF)
  {
      HeaderOutput.open(Job.Header1FileName, std::ios::binary);
      BinaryOutput.open(FileName, std::ios::binary);
  }

  Acq_geometry<float> Geom(Job.geom);

  switch(Job.MigFileMode)
  {
      case SVF:  // ScreenSeis SVF Format
      {
	  // Write SVF FileProperties
	  std::ofstream PropOutput(Job.Header2FileName);
	  // general settings
	  PropOutput << "AbortError = 0\n";
	  PropOutput << "ByteOrder = 1234\n";
	  PropOutput << "CoordType = 0\n";
	  PropOutput << "DataDimensions = 4\n";
	  PropOutput << "DataType = 4\n";
	  PropOutput << "FrameType = Depth\n";
	  PropOutput << "HeaderCompression = none\n";
	  PropOutput << "HeaderLength = 0\n";
	  PropOutput << "HeaderType = none\n";
	  PropOutput << "IO_Type = raw\n";
	  PropOutput << "Label1= Depth\n";
	  PropOutput << "Label2 = Angle\n";
	  PropOutput << "Label3 = X-Line\n";
	  PropOutput << "Label4 = Inline\n";
	  PropOutput << "Regular = 1\n";
	  PropOutput << "SeisVersion = 01/01/2005\n";
	  PropOutput << "TraceCompression = none\n";
	  PropOutput << "VfMapped = 0\n";

	  Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
	  PropOutput << "DataDeltas = \""
		     << Job.MigVol.dz << ","
		     << (Job.phi_open_end.v - Job.phi_open_start.v)/ Job.N_PHI_OPEN << ","
		     << Job.MigVol.dx_between_xlines << ","
		     << Job.MigVol.dy_between_inlines << "\"\n";
	  PropOutput << "DataOrigins = \""
		     << (-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz)) << ","
		     << Job.phi_open_start.v << ","
		     << Job.MigVol.first_xline_num << ","
		     << Job.MigVol.first_inline_num << "\"\n";
	  PropOutput << "DataSizes = \"" << Job.MigVol.nz << "," << Job.N_GATHER << "," << Job.MigVol.nx_xlines << "," << Job.MigVol.ny_inlines << "\"\n";
	  PropOutput << "LogicalDeltas = \"1,1,1,1\"\n";
	  PropOutput << "LogicalOrigins = \""
		     << static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz)) << ","
		     << Job.phi_open_start.v << ","
		     << P_WORLD_0.x[0] << ","
		     << P_WORLD_0.x[1] << "\"\n";
	  PropOutput << "SampleRate = " << static_cast<unsigned short>(Job.MigVol.dz * 1000) << "\n";
	  PropOutput << "SamplesPerTrace = " << Job.MigVol.nz << "\n";          
	  PropOutput << "TracesPerFrame = " << Job.N_GATHER << "\n";    
	  PropOutput << "FramesPerVolume = " << Job.MigVol.nx_xlines << "\n";            
	  PropOutput << "Volumes = " << Job.MigVol.ny_inlines << "\n";         

	  PropOutput.close();
	  break;
      }
      default:
      {
	  ierr = Writer.Open(FileName, Job.MigFileMode, Job.MigVol.nz, (unsigned short) (Job.MigVol.dz * 1e3));
	  if ( ierr != 0 )
	  {
	      
	      return -1;
	  }
	  break;
      }
  };

  float* buffer = new float[Job.MigVol.nz];

  const int extra_azimuth_classes = Job.N_GATHER / Job.N_PHI_OPEN - Job.N_AZIMUTH_CLASSES;
  int tracl = -1;
  int ep = -1;
  for (int iy = 0; iy < Job.MigVol.ny_inlines; iy++)
  {
      for (int ix = 0; ix < Job.MigVol.nx_xlines; ix++)
      {
	  ep++;
	  int cdpt = -1;
	  for (int igather = 0; igather < Job.N_GATHER; igather++)
	  {
	      tracl++;
	      cdpt++;
	      const int ipo = igather / ( Job.N_AZIMUTH_CLASSES + extra_azimuth_classes);
	      const int iazimuth = igather % ( Job.N_AZIMUTH_CLASSES + extra_azimuth_classes);

	      SegYHeader Header = {};
	      Header.trid = (short) 1;

	      Header.ns =  Job.MigVol.nz;
	      Header.dt = static_cast<unsigned short>(Job.MigVol.dz * 1000);

	      Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
	      Point2d<float> P_MOD;
	      P_MOD.x[0] = ix * Job.MigVol.dx_between_xlines;
	      P_MOD.x[1] = iy * Job.MigVol.dy_between_inlines;
	      Point2d<float> P = P_WORLD_0 + Geom.MODxy_to_WORLDxy(P_MOD) - Geom.inlxl_to_WORLDxy(Job.geom.first_inline_num, Job.geom.first_xline_num);
	      int il, xl;
	      Geom.WORLDxy_to_inlxl(P.x[0], P.x[1], &il, &xl);
	      Header.tracl = tracl;
	      Header.tracr = il;
	      Header.cdp = xl;
	      Header.cdpt = cdpt;
	      Header.ep = ep;
	      Header.sx = static_cast<int>(P.x[0]);
	      Header.sy = static_cast<int>(P.x[1]);
	      Header.gx = static_cast<int>(P.x[0]);
	      Header.gy = static_cast<int>(P.x[1]);
	      Header.offset = static_cast<int>(10 * (Job.phi_open_start.v + ipo * (Job.phi_open_end.v - Job.phi_open_start.v)/ (Job.N_PHI_OPEN - 1)));
	      Header.delrt = static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz));
	      Header.selev = (iazimuth < Job.N_AZIMUTH_CLASSES)?static_cast<int>( 10 * iazimuth * Job.dazimuth_deg.v ):9999;
	      Header.gelev = iazimuth;

	      for (int iz = 0; iz < Job.MigVol.nz; iz++)
	      {
		  /// Have to swap z direction :-(
		  buffer[Job.MigVol.nz - iz - 1] =  initval;
	      }

	      switch(Job.MigFileMode)
	      {
		  case SVF: // ScreenSeis SVF Format
		  {
		    if (endianess != LITENDIAN)
		      {
			swap_bytes((void*)&Header.tracl, 1, sizeof(int));
			swap_bytes((void*)&Header.tracr, 1, sizeof(int));
			swap_bytes((void*)&Header.cdp, 1, sizeof(int));
			swap_bytes((void*)&Header.cdpt, 1, sizeof(int));
			swap_bytes((void*)&Header.ep, 1, sizeof(int));
			swap_bytes((void*)&Header.sx, 1, sizeof(int));
			swap_bytes((void*)&Header.sy, 1, sizeof(int));
			swap_bytes((void*)&Header.gx, 1, sizeof(int));
			swap_bytes((void*)&Header.gy, 1, sizeof(int));
			swap_bytes((void*)&Header.offset, 1, sizeof(int));
			swap_bytes((void*)&Header.selev, 1, sizeof(int));
			swap_bytes((void*)&Header.gelev, 1, sizeof(int));
			swap_bytes((void*)&Header.ns, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.dt, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.trid, 1, sizeof(short));
			swap_bytes((void*)&Header.delrt, 1, sizeof(short));

			swap_bytes(buffer, Job.MigVol.nz, sizeof(int));

		      }
		    HeaderOutput.write( (char*) &Header,  sizeof(SegYHeader));
		    BinaryOutput.write( (char*) buffer, Job.MigVol.nz*sizeof(float));		  
		    break;
		  }
		  default:
		      ierr = Writer.Write( &Header, buffer);
		      break;
	      };
	  }
      }
  }
  delete[] buffer;
  if ( ierr != 0 )
  {
      
      return -1;
  }

   ierr = Writer.Close();
   if (ierr != 0)
   {
       
       return -1;
   }

   if (Job.MigFileMode == SVF)
   {
       HeaderOutput.close();
       BinaryOutput.close();
   }
   return 0;
}


int MigrationFileHandler::TouchStack(const AngleMigrationJob& Job, const char* FileName, const float initval)
{
  std::ofstream Output(FileName, std::ios::binary);
  if (Output.fail())
  {
      
      return -1;
  }

  std::ofstream HeaderOutput;
  if (Job.MigFileMode == SVF)
      HeaderOutput.open(Job.Header1FileName, std::ios::binary);

  Acq_geometry<float> Geom(Job.geom);

  switch(Job.MigFileMode)
  {
      case SEGY_BIGENDIAN:
      {
	  SegYEBCHeader EBCHeader = {};
	  Output.write( (char*) EBCHeader.desc,  sizeof(SegYEBCHeader));

	  SegYBHeader BHeader;
	  BHeader.hns = Job.MigVol.nz;
	  BHeader.hdt = (short) (Job.MigVol.dz * 1e3);
	  if (endianess != BIGENDIAN)
	    {
	      swap_bytes((void*)&BHeader.hns, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.hdt, 1, sizeof(short));
	      swap_bytes((void*)&BHeader.format, 1, sizeof(short));
	    }
	  Output.write( (char*) &BHeader,  sizeof(SegYBHeader));
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      case SVF:  // ScreenSeis SVF Format
      {
	  // Write SVF FileProperties
	  std::ofstream PropOutput(Job.Header2FileName);
	  // general settings
	  PropOutput << "AbortError = 0\n";
	  PropOutput << "ByteOrder = 1234\n";
	  PropOutput << "CoordType = 0\n";
	  PropOutput << "DataDimensions = 4\n";
	  PropOutput << "DataType = 4\n";
	  PropOutput << "FrameType = Depth\n";
	  PropOutput << "HeaderCompression = none\n";
	  PropOutput << "HeaderLength = 0\n";
	  PropOutput << "HeaderType = none\n";
	  PropOutput << "IO_Type = raw\n";
	  PropOutput << "Label1= Depth\n";
	  PropOutput << "Label2 = Angle\n";
	  PropOutput << "Label3 = X-Line\n";
	  PropOutput << "Label4 = Inline\n";
	  PropOutput << "Regular = 1\n";
	  PropOutput << "SeisVersion = 01/01/2005\n";
	  PropOutput << "TraceCompression = none\n";
	  PropOutput << "VfMapped = 0\n";

	  Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
	  PropOutput << "DataDeltas = \""
		     << Job.MigVol.dz << ","
		     << "1" << ","
		     << Job.MigVol.dx_between_xlines << ","
		     << Job.MigVol.dy_between_inlines << "\"\n";
	  PropOutput << "DataOrigins = \""
		     << (-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz)) << ","
		     << Job.phi_open_start.v << ","
		     << Job.MigVol.first_xline_num << ","
		     << Job.MigVol.first_inline_num << "\"\n";
	  PropOutput << "DataSizes = \"" << Job.MigVol.nz << "," << "1" << "," << Job.MigVol.nx_xlines << "," << Job.MigVol.ny_inlines << "\"\n";
	  PropOutput << "LogicalDeltas = \"1,1,1,1\"\n";
	  PropOutput << "LogicalOrigins = \""
		     << static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz)) << ","
		     << Job.phi_open_start.v << ","
		     << P_WORLD_0.x[0] << ","
		     << P_WORLD_0.x[1] << "\"\n";
	  PropOutput << "SampleRate = " << static_cast<unsigned short>(Job.MigVol.dz * 1000) << "\n";
	  PropOutput << "SamplesPerTrace = " << Job.MigVol.nz << "\n";          
	  PropOutput << "TracesPerFrame = " << "1" << "\n";    
	  PropOutput << "FramesPerVolume = " << Job.MigVol.nx_xlines << "\n";            
	  PropOutput << "Volumes = " << Job.MigVol.ny_inlines << "\n";         


	  PropOutput.close();
	  break;
      }

      default:
	  
	  break;
  };

  if (Output.fail())
  {
      
      return -1;
  }

  float* buffer = new float[Job.MigVol.nz];

  int tracl = -1;
  for (int iy = 0; iy < Job.MigVol.ny_inlines; iy++)
  {
      for (int ix = 0; ix < Job.MigVol.nx_xlines; ix++)
      {
	  int cdpt = -1;
	  //for (int ipo = 0; ipo < Job.N_GATHER; ipo++)
	  {
	      tracl++;
	      cdpt++;

	      SegYHeader Header = {};
	      Header.trid = (short) 1;

	      Header.ns =  Job.MigVol.nz;
	      Header.dt = static_cast<unsigned short>(Job.MigVol.dz * 1000);

	      Point2d<float> P_WORLD_0 = Geom.inlxl_to_WORLDxy(Job.MigVol.first_inline_num, Job.MigVol.first_xline_num);
	      Point2d<float> P_MOD;
	      P_MOD.x[0] = ix * Job.MigVol.dx_between_xlines;
	      P_MOD.x[1] = iy * Job.MigVol.dy_between_inlines;
	      Point2d<float> P = P_WORLD_0 + Geom.MODxy_to_WORLDxy(P_MOD) - Geom.inlxl_to_WORLDxy(Job.geom.first_inline_num, Job.geom.first_xline_num);
	      int il, xl;
	      Geom.WORLDxy_to_inlxl(P.x[0], P.x[1], &il, &xl);
	      Header.tracl = tracl;
	      Header.tracr = il;
	      Header.cdp = xl;
	      Header.cdpt = cdpt;
	      Header.sx = static_cast<int>(P.x[0]);
	      Header.sy = static_cast<int>(P.x[1]);
	      Header.gx = static_cast<int>(P.x[0]);
	      Header.gy = static_cast<int>(P.x[1]);
	      Header.offset = 0;
	      Header.delrt = static_cast<short>(-(Job.MigVol.first_z_coord + (Job.MigVol.nz-1) * Job.MigVol.dz));


	      for (int iz = 0; iz < Job.MigVol.nz; iz++)
	      {
		  /// Have to swap z direction :-(
		  buffer[Job.MigVol.nz - iz - 1] =  initval;
	      }

	      switch(Job.MigFileMode)
	      {
		  case SEGY_BIGENDIAN:
		  {
		    if (endianess != BIGENDIAN)
		      {
			swap_bytes((void*)&Header.tracl, 1, sizeof(int));
			swap_bytes((void*)&Header.tracr, 1, sizeof(int));
			swap_bytes((void*)&Header.cdp, 1, sizeof(int));
			swap_bytes((void*)&Header.cdpt, 1, sizeof(int));
			swap_bytes((void*)&Header.sx, 1, sizeof(int));
			swap_bytes((void*)&Header.sy, 1, sizeof(int));
			swap_bytes((void*)&Header.gx, 1, sizeof(int));
			swap_bytes((void*)&Header.gy, 1, sizeof(int));
			swap_bytes((void*)&Header.offset, 1, sizeof(int));
			swap_bytes((void*)&Header.ns, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.dt, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.trid, 1, sizeof(short));
			swap_bytes((void*)&Header.delrt, 1, sizeof(short));
		      }

		      float2ibm(buffer, Job.MigVol.nz, endianess);
		      break;
		  }
		  case SU_LITENDIAN:
		  case SVF:
		  {
		    if (endianess != LITENDIAN)
		      {
			swap_bytes((void*)&Header.tracl, 1, sizeof(int));
			swap_bytes((void*)&Header.tracr, 1, sizeof(int));
			swap_bytes((void*)&Header.cdp, 1, sizeof(int));
			swap_bytes((void*)&Header.cdpt, 1, sizeof(int));
			swap_bytes((void*)&Header.sx, 1, sizeof(int));
			swap_bytes((void*)&Header.sy, 1, sizeof(int));
			swap_bytes((void*)&Header.gx, 1, sizeof(int));
			swap_bytes((void*)&Header.gy, 1, sizeof(int));
			swap_bytes((void*)&Header.offset, 1, sizeof(int));
			swap_bytes((void*)&Header.ns, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.dt, 1, sizeof(unsigned short));
			swap_bytes((void*)&Header.trid, 1, sizeof(short));
			swap_bytes((void*)&Header.delrt, 1, sizeof(short));

			swap_bytes(buffer, Job.MigVol.nz, sizeof(float));
		      }

		      break;
		  }
		  default:
		      
		      break;
	      };

	      if (Job.MigFileMode == SVF)  // ScreenSeis SVF Format
	      {
		  HeaderOutput.write( (char*) &Header,  sizeof(SegYHeader));
		  Output.write( (char*) buffer, Job.MigVol.nz*sizeof(float));		  
	      }
	      else
	      {
		  Output.write( (char*) &Header,  sizeof(SegYHeader));
		  Output.write( (char*) buffer, Job.MigVol.nz*sizeof(float));
	      }
	  }
      }
  }
  delete[] buffer;
  if (Output.fail())
  {
      
      return -1;
  }

   Output.close();
   if (Output.fail())
   {
       
       return -1;
   }

   if (Job.MigFileMode == SVF)
       HeaderOutput.close();
   return 0;
}


int MigrationFileHandler::WriteAngGather(float *Result, const char* FileName, const FILE_MODE FileMode,
					 const int Nxtotal, const int Nytotal, const int Nztotal, const int Nopentotal,
					 const int ixbeg, const int iybeg, const int izbeg, const int iopenbeg,
					 const int Nx, const int Ny, const int Nz, const int Nopen, const bool flip)
{
    int ierr = 0;
    FILE* MigFile = fopen(FileName, "rb+");
    if (MigFile == NULL)
    {
	
	return -1;
    }

    unsigned long fileoffset = 0;
    unsigned long headeroffset = 0;
    switch(FileMode)
    {
	case SEGY_BIGENDIAN:
	{
	    fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
	    headeroffset = sizeof(SegYHeader);
	    break;
	}
	case SU_LITENDIAN:
	{
	    headeroffset = sizeof(SegYHeader);
	    break;
	}
	case SVF:
	{
	    break;
	}
	default:
	    
	    break;
    };


    float* buffer = new float[Nz];

    const unsigned long tracelength = headeroffset + Nztotal * sizeof(float);

    for (int iy = 0; iy < Ny; iy++)
    {
	for (int ix = 0; ix < Nx; ix++)
	{
	    for (int iopen = 0; iopen < Nopen; iopen++)
	    {

		// copy z stencil to buffer
		if (flip)
		{
		    for (int iz = 0; iz < Nz; iz++)
		    {
			int Result_index = ((iy * Nx + ix) * Nopen + iopen) * Nztotal + izbeg + iz;
			buffer[Nz - iz - 1] =  Result[Result_index];
		    }
		}
		else
		{
		    for (int iz = 0; iz < Nz; iz++)
		    {
			int Result_index = ((iy * Nx + ix) * Nopen + iopen) * Nztotal + izbeg + iz;
			buffer[iz] =  Result[Result_index];
		    }
		}

		// convert data to output file mode
		switch(FileMode)
		{
		    case SEGY_BIGENDIAN:
		    {
			float2ibm(buffer, Nz, endianess);
			break;
		    }
		    case SU_LITENDIAN:
		    case SVF:
		    {
			break;
		    }
		    default:
			
			break;
		};

		const unsigned long offsettraces = ((iybeg + iy) * Nxtotal + ixbeg + ix ) * Nopentotal + iopen;
		unsigned long filepos =  fileoffset + offsettraces * tracelength + headeroffset;
		if (flip)
		    filepos += (Nztotal - ( izbeg + Nz - 1) - 1) * sizeof(float);
		else
		    filepos += izbeg * sizeof(float);

		fseek(MigFile, filepos, SEEK_SET);
		fwrite( (char*) buffer, sizeof(float), Nz, MigFile);

	    }
	}
    }

    fclose(MigFile);
    delete[] buffer;
    return ierr;
}

int MigrationFileHandler::WriteWindAngGather(float *Result, const char* FileName, const FILE_MODE FileMode,
					     const int Nxtotal, const int Nytotal, const int Nztotal, const int Nopentotal,
					     const int ixbeg, const int iybeg, const int izbeg, const int iopenbeg,
					     const int Nx, const int Ny, const int Nz, const int Nopen,
					     const int ixwindbeg, const int iywindbeg, const int izwindbeg, const int iopenwindbeg,
					     const int Nxwind, const int Nywind, const int Nzwind, const int Nopenwind,
					     const bool flip)
{
    int ierr = 0;
    FILE* MigFile = fopen(FileName, "rb+");
    if (MigFile == NULL)
    {
	
	return -1;
    }

    unsigned long fileoffset = 0;
    unsigned long headeroffset = 0;
    switch(FileMode)
    {
	case SEGY_BIGENDIAN:
	{
	    fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
	    headeroffset = sizeof(SegYHeader);
	    break;
	}
	case SU_LITENDIAN:
	{
	    headeroffset = sizeof(SegYHeader);
	    break;
	}
	case SVF:
	{
	    break;
	}
	default:
	    
	    break;
    };


    float* buffer = new float[Nzwind];

    const int tracelength = headeroffset + Nztotal * sizeof(float);

    for (int iy = 0; iy < Nywind; iy++)
    {
	for (int ix = 0; ix < Nxwind; ix++)
	{
	    for (int iopen = 0; iopen < Nopenwind; iopen++)
	    {

		// copy z stencil to buffer
		if (flip)
		{
		    for (int iz = 0; iz < Nzwind; iz++)
		    {
			int Result_index = (((iy + iywindbeg) * Nx + (ix + ixwindbeg)) * Nopen + (iopen + iopenwindbeg)) * Nztotal + izbeg + iz + izwindbeg;
			buffer[Nzwind - iz - 1] =  Result[Result_index];
		    }
		}
		else
		{
		    for (int iz = 0; iz < Nzwind; iz++)
		    {
			int Result_index = (((iy + iywindbeg) * Nx + (ix + ixwindbeg)) * Nopen + (iopen + iopenwindbeg)) * Nztotal + izbeg + iz + izwindbeg;
			buffer[iz] =  Result[Result_index];
		    }
		}

		// convert data to output file mode
		switch(FileMode)
		{
		    case SEGY_BIGENDIAN:
		    {
			float2ibm(buffer, Nzwind, endianess);
			break;
		    }
		    case SU_LITENDIAN:
		    case SVF:
		    {
			break;
		    }
		    default:
			
			break;
		};

		const unsigned int offsettraces = ((iybeg + iy) * Nxtotal + ixbeg + ix ) * Nopentotal + iopen;
		unsigned long filepos =  fileoffset + offsettraces * tracelength + headeroffset;
		if (flip)
		    filepos += (Nztotal - ( izbeg + Nzwind - 1) - 1) * sizeof(float);
		else
		    filepos += izbeg * sizeof(float);

		fseek(MigFile, filepos, SEEK_SET);
		fwrite( (char*) buffer, sizeof(float), Nzwind, MigFile);

	    }
	}
    }

    fclose(MigFile);
    delete[] buffer;
    return ierr;
}


int MigrationFileHandler::WriteAngGather(float *Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume, const int ibx, const int iby)
{
    return WriteAngGather(Result, Job, FileName, BoxVolume, ibx, iby, 0, BoxVolume.NBz);
}


int MigrationFileHandler::WriteAngGather(float *Result, const AngleMigrationJob& Job, const char* FileName, BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz_beg, const int ibz_end)
{
  FILE* MigFile = fopen(FileName, "rb+");
  if (MigFile == NULL)
  {
      
      return -1;
  }

  unsigned long fileoffset = 0;
  unsigned long headeroffset = 0;
  switch(Job.MigFileMode)
  {
      case SEGY_BIGENDIAN:
      {
	  fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
	  headeroffset = sizeof(SegYHeader);
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      case SVF:
      {
	  break;
      }
      default:
	  
	  break;
  };

  float* buffer = new float[(ibz_end - ibz_beg)*BoxVolume.nz];
  const Box *SBox_beg = BoxVolume.getBox(ibx, iby, ibz_beg);
  const int ixoffset = SBox_beg->ixp;
  const int iyoffset = SBox_beg->iyp;
  const int izoffset = SBox_beg->izp;

  for (int iy = 0; iy < SBox_beg->Ny; iy++)
  {
      for (int ix = 0; ix < SBox_beg->Nx; ix++)
      {
	  for (int ipo = 0; ipo < Job.N_GATHER; ipo++)
	  {
	      int Nztotal = 0;
	      for (int ibz = ibz_beg; ibz < ibz_end; ibz++)
	      {
		  const Box *SBox = BoxVolume.getBox(ibx, iby, ibz);
		  for (int iz = 0; iz < SBox->Nz; iz++)
		  {
		      const int index = (((ibz * Job.N_GATHER + ipo) * SBox_beg->Nx + ix) * SBox_beg->Ny + iy) * SBox->Nz + iz;
		      //const int index = (((ibz * Job.N_GATHER + ipo) * BoxVolume.nx * BoxVolume.ny * BoxVolume.nz))  + ix * SBox_beg->Ny + iy) * BoxVolume.nz + iz;

            /// Have to swap z direction :-(
		      buffer[(ibz_end - ibz_beg)*BoxVolume.nz - Nztotal - 1] =  Result[index];
		      Nztotal++;
		  }
	      }

 	      switch(Job.MigFileMode)
 	      {
 		  case SEGY_BIGENDIAN:
 		  {
 		      float2ibm(buffer, (ibz_end - ibz_beg)*BoxVolume.nz, endianess);
 		      break;
 		  }
 		  case SU_LITENDIAN:
 		  case SVF:
 		  {
 		      break;
 		  }
 		  default:
 		      
 		      break;
 	      };
		  
	      const unsigned long filepos =  (( (iyoffset + iy) * Job.MigVol.nx_xlines + ixoffset + ix ) * Job.N_GATHER + ipo) 
		  * ( Job.MigVol.nz * sizeof(float) + headeroffset) + headeroffset +   (Job.MigVol.nz - izoffset - Nztotal)*sizeof(float);

	      fseek(MigFile, fileoffset + filepos, SEEK_SET);

	      fwrite( (char*) &(buffer[(ibz_end - ibz_beg)*BoxVolume.nz - Nztotal]), sizeof(float), Nztotal, MigFile);
	  }
      }
  }
  delete[] buffer;
  fclose(MigFile);
  return 0;
}

int MigrationFileHandler::TouchRestartAngGather(const char* FName, const BlockVolume& BoxVolume)
{
    std::ofstream Output(FName, std::ios::binary);
    if (Output.fail())
    {
	
	return -1;
    }

    int* buffer = new int[BoxVolume.NBz];
    for (int ibz = 0; ibz < BoxVolume.NBz; ibz++)
    {
	buffer[ibz] =  -1;
    }

    for (int iby = 0; iby < BoxVolume.NBy; iby++)
    {
	for (int ibx = 0; ibx < BoxVolume.NBx; ibx++)
	{
	    Output.write( (char*) buffer, BoxVolume.NBz*sizeof(int));
	}
    }
    delete[] buffer;

    if (Output.fail())
    {
	
	return -1;
    }

    Output.close();
    if (Output.fail())
    {
	
	return -1;
    }
   return 0;
}

unsigned long MigrationFileHandler::InitialCheckRestartAngGather(const char* FName, ERROR_TYPE& err)
{
    err = OK_GE;
    unsigned long FirstEntryToBeCalculated = 0;

    FILE* RestartFile = fopen(FName, "rb");
    if ( RestartFile == NULL)
    {
	
	err = WARNING_GE;
	return FirstEntryToBeCalculated;
    }

    if ( fseek(RestartFile, 0, SEEK_END) != 0 )
      err = FATAL_ERROR_GE;
    const long endpos = ftell(RestartFile);
    rewind(RestartFile);
    if ( endpos < 0)
      err = FATAL_ERROR_GE;
    
    if ( err != OK_GE)
    {
	
	return FirstEntryToBeCalculated;
    }

    const unsigned long TotNofEntries = endpos / sizeof(int);

    int* Values = new int[TotNofEntries];
    if (Values == NULL)
      {
	
	err = FATAL_ERROR_GE;
	return FirstEntryToBeCalculated;
      }

    if ( fread( (char*) Values, sizeof(int), TotNofEntries, RestartFile) != TotNofEntries)
      {
	
	err = FATAL_ERROR_GE;
	return FirstEntryToBeCalculated;
      }


    for (; FirstEntryToBeCalculated < TotNofEntries; ++FirstEntryToBeCalculated)
      {
	if (Values[FirstEntryToBeCalculated] != 0)
	  break;
      }

    if ( fclose(RestartFile) != 0)
      {
	
	err = FATAL_ERROR_GE;
	return FirstEntryToBeCalculated;
      }

    if ( FirstEntryToBeCalculated == TotNofEntries)
      {
	
	
	
	err = WARNING_GE;
	return FirstEntryToBeCalculated;
      }

    return FirstEntryToBeCalculated;
}

bool MigrationFileHandler::CheckRestartAngGather(const char* FName, const BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz)
{
    FILE* RestartFile = fopen(FName, "rb");
    if ( RestartFile == NULL)
    {
	
	return false;
    }

    const unsigned long filepos =  (( iby  * BoxVolume.NBx + ibx ) * BoxVolume.NBz + ibz ) * sizeof(int);
    fseek(RestartFile, filepos, SEEK_SET);

    int Value;
    fread( (char*) &Value, sizeof(int), 1, RestartFile);

    bool ret = true;
    if ( Value != -1 )
	 ret = false;

    fclose(RestartFile);
    return ret;
}

int MigrationFileHandler::SetRestartAngGather(const char* FileName, const BlockVolume& BoxVolume, const int ibx, const int iby, const int ibz_beg, const int ibz_end)
{
    FILE* RestartFile = fopen(FileName, "rb+");
    if ( RestartFile == NULL)
    {
	
	return false;
    }

    for (int ibz = ibz_beg; ibz < ibz_end; ibz++)
    {
	const unsigned long filepos =  (( iby  * BoxVolume.NBx + ibx ) * BoxVolume.NBz + ibz ) * sizeof(int);
	fseek(RestartFile, filepos, SEEK_SET);

	int Value = 0;
	fwrite( (char*) &Value, sizeof(int), 1, RestartFile);
    }

    fclose(RestartFile);
    return 0;
}

bool MigrationFileHandler::CheckAngGather(const AngleMigrationJob& Job, const BlockVolume& BoxVolume, const int ibx, const int iby)
{
  FILE* MigFile = fopen(Job.MigFileName, "rb");
  if ( MigFile == NULL)
  {
      
      return false;
  }

  unsigned long fileoffset = 0;
  switch(Job.MigFileMode)
  {
      case SEGY_BIGENDIAN:
      {
	  fileoffset =  sizeof(SegYEBCHeader) + sizeof(SegYBHeader);
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      default:
	  
	  break;
  };

  const unsigned long traclmax = BoxVolume.NBy*BoxVolume.ny * BoxVolume.NBx*BoxVolume.nx * Job.N_GATHER;

  const unsigned long filepos =  (( iby * BoxVolume.ny * BoxVolume.NBx*BoxVolume.nx + ibx * BoxVolume.nx ) * Job.N_GATHER )
      * ( BoxVolume.NBz*BoxVolume.nz * sizeof(float) + sizeof(SegYHeader));
  fseek(MigFile, fileoffset + filepos, SEEK_SET);

  SegYHeader Header;
  fread( (char*) &Header, sizeof(SegYHeader), 1, MigFile);

  switch(Job.MigFileMode)
  {
      case SEGY_BIGENDIAN:
      {
          swap_bytes(&Header.tracl, 1, sizeof(int));
	  break;
      }
      case SU_LITENDIAN:
      {
	  break;
      }
      default:
	  
	  break;
  };

  bool ret = true;
  if ( Header.tracl != traclmax)
      ret = false;

  fclose(MigFile);
  return ret;
}
