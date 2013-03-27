/***************************************************************************
                          ttvmmemhandler.cpp  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "ttvmmemhandler.h"
volatile int TTVMMemHandler::threadcom = 0;

#include <pthread.h>

TTVMMemHandler::TTVMMemHandler():Src00(NULL),Src01(NULL),Src10(NULL),Src11(NULL),Rcv00(NULL),Rcv01(NULL),Rcv10(NULL),Rcv11(NULL)
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
TTVMMemHandler::TTVMMemHandler(const int _Nx, const int _Ny, const int _Nz):Src00(NULL),Src01(NULL),Src10(NULL),Src11(NULL),Rcv00(NULL),Rcv01(NULL),Rcv10(NULL),Rcv11(NULL)
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

TTVMMemHandler::TTVMMemHandler ( const MigrationJob &_Job
                               , int _Ntid
                               , char * _VMem = NULL
                               )
  :Src00(NULL),Src01(NULL),Src10(NULL),Src11(NULL),Rcv00(NULL),Rcv01(NULL),Rcv10(NULL),Rcv11(NULL)
{
  Ntid = _Ntid;

//   VMem = ((char*) PE->getMemPtr())+_Job.globTTbufoff;
//   PRank = PE->GetRank();
//   PSize = PE->GetNodeCount();

  VMem = _VMem;
  PRank = fvmGetRank();
  PSize = fvmGetNodeCount();

  endianess = TestByteOrder();
  PreFileName = NULL;
  Nx = _Job.TTVol.nx_xlines;
  Ny = _Job.TTVol.ny_inlines;
  Nz = _Job.TTVol.nz;
  NSrfx = _Job.SrfcGridNx;
  NSrfy = _Job.SrfcGridNy;
  ixS = -100;
  iyS = -100;
  ixR = -100;
  iyR = -100;

  TTLength = sizeof(SegYHeader) + Nx*Ny*Nz*N_SIGNALS*sizeof(float);
}

void TTVMMemHandler::Reset()
{
  ixS = -100;
  iyS = -100;
  ixR = -100;
  iyR = -100;
}
unsigned long TTVMMemHandler::GetMem(const char* NameBase, const grid2D& GSrc, const grid3D& GVol, const int NodeCount, const int CoreCount)
{
  Nx = GVol.getNx();
  Ny = GVol.getNy();
  Nz = GVol.getNz();

  NSrfx =  GSrc.getNx();
  NSrfy =  GSrc.getNy();

  TTLength = sizeof(SegYHeader) + (unsigned long) Nx* (unsigned long) Ny*(unsigned long) Nz*(unsigned long) N_SIGNALS*(unsigned long) sizeof(float);

  unsigned long TotalMem = ((NodeCount*CoreCount*8 + NodeCount * ((NSrfx*NSrfy + NodeCount-1)/NodeCount)) * TTLength);



  // 10% of additional space
  TotalMem += (unsigned long) (0.1 * TotalMem);
  return TotalMem;
}

#include <fvm-pc/pc.hpp>
#include <stdexcept>

void TTVMMemHandler::InitVol ( const MigrationJob& MigJob
                             , const char* NameBase
                             , const grid2D& GSrc
                             , const grid3D& GVol
                             , const int _Ntid
                             , const int mtid
                             , const long myPart
                             , const long numPart
                             , const fvmAllocHandle_t handle_TT
                             )
{
  Ntid = _Ntid;

//   VMem = ( (char*) PE->getMemPtr() )+MigJob.globTTbufoff;
//   PRank = PE->GetRank();
//   PSize = PE->GetNodeCount();

//???  const unsigned long VMemOff (sizeof (MigJob));
  const unsigned long VMemOff (MigJob.shift_for_TT);

  VMem = ((char *)fvmGetShmemPtr()) + VMemOff;
  PRank = fvmGetRank();
  PSize = fvmGetNodeCount();

  Nx = GVol.getNx();
  Ny = GVol.getNy();
  Nz = GVol.getNz();

  NSrfx =  GSrc.getNx();
  NSrfy =  GSrc.getNy();

  TTLength = sizeof(SegYHeader) + Nx*Ny*Nz*N_SIGNALS*sizeof(float);

  const unsigned long SizePerFile
    (sizeof(SegYHeader) + Nx*Ny*Nz*N_SIGNALS*sizeof(float));

  // read all TT files
  if (mtid == 0)
    {
      char FileName[199];
      Acq_geometry<float> Geom(MigJob.geom);
      for (int iSrcx = 0; iSrcx < NSrfx; iSrcx++)
        {
          for (int iSrcy = 0; iSrcy < NSrfy; iSrcy++)
            {
              const long SrcIndex = iSrcx*NSrfy + iSrcy;

              //              if ( (SrcIndex % PSize) == PRank )
              if (SrcIndex % numPart == myPart)
                {
                  const unsigned long HeaderAddress
                    //                    = Ntid*8*TTLength + (SrcIndex/PSize) * TTLength;
                    = 0;

                  sprintf(FileName,"%s_%d_%d.dat", NameBase, iSrcx, iSrcy);
                  std::ifstream Input(FileName, std::ios::binary);
                  //                  std::cout << "FileName = " << FileName << std::endl;

                  if (Input.fail())
                    {
                      std::cerr << "Error in TTVMMemHandler::ReadRT: Failed to open file " << FileName << std::endl;
                      continue;
                    }

                  for (int i = 0; i < Nx; i++)
                    for (int j = 0; j < Ny; j++)
                      {
                        const unsigned long Address
                          = HeaderAddress
                          + sizeof(SegYHeader)
                          + (i * Ny + j)*Nz*N_SIGNALS*sizeof(float);

                        SegYHeader* Header =  (SegYHeader*) &(VMem[HeaderAddress]);
                        char* buffer = &(VMem[Address]);
                        Input.read((char*) Header, sizeof(SegYHeader));
                        Input.read(buffer, Nz*N_SIGNALS*sizeof(float));

                        if (endianess != LITENDIAN)
                          {
                            swap_bytes((void*)&(Header->sx), 1, sizeof(int));
                            swap_bytes((void*)&(Header->sy), 1, sizeof(int));
                            swap_bytes((void*)&(Header->gx), 1, sizeof(int));
                            swap_bytes((void*)&(Header->gy), 1, sizeof(int));
                            swap_bytes((void*)&(Header->scalco), 1, sizeof(short));
                            swap_bytes((void*)&(Header->selev), 1, sizeof(int));
                            swap_bytes((void*)&(Header->gelev), 1, sizeof(int));
                            swap_bytes((void*)&(Header->scalel), 1, sizeof(short));

                            swap_bytes((void*) buffer, Nz*N_SIGNALS, sizeof(float));
                          }

                        float sx_MOD, sy_MOD, gx_MOD, gy_MOD;
                        Geom.WORLDxy_to_MODxy(Header->sx, Header->sy,
                                              &sx_MOD, &sy_MOD);
                        Geom.WORLDxy_to_MODxy(Header->gx, Header->gy,
                                              &gx_MOD, &gy_MOD);

                        Header->sx = static_cast<int>(roundf(sx_MOD));
                        Header->sy = static_cast<int>(roundf(sy_MOD));
                        Header->gx = static_cast<int>(roundf(gx_MOD));
                        Header->gy = static_cast<int>(roundf(gy_MOD));
                      }
                  Input.close();

                  const unsigned long off
                    (Ntid*8*TTLength + (SrcIndex/PSize) * TTLength);
                  const unsigned long SrcRank (SrcIndex % PSize);
                  const unsigned long shift
                    (SrcRank * MigJob.globTTbufsizelocal);

//                   std::cout << "Put to off "
//                             << (off + shift)
//                             << " from " << VMemOff
//                             << " size " << SizePerFile
//                             << std::endl;

                  waitComm
                    ( fvmPutGlobalData ( handle_TT
                                       , off + shift
                                       , SizePerFile
                                       , VMemOff
                                       , 0
                                       )
                    );
                }
            }
        }
    }

  if (mtid == 0)
    {
      //      PE->Barrier();
    }

  if (mtid == 0)
    threadcom = 1;
  else
    while (threadcom != 1) {};
  //  std::cout << PRank << "," << mtid << " has passed the barrier\n";
}

TTVMMemHandler::~TTVMMemHandler()
{
    if ( PreFileName != NULL )
        delete[] PreFileName;
    clear();
}
void TTVMMemHandler::clear()
{
  if (Src00 != NULL)
  {
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
void TTVMMemHandler::TTVMMemHandleralloc(const int Nx, const int Ny, const int Nz, const int mtid)
{
  if (Src00 == NULL)
    {
#ifdef DEBUG
      std::cout << "TTVMMemHandleralloc is allocating memory\n";
#endif
      Src00 = (float*) &(VMem[(mtid * 8 + 0)*TTLength]);
      Src00_Address = (mtid * 8 + 0)*TTLength;
// printf("Src00_Address=%li (mtid=%i TTLength=%li pos=%i)\n",Src00_Address,mtid,TTLength,mtid * 8 + 0);
      Src01 = (float*) &(VMem[(mtid * 8 + 1)*TTLength]);
      Src01_Address = (mtid * 8 + 1)*TTLength;
// printf("Src01_Address=%li (mtid=%i TTLength=%li pos=%i)\n",Src01_Address,mtid,TTLength,mtid * 8 + 1);
      Src10 = (float*) &(VMem[(mtid * 8 + 2)*TTLength]);
      Src10_Address = (mtid * 8 + 2)*TTLength;
// printf("Src10_Address=%li (mtid=%i TTLength=%li pos=%i)\n",Src10_Address,mtid,TTLength,mtid * 8 + 2);
      Src11 = (float*) &(VMem[(mtid * 8 + 3)*TTLength]);
      Src11_Address = (mtid * 8 + 3)*TTLength;
// printf("Src11_Address=%li (mtid=%i TTLength=%li pos=%i)\n",Src11_Address,mtid,TTLength,mtid * 8 + 3);
      Rcv00 = (float*) &(VMem[(mtid * 8 + 4)*TTLength]);
      Rcv00_Address = (mtid * 8 + 4)*TTLength;
      Rcv01 = (float*) &(VMem[(mtid * 8 + 5)*TTLength]);
      Rcv01_Address = (mtid * 8 + 5)*TTLength;
      Rcv10 = (float*) &(VMem[(mtid * 8 + 6)*TTLength]);
      Rcv10_Address = (mtid * 8 + 6)*TTLength;
      Rcv11 = (float*) &(VMem[(mtid * 8 + 7)*TTLength]);
      Rcv11_Address = (mtid * 8 + 7)*TTLength;
    }
}

// bool TTVMMemHandler::ReadRT ( const int& ix
//                             , const int& iy
//                             , float* buffer
//                             , unsigned long bufferaddress
//                             , SegYHeader& Header
//                             , const int mtid
//                             , const fvmAllocHandle_t handle_TT
//                             , const MigrationJob & Job
//                             )

// {
// //   const unsigned int SrcIndex = ix*NSrfy + iy;
// //   int SrcRank = SrcIndex % PSize;
// //   unsigned long SrcAddress = Ntid*8*TTLength + (SrcIndex/PSize) * TTLength;

// //   if ( (SrcRank) == PRank )
// //   {
// //       memcpy((char*) buffer, (char*) &(VMem[SrcAddress]), TTLength);
// //   }
// //   else
// //   {
// //     //      PE->readMem(bufferaddress, SrcAddress, TTLength, SrcRank, (vm_queue) mtid);
// //       //      PE->WaitOnQueue( (vm_queue) mtid);
// //   }

// //   Header = *((SegYHeader*)buffer);

// //   return true;

//   const unsigned int SrcIndex = ix*NSrfy + iy;
//   int SrcRank = SrcIndex % PSize;
//   unsigned long SrcAddress = Ntid*8*TTLength + (SrcIndex/PSize) * TTLength;

// //     PE->readMem( bufferaddress // OFFSET on local node
// //                , SrcAddress    // OFFSET on remote node
// //                , TTLength
// //                , SrcRank
// //                , (vm_queue) mtid
// //                );
//       //      PE->WaitOnQueue( (vm_queue) mtid);


//   pthread_mutex_lock (&fvm_mutex);

// //   std::cout << "mtid " << mtid
// //             << " get from handle_TT " << SrcAddress + SrcRank * Job.globTTbufsizelocal
// //             << " to shmem " << bufferaddress + Job.shift_for_TT
// //             << " size " << TTLength
// //             << std::endl;

//   waitComm (fvmGetGlobalData ( handle_TT
//                              , SrcAddress + SrcRank * Job.globTTbufsizelocal
//                              , TTLength
//                              , bufferaddress + Job.shift_for_TT
//                              , 0
//                              )
//            )
//     ;

//   pthread_mutex_unlock (&fvm_mutex);

//   Header = *((SegYHeader*)buffer);

//   return true;
// };

bool TTVMMemHandler::ReadRT ( const int& ix
                            , const int& iy
                            , float* buffer
                            , unsigned long bufferaddress
                            , SegYHeader & Header
                            , const int N0x
                            , const int Npartx
                            , const int N0y
                            , const int Nparty
                            , const int mtid
                             , const fvmAllocHandle_t handle_TT
                            , const MigrationJob & Job
                            )
{
  const unsigned long SrcIndex = ix*NSrfy + iy;
  unsigned long SrcRank = SrcIndex % PSize;
  unsigned long SrcAddress = Ntid*8*TTLength + (SrcIndex/PSize) * TTLength;
  const unsigned long shift (SrcRank * Job.globTTbufsizelocal);

  // read header

  {
    waitComm (fvmGetGlobalData ( handle_TT
                               , SrcAddress + shift
                               , sizeof(SegYHeader)
                               , bufferaddress + Job.shift_for_TT
                               , 0
                               )
             );

    //     PE->readMem( bufferaddress
    //                , SrcAddress
    //                , sizeof(SegYHeader)
    //                , SrcRank
    //                , (vm_queue) mtid
    //                );
  }


  // read x slices
  {
    const unsigned int TTLengthpart = (Nparty + 1)*Nz*N_SIGNALS*sizeof(float);

    for (int iVolx = N0x; iVolx < N0x + Npartx + 1; iVolx++)
    {
      const unsigned long addressoffset =  sizeof(SegYHeader) + (iVolx*Ny + N0y)*Nz*N_SIGNALS*sizeof(float);

      waitComm ( fvmGetGlobalData
                 ( handle_TT
                 , SrcAddress + addressoffset + shift
                 , TTLengthpart
                 , bufferaddress + addressoffset + Job.shift_for_TT
                 , 0
                 )
               );
    }
  }

//   std::cout << "mtid " << mtid << " done ReadRT" << std::endl;

  Header = *((SegYHeader*)buffer);

  return true;
};


// bool TTVMMemHandler::Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
//                          float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
//                          float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
//                       SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
//                       SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11, const int mtid
//                          , const fvmAllocHandle_t handle_TT
//                          , const MigrationJob & Job
//                          )
// {
//   TTVMMemHandleralloc(Nx, Ny, Nz, mtid);

// //   std::cout << "TTVMMemHandler::Init : " << ixS << " " << iyS << " " << ixR << " " << iyR << std::endl;
// //   std::cout << "TTVMMemHandler::Init : " << _ixS << " " << _iyS << " " << _ixR << " " << _iyR;
//   if ( (ixS != _ixS) || (iyS != _iyS))
//   {
//     if (( ixS == _ixS ) && ( iyS == _iyS-1))
//       {
//         float* tmp0 = Src00;
//         Src00 = Src01;
//         Src01 = tmp0;
//      unsigned long atmp0 = Src00_Address;
//      Src00_Address = Src01_Address;
//      Src01_Address = atmp0;
//      HeaderS00 = HeaderS01;

//         retS00 = retS01;

//         float* tmp1 = Src10;
//         Src10 = Src11;
//         Src11 = tmp1;
//      unsigned long atmp1 = Src10_Address;
//      Src10_Address = Src11_Address;
//      Src11_Address = atmp1;
//      HeaderS10 = HeaderS11;

//         retS10 =retS11;

// //   std::cout << " read 2 Sources  ";
//         retS01 = ReadRT(_ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, mtid, handle_TT, Job);
//         retS11 = ReadRT(_ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, mtid, handle_TT, Job);
//       }
//     else if (( ixS == _ixS ) && ( iyS == _iyS+1))
//       {
//         float* tmp0 = Src00;
//         Src00 = Src01;
//         Src01 = tmp0;
//      unsigned long atmp0 = Src00_Address;
//      Src00_Address = Src01_Address;
//      Src01_Address = atmp0;
//      HeaderS01 = HeaderS00;

//         retS01 = retS00;

//         float* tmp1 = Src10;
//         Src10 = Src11;
//         Src11 = tmp1;
//      unsigned long atmp1 = Src10_Address;
//      Src10_Address = Src11_Address;
//      Src11_Address = atmp1;
//      HeaderS11 = HeaderS10;

//         retS11 = retS10;

// //   std::cout << " read 2 Sources  ";
//         retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, mtid, handle_TT, Job);
//         retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, mtid, handle_TT, Job);
//       }
//     else if (( ixS == _ixS-1 ) && ( iyS == _iyS))
//       {
//         float* tmp0 = Src00;
//         Src00 = Src10;
//         Src10 = tmp0;
//      unsigned long atmp0 = Src00_Address;
//      Src00_Address = Src10_Address;
//      Src10_Address = atmp0;
//      HeaderS00 = HeaderS10;

//         retS00 =retS10;

//         float* tmp1 = Src01;
//         Src01 = Src11;
//         Src11 = tmp1;
//      unsigned long atmp1 = Src01_Address;
//      Src01_Address = Src11_Address;
//      Src11_Address = atmp1;
//      HeaderS01 = HeaderS11;

//         retS01 =retS11;

// //   std::cout << " read 2 Sources  ";
//         retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, mtid, handle_TT, Job);
//         retS11 = ReadRT( _ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, mtid, handle_TT, Job);
//       }
//     else if (( ixS == _ixS+1 ) && ( iyS == _iyS))
//       {
//         float* tmp0 = Src00;
//         Src00 = Src10;
//         Src10 = tmp0;
//      unsigned long atmp0 = Src00_Address;
//      Src00_Address = Src10_Address;
//      Src10_Address = atmp0;
//      HeaderS10 = HeaderS00;

//         retS10 = retS00;

//         float* tmp1 = Src01;
//         Src01 = Src11;
//         Src11 = tmp1;
//      unsigned long atmp1 = Src01_Address;
//      Src01_Address = Src11_Address;
//      Src11_Address = atmp1;
//      HeaderS11 = HeaderS01;

//         retS11 = retS01;

// //   std::cout << " read 2 Sources  ";
//         retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, mtid, handle_TT, Job);
//         retS01 = ReadRT( _ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, mtid, handle_TT, Job);
//       }
//     else
//       {
// //   std::cout << " read 4 Sources  ";
//         retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, mtid, handle_TT, Job);
//         retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, mtid, handle_TT, Job);
//         retS01 = ReadRT( _ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, mtid, handle_TT, Job);
//         retS11 = ReadRT( _ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, mtid, handle_TT, Job);
//       }
//     ixS = _ixS;
//     iyS = _iyS;
//   }

//   if ( (ixR != _ixR) || (iyR != _iyR))
//   {
//     if (( ixR == _ixR ) && ( iyR == _iyR-1))
//       {
//         float* tmp0 = Rcv00;
//         Rcv00 = Rcv01;
//         Rcv01 = tmp0;
//      unsigned long atmp0 = Rcv00_Address;
//      Rcv00_Address = Rcv01_Address;
//      Rcv01_Address = atmp0;
//      HeaderR00 = HeaderR01;

//         retR00 =retR01;

//         float* tmp1 = Rcv10;
//         Rcv10 = Rcv11;
//         Rcv11 = tmp1;
//      unsigned long atmp1 = Rcv10_Address;
//      Rcv10_Address = Rcv11_Address;
//      Rcv11_Address = atmp1;
//      HeaderR10 = HeaderR11;

//         retR10 =retR11;

// //   std::cout << " read 2 Rcvs\n  ";
//         retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, mtid, handle_TT, Job);
//         retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, mtid, handle_TT, Job);
//       }
//     else if (( ixR == _ixR ) && ( iyR == _iyR+1))
//       {
//         float* tmp0 = Rcv00;
//         Rcv00 = Rcv01;
//         Rcv01 = tmp0;
//      unsigned long atmp0 = Rcv00_Address;
//      Rcv00_Address = Rcv01_Address;
//      Rcv01_Address = atmp0;
//      HeaderR01 = HeaderR00;

//         retR01 = retR00;

//         float* tmp1 = Rcv10;
//         Rcv10 = Rcv11;
//         Rcv11 = tmp1;
//      unsigned long atmp1 = Rcv10_Address;
//      Rcv10_Address = Rcv11_Address;
//      Rcv11_Address = atmp1;
//      HeaderR11 = HeaderR10;

//         retR11 = retR10;

// //   std::cout << " read 2 Rcvs\n  ";
//         retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, mtid, handle_TT, Job);
//         retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, mtid, handle_TT, Job);
//       }
//     else if (( ixR == _ixR-1 ) && ( iyR == _iyR))
//       {
//         float* tmp0 = Rcv00;
//         Rcv00 = Rcv10;
//         Rcv10 = tmp0;
//      unsigned long atmp0 = Rcv00_Address;
//      Rcv00_Address = Rcv10_Address;
//      Rcv10_Address = atmp0;
//      HeaderR00 = HeaderR10;

//         retR00 =retR10;

//         float* tmp1 = Rcv01;
//         Rcv01 = Rcv11;
//         Rcv11 = tmp1;
//      unsigned long atmp1 = Rcv01_Address;
//      Rcv01_Address = Rcv11_Address;
//      Rcv11_Address = atmp1;
//      HeaderR01 = HeaderR11;

//         retR01 =retR11;

// //   std::cout << " read 2 Rcvs\n  ";
//         retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, mtid, handle_TT, Job);
//         retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, mtid, handle_TT, Job);
//       }
//     else if (( ixR == _ixR+1 ) && ( iyR == _iyR))
//       {
//         float* tmp0 = Rcv00;
//         Rcv00 = Rcv10;
//         Rcv10 = tmp0;
//      unsigned long atmp0 = Rcv00_Address;
//      Rcv00_Address = Rcv10_Address;
//      Rcv10_Address = atmp0;
//      HeaderR10 = HeaderR00;

//         retR10 = retR00;

//         float* tmp1 = Rcv01;
//         Rcv01 = Rcv11;
//         Rcv11 = tmp1;
//      unsigned long atmp1 = Rcv01_Address;
//      Rcv01_Address = Rcv11_Address;
//      Rcv11_Address = atmp1;
//      HeaderR11 = HeaderR01;

//         retR11 = retR01;

// //   std::cout << " read 2 Rcvs\n  ";
//         retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, mtid, handle_TT, Job);
//         retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, mtid, handle_TT, Job);
//       }
//     else
//       {
// //   std::cout << " read 4 Rcvs\n  ";
//         retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, mtid, handle_TT, Job);
//         retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, mtid, handle_TT, Job);
//         retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, mtid, handle_TT, Job);
//         retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, mtid, handle_TT, Job);
//       }
//     ixR = _ixR;
//     iyR = _iyR;
//   }

//   _Src00 = &(Src00[sizeof(SegYHeader)/sizeof(float)]);
//   _Src01 = &(Src01[sizeof(SegYHeader)/sizeof(float)]);
//   _Src10 = &(Src10[sizeof(SegYHeader)/sizeof(float)]);
//   _Src11 = &(Src11[sizeof(SegYHeader)/sizeof(float)]);
//   _Rcv00 = &(Rcv00[sizeof(SegYHeader)/sizeof(float)]);
//   _Rcv01 = &(Rcv01[sizeof(SegYHeader)/sizeof(float)]);
//   _Rcv10 = &(Rcv10[sizeof(SegYHeader)/sizeof(float)]);
//   _Rcv11 = &(Rcv11[sizeof(SegYHeader)/sizeof(float)]);

//   _HeaderS00 = HeaderS00;
//   _HeaderS01 = HeaderS01;
//   _HeaderS10 = HeaderS10;
//   _HeaderS11 = HeaderS11;

//   _HeaderR00 = HeaderR00;
//   _HeaderR01 = HeaderR01;
//   _HeaderR10 = HeaderR10;
//   _HeaderR11 = HeaderR11;

//   return ( retS00 && retS01 && retS10 && retS11 && retR00 && retR01 && retR10 && retR11);
// }

// used in kdm_simple
bool TTVMMemHandler::Init(const char* NameBase, const int& _ixS, const int& _iyS, const int& _ixR, const int& _iyR,
                         float* &_Src00, float* &_Src01, float* &_Src10, float* &_Src11,
                         float* &_Rcv00, float* &_Rcv01, float* &_Rcv10, float* &_Rcv11,
                         SegYHeader& _HeaderS00, SegYHeader& _HeaderS01, SegYHeader& _HeaderS10, SegYHeader& _HeaderS11,
                         SegYHeader& _HeaderR00, SegYHeader& _HeaderR01, SegYHeader& _HeaderR10, SegYHeader& _HeaderR11,
                         const int N0x, const int Npartx, const int N0y, const int Nparty, const int mtid
                         , const fvmAllocHandle_t handle_TT
                         , const MigrationJob & Job
                         )
{
  TTVMMemHandleralloc(Nx, Ny, Nz, mtid);

//   std::cout << "TTVMMemHandler::Init : " << ixS << " " << iyS << " " << ixR << " " << iyR << std::endl;
//   std::cout << "TTVMMemHandler::Init : " << _ixS << " " << _iyS << " " << _ixR << " " << _iyR;
//   printf("%i: TTVMMemHandler::Init : %i %i (%i %i)\n",mtid, ixS, iyS, _ixS, _iyS);

  if ( (ixS != _ixS) || (iyS != _iyS))
  {
    if (( ixS == _ixS ) && ( iyS == _iyS-1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
        unsigned long atmp0 = Src00_Address;
        Src00_Address = Src01_Address;
        Src01_Address = atmp0;
        HeaderS00 = HeaderS01;

        retS00 = retS01;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
        unsigned long atmp1 = Src10_Address;
        Src10_Address = Src11_Address;
        Src11_Address = atmp1;
        HeaderS10 = HeaderS11;

        retS10 =retS11;

//      std::cout << " read 2 Sources  ";
        retS01 = ReadRT( _ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS11 = ReadRT( _ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixS == _ixS ) && ( iyS == _iyS+1))
      {
        float* tmp0 = Src00;
        Src00 = Src01;
        Src01 = tmp0;
        unsigned long atmp0 = Src00_Address;
        Src00_Address = Src01_Address;
        Src01_Address = atmp0;
        HeaderS01 = HeaderS00;

        retS01 = retS00;

        float* tmp1 = Src10;
        Src10 = Src11;
        Src11 = tmp1;
        unsigned long atmp1 = Src10_Address;
        Src10_Address = Src11_Address;
        Src11_Address = atmp1;
        HeaderS11 = HeaderS10;

        retS11 = retS10;

//      std::cout << " read 2 Sources  ";
        retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixS == _ixS-1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
        unsigned long atmp0 = Src00_Address;
        Src00_Address = Src10_Address;
        Src10_Address = atmp0;
        HeaderS00 = HeaderS10;

        retS00 =retS10;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
        unsigned long atmp1 = Src01_Address;
        Src01_Address = Src11_Address;
        Src11_Address = atmp1;
        HeaderS01 = HeaderS11;

        retS01 =retS11;

//      std::cout << " read 2 Sources  ";
        retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS11 = ReadRT( _ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixS == _ixS+1 ) && ( iyS == _iyS))
      {
        float* tmp0 = Src00;
        Src00 = Src10;
        Src10 = tmp0;
        unsigned long atmp0 = Src00_Address;
        Src00_Address = Src10_Address;
        Src10_Address = atmp0;
        HeaderS10 = HeaderS00;

        retS10 = retS00;

        float* tmp1 = Src01;
        Src01 = Src11;
        Src11 = tmp1;
        unsigned long atmp1 = Src01_Address;
        Src01_Address = Src11_Address;
        Src11_Address = atmp1;
        HeaderS11 = HeaderS01;

        retS11 = retS01;

//      std::cout << " read 2 Sources  ";
        retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS01 = ReadRT( _ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else
      {
//      std::cout << " read 4 Sources  ";
        retS00 = ReadRT( _ixS, _iyS, (float*) Src00, Src00_Address, HeaderS00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS10 = ReadRT( _ixS+1, _iyS, (float*) Src10, Src10_Address, HeaderS10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS01 = ReadRT( _ixS, _iyS+1, (float*) Src01, Src01_Address, HeaderS01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retS11 = ReadRT( _ixS+1, _iyS+1, (float*) Src11, Src11_Address, HeaderS11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
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
        unsigned long atmp0 = Rcv00_Address;
        Rcv00_Address = Rcv01_Address;
        Rcv01_Address = atmp0;
        HeaderR00 = HeaderR01;

        retR00 =retR01;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
        unsigned long atmp1 = Rcv10_Address;
        Rcv10_Address = Rcv11_Address;
        Rcv11_Address = atmp1;
        HeaderR10 = HeaderR11;

        retR10 =retR11;

//      std::cout << " read 2 Rcvs\n  ";
        retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixR == _ixR ) && ( iyR == _iyR+1))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv01;
        Rcv01 = tmp0;
        unsigned long atmp0 = Rcv00_Address;
        Rcv00_Address = Rcv01_Address;
        Rcv01_Address = atmp0;
        HeaderR01 = HeaderR00;

        retR01 = retR00;

        float* tmp1 = Rcv10;
        Rcv10 = Rcv11;
        Rcv11 = tmp1;
        unsigned long atmp1 = Rcv10_Address;
        Rcv10_Address = Rcv11_Address;
        Rcv11_Address = atmp1;
        HeaderR11 = HeaderR10;

        retR11 = retR10;

//      std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixR == _ixR-1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
        unsigned long atmp0 = Rcv00_Address;
        Rcv00_Address = Rcv10_Address;
        Rcv10_Address = atmp0;
        HeaderR00 = HeaderR10;

        retR00 =retR10;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
        unsigned long atmp1 = Rcv01_Address;
        Rcv01_Address = Rcv11_Address;
        Rcv11_Address = atmp1;
        HeaderR01 = HeaderR11;

        retR01 =retR11;

//      std::cout << " read 2 Rcvs\n  ";
        retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else if (( ixR == _ixR+1 ) && ( iyR == _iyR))
      {
        float* tmp0 = Rcv00;
        Rcv00 = Rcv10;
        Rcv10 = tmp0;
        unsigned long atmp0 = Rcv00_Address;
        Rcv00_Address = Rcv10_Address;
        Rcv10_Address = atmp0;
        HeaderR10 = HeaderR00;

        retR10 = retR00;

        float* tmp1 = Rcv01;
        Rcv01 = Rcv11;
        Rcv11 = tmp1;
        unsigned long atmp1 = Rcv01_Address;
        Rcv01_Address = Rcv11_Address;
        Rcv11_Address = atmp1;
        HeaderR11 = HeaderR01;

        retR11 = retR01;

//      std::cout << " read 2 Rcvs\n  ";
        retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    else
      {
//      std::cout << " read 4 Rcvs\n  ";
        retR00 = ReadRT( _ixR, _iyR, (float*) Rcv00, Rcv00_Address, HeaderR00, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR10 = ReadRT( _ixR+1, _iyR, (float*) Rcv10, Rcv10_Address, HeaderR10, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR01 = ReadRT( _ixR, _iyR+1, (float*) Rcv01, Rcv01_Address, HeaderR01, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
        retR11 = ReadRT( _ixR+1, _iyR+1, (float*) Rcv11, Rcv11_Address, HeaderR11, N0x, Npartx, N0y, Nparty, mtid, handle_TT, Job);
      }
    ixR = _ixR;
    iyR = _iyR;
  }

  _Src00 = &(Src00[sizeof(SegYHeader)/sizeof(float)]);
  _Src01 = &(Src01[sizeof(SegYHeader)/sizeof(float)]);
  _Src10 = &(Src10[sizeof(SegYHeader)/sizeof(float)]);
  _Src11 = &(Src11[sizeof(SegYHeader)/sizeof(float)]);
  _Rcv00 = &(Rcv00[sizeof(SegYHeader)/sizeof(float)]);
  _Rcv01 = &(Rcv01[sizeof(SegYHeader)/sizeof(float)]);
  _Rcv10 = &(Rcv10[sizeof(SegYHeader)/sizeof(float)]);
  _Rcv11 = &(Rcv11[sizeof(SegYHeader)/sizeof(float)]);

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


/** TTVMMemHandler (const TTVMMemHandler& ) */
TTVMMemHandler::TTVMMemHandler (const TTVMMemHandler& ){
}
/** operator = (const TTVMMemHandler& ) */
TTVMMemHandler& TTVMMemHandler::operator = (const TTVMMemHandler& _RT){
  std::cerr << "TTVMMemHandler operator = should never be called\n";
  return *this;
}


