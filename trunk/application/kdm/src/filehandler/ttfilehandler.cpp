/***************************************************************************
                          ttfilehandler.cpp  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "ttfilehandler.h"

TTFileHandler::TTFileHandler():Src00(NULL),Src01(NULL),Src10(NULL),Src11(NULL),Rcv00(NULL),Rcv01(NULL),Rcv10(NULL),Rcv11(NULL)
{
  endianess = TestByteOrder();
  PreFileName = NULL;
  Nx = 0;
  Ny = 0;
  Nz = 0;
  ixS = -100;
  iyS = -100;
  ixR = -100;
  iyR = -100;
}
TTFileHandler::TTFileHandler(const int _Nx, const int _Ny, const int _Nz):Src00(NULL),Src01(NULL),Src10(NULL),Src11(NULL),Rcv00(NULL),Rcv01(NULL),Rcv10(NULL),Rcv11(NULL)
{
  endianess = TestByteOrder();
  PreFileName = NULL;
  Nx = _Nx;
  Ny = _Ny;
  Nz = _Nz;
  ixS = -100;
  iyS = -100;
  ixR = -100;
  iyR = -100;
}
void TTFileHandler::InitVol(const int _Nx, const int _Ny, const int _Nz)
{
  Nx = _Nx;
  Ny = _Ny;
  Nz = _Nz;
};
TTFileHandler::~TTFileHandler()
{
    if ( PreFileName != NULL )
	delete[] PreFileName;
    clear();
}
void TTFileHandler::clear()
{    
  if (Src00 != NULL)
  {
#if (defined(__ALTIVEC__) || defined(__SPU__))
    _free_align(Src00);
    _free_align(Src10);
    _free_align(Src01);
    _free_align(Src11);
    _free_align(Rcv00);
    _free_align(Rcv10);
    _free_align(Rcv01);
    _free_align(Rcv11);
#else
    delete[] Src00;
    delete[] Src10;
    delete[] Src01;
    delete[] Src11;
    delete[] Rcv00;
    delete[] Rcv10;
    delete[] Rcv01;
    delete[] Rcv11;
#endif

    Src00=NULL;
    Src01=NULL;
    Src10=NULL;
    Src11=NULL;
    Rcv00=NULL;
    Rcv10=NULL;
    Rcv01=NULL;
    Rcv11=NULL;
  }  
}
void TTFileHandler::TTFileHandleralloc(const int Nx, const int Ny, const int Nz)
{
  if (Src00 == NULL)
  {
#ifdef DEBUG
    std::cout << "TTFileHandleralloc is allocating memory\n";
#endif

#if (defined(__ALTIVEC__) || defined(__SPU__))
    Src00 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Src01 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Src10 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Src11 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Rcv00 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Rcv01 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Rcv10 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
    Rcv11 = (float*) _malloc_align(Nx*Ny*Nz*N_SIGNALS*sizeof(float), 7);
#else
    Src00 = new float[Nx*Ny*Nz*N_SIGNALS];
    Src01 = new float[Nx*Ny*Nz*N_SIGNALS];
    Src10 = new float[Nx*Ny*Nz*N_SIGNALS];
    Src11 = new float[Nx*Ny*Nz*N_SIGNALS];
    Rcv00 = new float[Nx*Ny*Nz*N_SIGNALS];
    Rcv01 = new float[Nx*Ny*Nz*N_SIGNALS];
    Rcv10 = new float[Nx*Ny*Nz*N_SIGNALS];
    Rcv11 = new float[Nx*Ny*Nz*N_SIGNALS];
#endif

  }
}

bool TTFileHandler::ReadRT(const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header)
{
  char FileName[199];
  sprintf(FileName,"%s_%d_%d.dat", NameBase, ix, iy);

#ifdef DEBUG
  std::cout << "TTFileHandler::ReadRT: Read " << FileName << std::endl;
#endif
  std::ifstream Input(FileName, std::ios::binary);

  if (Input.fail())
    {
      std::cerr << "Error in TTFileHandler::ReadRT: Failed to open file " << FileName << std::endl;
      return false;
    }

  //std::cout << "ReadRT. " << Nx << " x " << Ny << " x " << Nz << " --> " << Nx*Ny*(sizeof(SegYHeader) + Nz*sizeof(float)) << " Byte\n";

  char* buffertmp = new char[Nx*Ny*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float))];
  Input.read((char*) buffertmp, Nx*Ny*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float)));
  memcpy((void*) &Header, buffertmp, sizeof(SegYHeader));
  
  for (int i = 0; i < Nx; i++)
    for (int j = 0; j < Ny; j++)
    {
      memcpy((void*) &Header, (void*) &(buffertmp[(i*Ny+j)*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float))]),  sizeof(SegYHeader));
      memcpy((void*) (&(buffer[(i*Ny+j)*Nz*N_SIGNALS])), (void*) &(buffertmp[(i*Ny+j)*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float)) + sizeof(SegYHeader)]), Nz*N_SIGNALS*sizeof(float));

//       if (endianess != LITENDIAN)
// 	{
// 	  swap_bytes((void*)&Header.sx, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.sy, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gx, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gy, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.scalco, 1, sizeof(short));
// 	  swap_bytes((void*)&Header.selev, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gelev, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.scalel, 1, sizeof(short));

// 	  swap_bytes((void*)&(buffer[(i*Ny+j)*Nz*N_SIGNALS]), Nz*N_SIGNALS, sizeof(float));
// 	}

//       Input.read((char*) (buffer_tmp), Nz*sizeof(float));
//       for (int iz = 0; iz < Nz; iz++)
//         buffer[i][j][iz] = buffer_tmp[Nz-iz-1];
    }

//   for (int i = 0; i < Nx; i++)
//     for (int j = 0; j < Ny; j++)
//     {
//       Input.read((char*) &Header, sizeof(SegYHeader));
//       Input.read((char*) (&(buffer[(i*Ny+j)*Nz*N_SIGNALS])), Nz*N_SIGNALS*sizeof(float));

//       if (endianess != LITENDIAN)
// 	{
// 	  swap_bytes((void*)&Header.sx, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.sy, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gx, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gy, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.scalco, 1, sizeof(short));
// 	  swap_bytes((void*)&Header.selev, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.gelev, 1, sizeof(int));
// 	  swap_bytes((void*)&Header.scalel, 1, sizeof(short));

// 	  //swap_bytes((void*)&(buffer[(i*Ny+j)*Nz*N_SIGNALS]), Nz*N_SIGNALS, sizeof(float));
// 	}

// //       Input.read((char*) (buffer_tmp), Nz*sizeof(float));
// //       for (int iz = 0; iz < Nz; iz++)
// //         buffer[i][j][iz] = buffer_tmp[Nz-iz-1];
//     }

  Input.close();  
  delete[] buffertmp;
  return true;
};


bool TTFileHandler::ReadRT(const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header,
			   const int N0x, const int Npartx, const int N0y, const int Nparty)
{
  char FileName[199];
  sprintf(FileName,"%s_%d_%d.dat", NameBase, ix, iy);

#ifdef DEBUG
  std::cout << "TTFileHandler::ReadRT: Read " << FileName << std::endl;
#endif
  std::ifstream Input(FileName, std::ios::binary);

  if (Input.fail())
    {
      std::cerr << "Error in TTFileHandler::ReadRT: Failed to open file " << FileName << std::endl;
      return false;
    }

//  std::cout << "ReadRT. " << Nx << " x " << Ny << " x " << Nz << " --> " << Nx*Ny*(sizeof(SegYHeader) + Nz*sizeof(float)) << " Byte\n";

  for (int i = N0x; i < N0x + Npartx; i++)
    {
      Input.seekg((i*Ny+N0y)*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float)), std::ios::beg);
      for (int j = N0y; j < N0y + Nparty; j++)
	{
	  Input.read((char*) &Header, sizeof(SegYHeader));
	  Input.read((char*) (&(buffer[(i*Ny+j)*Nz*N_SIGNALS])), Nz*N_SIGNALS*sizeof(float));
	  
/*        if (endianess != LITENDIAN)
  	{
  	  swap_bytes((void*)&Header.sx, 1, sizeof(int));
  	  swap_bytes((void*)&Header.sy, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gx, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gy, 1, sizeof(int));
  	  swap_bytes((void*)&Header.scalco, 1, sizeof(short));
  	  swap_bytes((void*)&Header.selev, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gelev, 1, sizeof(int));
  	  swap_bytes((void*)&Header.scalel, 1, sizeof(short));
 
  	  swap_bytes((void*)&(buffer[(i*Ny+j)*Nz*N_SIGNALS]), Nz*N_SIGNALS, sizeof(float));
  	}

        float sx_MOD, sy_MOD, gx_MOD, gy_MOD;
	Geom.WORLDxy_to_MODxy(Header.sx, Header.sy,
			      &sx_MOD, &sy_MOD);
	Geom.WORLDxy_to_MODxy(Header.gx, Header.gy,
			      &gx_MOD, &gy_MOD);

// 	std::cout << "sx: " << Header->sx << " --> " << sx_MOD << "\n";
// 	std::cout << "sy: " << Header->sy << " --> " << sy_MOD << "\n";
// 	std::cout << "gx: " << Header->gx << " --> " << gx_MOD << "\n";
// 	std::cout << "gy: " << Header->gy << " --> " << gy_MOD << "\n";
	Header.sx = sx_MOD;
	Header.sy = sy_MOD;
	Header.gx = gx_MOD;
	Header.gy = gy_MOD;        */
 
	}
    }

  Input.close();  
  return true;
};

bool TTFileHandler::ReadRT(const MigrationJob& MigJob, const char* NameBase, const int& ix, const int& iy, float* buffer, SegYHeader& Header,
			   const int N0x, const int Npartx, const int N0y, const int Nparty)
{
  char FileName[199];
  Acq_geometry<float> Geom(MigJob.geom);
  sprintf(FileName,"%s_%d_%d.dat", NameBase, ix, iy);

#ifdef DEBUG
  std::cout << "TTFileHandler::ReadRT: Read " << FileName << std::endl;
#endif
  std::ifstream Input(FileName, std::ios::binary);

  if (Input.fail())
    {
      std::cerr << "Error in TTFileHandler::ReadRT: Failed to open file " << FileName << std::endl;
      return false;
    }

  //std::cout << "ReadRT. " << Nx << " x " << Ny << " x " << Nz << " --> " << Nx*Ny*(sizeof(SegYHeader) + Nz*sizeof(float)) << " Byte\n";

  for (int i = N0x; i < N0x + Npartx; i++)
    {
      Input.seekg((i*Ny+N0y)*(sizeof(SegYHeader) + Nz*N_SIGNALS*sizeof(float)), std::ios::beg);
      for (int j = N0y; j < N0y + Nparty; j++)
	{
	  Input.read((char*) &Header, sizeof(SegYHeader));
	  Input.read((char*) (&(buffer[(i*Ny+j)*Nz*N_SIGNALS])), Nz*N_SIGNALS*sizeof(float));
	  
        if (endianess != LITENDIAN)
  	{
  	  swap_bytes((void*)&Header.sx, 1, sizeof(int));
  	  swap_bytes((void*)&Header.sy, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gx, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gy, 1, sizeof(int));
  	  swap_bytes((void*)&Header.scalco, 1, sizeof(short));
  	  swap_bytes((void*)&Header.selev, 1, sizeof(int));
  	  swap_bytes((void*)&Header.gelev, 1, sizeof(int));
  	  swap_bytes((void*)&Header.scalel, 1, sizeof(short));
 
  	  swap_bytes((void*)&(buffer[(i*Ny+j)*Nz*N_SIGNALS]), Nz*N_SIGNALS, sizeof(float));
  	}

        float sx_MOD, sy_MOD, gx_MOD, gy_MOD;
	Geom.WORLDxy_to_MODxy(Header.sx, Header.sy,
			      &sx_MOD, &sy_MOD);
	Geom.WORLDxy_to_MODxy(Header.gx, Header.gy,
			      &gx_MOD, &gy_MOD);

// 	std::cout << "sx: " << Header->sx << " --> " << sx_MOD << "\n";
// 	std::cout << "sy: " << Header->sy << " --> " << sy_MOD << "\n";
// 	std::cout << "gx: " << Header->gx << " --> " << gx_MOD << "\n";
// 	std::cout << "gy: " << Header->gy << " --> " << gy_MOD << "\n";
        /*
	Header.sx = sx_MOD;
	Header.sy = sy_MOD;
	Header.gx = gx_MOD;
	Header.gy = gy_MOD; 
        */
        Header.sx = static_cast<int>(roundf(sx_MOD));
	Header.sy = static_cast<int>(roundf(sy_MOD));
	Header.gx = static_cast<int>(roundf(gx_MOD));
	Header.gy = static_cast<int>(roundf(gy_MOD));       
 
	}
    }

  Input.close();  
  return true;
};

bool TTFileHandler::Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
                         float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
                         float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
			 SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
			 SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11 )
{
  TTFileHandleralloc(Nx, Ny, Nz);

//   std::cout << "TTFileHandler::Init : " << ixS << " " << iyS << " " << ixR << " " << iyR << std::endl;
//   std::cout << "TTFileHandler::Init : " << _ixS << " " << _iyS << " " << _ixR << " " << _iyR;
  if ( (ixS != _ixS) || (iyS != _iyS))
  {
    if (( ixS == _ixS ) && ( iyS == _iyS-1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS00 = HeaderS01;

        retS00 = retS01;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS10 = HeaderS11;

        retS10 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11);
      }
    else if (( ixS == _ixS ) && ( iyS == _iyS+1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS01 = HeaderS00;

        retS01 = retS00;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS10;

        retS11 = retS10;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00);
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10);
      }
    else if (( ixS == _ixS-1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS00 = HeaderS10;

        retS00 =retS10;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS01 = HeaderS11;

        retS01 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11);
      }
    else if (( ixS == _ixS+1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS10 = HeaderS00;

        retS10 = retS00;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS01;

        retS11 = retS01;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00);
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01);
      }
    else
      {
// 	std::cout << " read 4 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00);
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10);
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11);
      }
    ixS = _ixS;
    iyS = _iyS;
  }

  if ( (ixR != _ixR) || (iyR != _iyR))
  {
    if (( ixR == _ixR ) && ( iyR == _iyR-1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR00 = HeaderR01;

        retR00 =retR01;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR10 = HeaderR11;

        retR10 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11);
      }
    else if (( ixR == _ixR ) && ( iyR == _iyR+1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR01 = HeaderR00;

        retR01 = retR00;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR10;

        retR11 = retR10;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00);
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10);
      }
    else if (( ixR == _ixR-1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR00 = HeaderR10;

        retR00 =retR10;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR01 = HeaderR11;

        retR01 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11);
      }
    else if (( ixR == _ixR+1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR10 = HeaderR00;

        retR10 = retR00;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR01;

        retR11 = retR01;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00);
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01);
      }
    else
      {
// 	std::cout << " read 4 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00);
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10);
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11);
      }
    ixR = _ixR;
    iyR = _iyR;
  }

  _Src00 = Src00;
  _Src01 = Src01;
  _Src10 = Src10;
  _Src11 = Src11;
  _Rcv00 = Rcv00;
  _Rcv01 = Rcv01;
  _Rcv10 = Rcv10;
  _Rcv11 = Rcv11;

  _HeaderS00 = HeaderS00;
  _HeaderS01 = HeaderS01;
  _HeaderS10 = HeaderS10;
  _HeaderS11 = HeaderS11;

  _HeaderR00 = HeaderR00;
  _HeaderR01 = HeaderR01;
  _HeaderR10 = HeaderR10;
  _HeaderR11 = HeaderR11;

  return ( retS00 && retS01 && retS10 && retS11 && retR00 && retR01 && retR10 && retR11);
}

bool TTFileHandler::Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
                         float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
                         float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
			 SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
			 SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
			 const int N0x, const int Npartx, const int N0y, const int Nparty)
{
  TTFileHandleralloc(Nx, Ny, Nz);

//   std::cout << "TTFileHandler::Init : " << ixS << " " << iyS << " " << ixR << " " << iyR << std::endl;
//   std::cout << "TTFileHandler::Init : " << _ixS << " " << _iyS << " " << _ixR << " " << _iyR;
  if ( (ixS != _ixS) || (iyS != _iyS))
  {
    if (( ixS == _ixS ) && ( iyS == _iyS-1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS00 = HeaderS01;

        retS00 = retS01;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS10 = HeaderS11;

        retS10 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS ) && ( iyS == _iyS+1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS01 = HeaderS00;

        retS01 = retS00;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS10;

        retS11 = retS10;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS-1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS00 = HeaderS10;

        retS00 =retS10;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS01 = HeaderS11;

        retS01 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS+1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS10 = HeaderS00;

        retS10 = retS00;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS01;

        retS11 = retS01;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
      }
    else
      {
// 	std::cout << " read 4 Sources  ";
        retS00 = ReadRT(NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS10 = ReadRT(NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    ixS = _ixS;
    iyS = _iyS;
  }

  if ( (ixR != _ixR) || (iyR != _iyR))
  {
    if (( ixR == _ixR ) && ( iyR == _iyR-1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR00 = HeaderR01;

        retR00 =retR01;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR10 = HeaderR11;

        retR10 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR ) && ( iyR == _iyR+1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR01 = HeaderR00;

        retR01 = retR00;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR10;

        retR11 = retR10;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR-1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR00 = HeaderR10;

        retR00 =retR10;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR01 = HeaderR11;

        retR01 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR+1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR10 = HeaderR00;

        retR10 = retR00;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR01;

        retR11 = retR01;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
      }
    else
      {
// 	std::cout << " read 4 Rcvs\n  ";
        retR00 = ReadRT(NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR10 = ReadRT(NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
        retR01 = ReadRT(NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    ixR = _ixR;
    iyR = _iyR;
  }

  _Src00 = Src00;
  _Src01 = Src01;
  _Src10 = Src10;
  _Src11 = Src11;
  _Rcv00 = Rcv00;
  _Rcv01 = Rcv01;
  _Rcv10 = Rcv10;
  _Rcv11 = Rcv11;

  _HeaderS00 = HeaderS00;
  _HeaderS01 = HeaderS01;
  _HeaderS10 = HeaderS10;
  _HeaderS11 = HeaderS11;

  _HeaderR00 = HeaderR00;
  _HeaderR01 = HeaderR01;
  _HeaderR10 = HeaderR10;
  _HeaderR11 = HeaderR11;

  return ( retS00 && retS01 && retS10 && retS11 && retR00 && retR01 && retR10 && retR11);
}

bool TTFileHandler::Init(const MigrationJob &Job, const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
                         float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
                         float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
			 SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
			 SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
			 const int N0x, const int Npartx, const int N0y, const int Nparty)
{
  TTFileHandleralloc(Nx, Ny, Nz);

//   std::cout << "TTFileHandler::Init : " << ixS << " " << iyS << " " << ixR << " " << iyR << std::endl;
//   std::cout << "TTFileHandler::Init : " << _ixS << " " << _iyS << " " << _ixR << " " << _iyR;
  if ( (ixS != _ixS) || (iyS != _iyS))
  {
    if (( ixS == _ixS ) && ( iyS == _iyS-1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS00 = HeaderS01;

        retS00 = retS01;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS10 = HeaderS11;

        retS10 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS01 = ReadRT(Job,NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(Job,NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS ) && ( iyS == _iyS+1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
	HeaderS01 = HeaderS00;

        retS01 = retS00;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS10;

        retS11 = retS10;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(Job,NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS10 = ReadRT(Job,NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS-1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS00 = HeaderS10;

        retS00 =retS10;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS01 = HeaderS11;

        retS01 =retS11;

// 	std::cout << " read 2 Sources  ";
        retS10 = ReadRT(Job,NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(Job,NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixS == _ixS+1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
	HeaderS10 = HeaderS00;

        retS10 = retS00;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
	HeaderS11 = HeaderS01;

        retS11 = retS01;

// 	std::cout << " read 2 Sources  ";
        retS00 = ReadRT(Job,NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS01 = ReadRT(Job,NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
      }
    else
      {
// 	std::cout << " read 4 Sources  ";
        retS00 = ReadRT(Job,NameBase, _ixS, _iyS, (float*) Src00, HeaderS00, N0x, Npartx, N0y, Nparty);
        retS10 = ReadRT(Job,NameBase, _ixS+1, _iyS, (float*) Src10, HeaderS10, N0x, Npartx, N0y, Nparty);
        retS01 = ReadRT(NameBase, _ixS, _iyS+1, (float*) Src01, HeaderS01, N0x, Npartx, N0y, Nparty);
        retS11 = ReadRT(NameBase, _ixS+1, _iyS+1, (float*) Src11, HeaderS11, N0x, Npartx, N0y, Nparty);
      }
    ixS = _ixS;
    iyS = _iyS;
  }

  if ( (ixR != _ixR) || (iyR != _iyR))
  {
    if (( ixR == _ixR ) && ( iyR == _iyR-1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR00 = HeaderR01;

        retR00 =retR01;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR10 = HeaderR11;

        retR10 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR01 = ReadRT(Job,NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(Job,NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR ) && ( iyR == _iyR+1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
	HeaderR01 = HeaderR00;

        retR01 = retR00;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR10;

        retR11 = retR10;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(Job,NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR10 = ReadRT(Job,NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR-1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR00 = HeaderR10;

        retR00 =retR10;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR01 = HeaderR11;

        retR01 =retR11;

// 	std::cout << " read 2 Rcvs\n  ";
        retR10 = ReadRT(Job,NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(Job,NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    else if (( ixR == _ixR+1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
	HeaderR10 = HeaderR00;

        retR10 = retR00;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
	HeaderR11 = HeaderR01;

        retR11 = retR01;

// 	std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT(Job,NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR01 = ReadRT(Job,NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
      }
    else
      {
// 	std::cout << " read 4 Rcvs\n  ";
        retR00 = ReadRT(Job,NameBase, _ixR, _iyR, (float*) Rcv00, HeaderR00, N0x, Npartx, N0y, Nparty);
        retR10 = ReadRT(Job,NameBase, _ixR+1, _iyR, (float*) Rcv10, HeaderR10, N0x, Npartx, N0y, Nparty);
        retR01 = ReadRT(Job,NameBase, _ixR, _iyR+1, (float*) Rcv01, HeaderR01, N0x, Npartx, N0y, Nparty);
        retR11 = ReadRT(Job,NameBase, _ixR+1, _iyR+1, (float*) Rcv11, HeaderR11, N0x, Npartx, N0y, Nparty);
      }
    ixR = _ixR;
    iyR = _iyR;
  }

  _Src00 = Src00;
  _Src01 = Src01;
  _Src10 = Src10;
  _Src11 = Src11;
  _Rcv00 = Rcv00;
  _Rcv01 = Rcv01;
  _Rcv10 = Rcv10;
  _Rcv11 = Rcv11;

  _HeaderS00 = HeaderS00;
  _HeaderS01 = HeaderS01;
  _HeaderS10 = HeaderS10;
  _HeaderS11 = HeaderS11;

  _HeaderR00 = HeaderR00;
  _HeaderR01 = HeaderR01;
  _HeaderR10 = HeaderR10;
  _HeaderR11 = HeaderR11;

  return ( retS00 && retS01 && retS10 && retS11 && retR00 && retR01 && retR10 && retR11);
}

bool TTFileHandler::Analyse(const char* NameBase, grid3D& GVol, grid2D& GSrc)
{
  char FileName[199];
  sprintf(FileName,"%s_%d_%d.dat", NameBase, 0, 0);

  std::ifstream InputFile(FileName, std::ios::binary);
  if (InputFile.fail())
    {
      std::cerr << "Error in TTFileHandler::Analyse: Failed to open file " << FileName << std::endl;
      return false;
    }
  
  point3D<int> N;
  point3D<float> X0;
  point3D<float> dx;

  int NSrfx, NSrfy;
  float X0Srf, Y0Srf;
  float dxSrf, dySrf;

  SegYHeader Header;
  InputFile.read((char*) &Header, sizeof(SegYHeader));

  X0[0] = Header.gx;
  X0[1] = Header.gy;
  N[2] = Header.ns; 
  dx[2] = Header.dt;
  X0[2] = -(N[2]-1) * dx[2];

  float* buffer = new float[N[2]*N_SIGNALS];
  InputFile.read((char*) buffer, N[2] * N_SIGNALS * sizeof(float));
  InputFile.read((char*) &Header, sizeof(SegYHeader));
  
  dx[1] = Header.gy - X0[1];
  N[1] = 1;
  while ( Header.gy == X0[1] + N[1] * dx[1])
    {
      InputFile.read((char*) buffer, N[2] * N_SIGNALS * sizeof(float));
      InputFile.read((char*) &Header, sizeof(SegYHeader));
      N[1]++;
    }

  dx[0] = Header.gx - X0[0];
  InputFile.seekg(0, std::ios::end);
  int Size = InputFile.tellg();
  int Ntot = Size/(N[2] * N_SIGNALS * sizeof(float) + sizeof(SegYHeader));
  N[0] = Ntot/(N[1]);
  InputFile.close();

  std::cout << X0 << std::endl;
  std::cout << N << std::endl;
  std::cout << dx << std::endl;

  GVol.Init(X0, N, dx);

  X0Srf = Header.sx;
  Y0Srf = Header.sy;

  sprintf(FileName,"%s_%d_%d.dat", NameBase, 1, 1);

  InputFile.open(FileName, std::ios::binary);
  if (InputFile.fail())
    {
      std::cerr << "Error in TTFileHandler::Analyse: Failed to open file " << FileName << std::endl;
      delete[] buffer;
      return false;
    }
  InputFile.read((char*) &Header, sizeof(SegYHeader));

  dxSrf = Header.sx - X0Srf;
  dySrf = Header.sy - Y0Srf;
  InputFile.close();
  
  NSrfx = 0; 
  NSrfy = 0;
  sprintf(FileName,"%s_%d_%d.dat", NameBase, NSrfx, NSrfy);
  InputFile.open(FileName, std::ios::binary);
  while (!InputFile.fail())
    {
      InputFile.close();
      NSrfy++;
      sprintf(FileName,"%s_%d_%d.dat", NameBase, NSrfx, NSrfy);
      InputFile.open(FileName, std::ios::binary);
    }
  InputFile.close();
  sprintf(FileName,"%s_%d_%d.dat", NameBase, NSrfx, NSrfy-1);

  std::ifstream InputFile2(FileName, std::ios::binary);
  while (!InputFile2.fail())
    {
      InputFile2.close();
      NSrfx++;
      sprintf(FileName,"%s_%d_%d.dat", NameBase, NSrfx, NSrfy-1);
      InputFile2.open(FileName, std::ios::binary);
    }
  InputFile2.close();

  std::cout << X0Srf << " " << Y0Srf << std::endl;
  std::cout << NSrfx << " " << NSrfy << std::endl;
  std::cout << dxSrf << " " << dySrf << std::endl;

  GSrc.Init(X0Srf, Y0Srf, NSrfx, NSrfy, dxSrf, dySrf);

  delete[] buffer;
  return true;
}

/** TTFileHandler (const TTFileHandler& ) */
TTFileHandler::TTFileHandler (const TTFileHandler& ){
}
/** operator = (const TTFileHandler& ) */
TTFileHandler& TTFileHandler::operator = (const TTFileHandler& _RT){
  std::cerr << "TTFileHandler operator = should never be called\n";
  return *this;
}

int TTFileHandler::WriteXML(const TracingJob& Job)
{
    int ierr = 0;
  char* filename = new char[199];
  if ( PreFileName != NULL )
  {
      sprintf(filename, "%sTTTable.xml", PreFileName);
  }
  else
      sprintf(filename, "TTTable.xml");

  CheckReadTracingJob Writer;

  ierr = Writer.WriteConfigFileXML(filename, Job);

  delete[] filename;
  return ierr;
}

void TTFileHandler::WriteRcvSegY(const point3D<float>& Srcpos, const ReceiverGrid& RcvGrid, Acq_geometry<float>& Geom, const int& SrcX, const int& SrcY)
{
  char* filename = new char[199];
  if ( PreFileName != NULL )
  {
      sprintf(filename, "%sLZ_%d_%d.dat", PreFileName, SrcX, SrcY);
  }
  else
      sprintf(filename, "LZ_%d_%d.dat", SrcX, SrcY);
  
  
  std::ofstream outfile(filename, std::ios::binary);
  if (outfile.fail()) {
    
    return; 
  }

  int Nx = RcvGrid.getNx();
  int Ny = RcvGrid.getNy();
  int Nz = RcvGrid.getNz();

  point3D<float> X0 = RcvGrid.getX0();
  point3D<float> dx = RcvGrid.getdx();

  float* buffer = new float[Nz*N_SIGNALS];

  SegYHeader Header = {};

  Header.ns = Nz * N_SIGNALS;
  Header.dt = (int) (dx[2] * 1000);

  int index = 0;
  for (int i = 0; i < Nx; i++)
    for (int j = 0; j < Ny; j++)
      {
        Header.tracl = index;
        Header.tracr = index;
        index++;

	float sx_UTM, sy_UTM;
	Geom.MODxy_to_WORLDxy(Srcpos[0], Srcpos[1], &sx_UTM, &sy_UTM);
        Header.sx = static_cast<int>(sx_UTM);
        Header.sy = static_cast<int>(sy_UTM);
        Header.selev = -static_cast<int>(Srcpos[2]);

	float gx_UTM, gy_UTM;
	Geom.MODxy_to_WORLDxy(X0[0] + i * dx[0], X0[1] + j * dx[1], &gx_UTM, &gy_UTM);
        Header.gx = static_cast<int>(gx_UTM);
        Header.gy = static_cast<int>(gy_UTM);
        Header.gelev = -static_cast<int>(X0[2]);

        outfile.write(reinterpret_cast<char*>(&Header), sizeof(SegYHeader));
	memset( (void*) buffer, 0, Nz*N_SIGNALS*sizeof(float));
        for (int k = 0; k < Nz; k++)
          {
            Receiver* Rcv = RcvGrid.GetPointAt(i, j, k);
            RecSig* pSig = Rcv->pSignal;
            if ( pSig == &Rcv->Signals[g_MAXSIG-1]) // no signal recorded
              buffer[k*N_SIGNALS] = INVALID_SIGT;
//               buffer[Nz-k-1] = NoSig;
            else // write last signal
              {
		  // first arrival
		  pSig = &Rcv->Signals[g_MAXSIG-1];
		  // last arrival
		  //pSig++;
		  if ( (pSig->GetT() != pSig->GetT())
		       || (pSig->GetT()+1 == pSig->GetT()) )
		      buffer[k*N_SIGNALS] = INVALID_SIGT;
//                   buffer[Nz-k-1] = NoSig;
		  else
		  {
		      buffer[k*N_SIGNALS + 0] = pSig->GetT();
		      if (N_SIGNALS > 1)
		      {
			  buffer[k*N_SIGNALS + 1] = pSig->Getpx();
			  buffer[k*N_SIGNALS + 2] = pSig->Getpy();
			  buffer[k*N_SIGNALS + 3] = pSig->Getpz();
			  buffer[k*N_SIGNALS + 4] = pSig->Getdpxdx();
			  buffer[k*N_SIGNALS + 5] = pSig->Getdpydx();
			  buffer[k*N_SIGNALS + 6] = pSig->Getdpzdx();
			  buffer[k*N_SIGNALS + 7] = pSig->Getdpxdy();
			  buffer[k*N_SIGNALS + 8] = pSig->Getdpydy();
			  buffer[k*N_SIGNALS + 9] = pSig->Getdpzdy();
			  buffer[k*N_SIGNALS + 10] = pSig->Getv();
			  buffer[k*N_SIGNALS + 11] = pSig->GetAmp();
		      }
		  }
//                   buffer[Nz-k-1] = pSig->arrtime;
              }
          }

        outfile.write(reinterpret_cast<char*>(buffer), Nz*N_SIGNALS*sizeof(float));
      }

  delete[] buffer;
  delete[] filename;
  outfile.close();
}

int TTFileHandler::SetDirectory(const char* DirName, const char* PreFix)
{
    int ierr = 0;
    if (DirName != NULL)
    {
	int len = strlen(DirName);
	if (PreFix != NULL)
	    len += strlen(PreFix);

	if ( len > 198 )
	{
	    
	    ierr = 1;
	}
	else
	{
	    if (PreFileName != NULL)
		delete PreFileName;
	    PreFileName = new char[len+3];
	    if ((PreFix != NULL) && (strlen(PreFix) > 0))
		sprintf(PreFileName, "%s/%s_", DirName, PreFix);
	    else
		sprintf(PreFileName, "%s/", DirName);
	}
	
    }
    return ierr;
}
