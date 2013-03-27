#include <TravTimeTab.hpp>

TravTimeTab::TravTimeTab()
    :coeff(NULL)
    ,coeffxy(NULL)
    ,coeffz(NULL)
    ,SrcRcv(NULL)
    ,SrcRcvLocal(NULL)
    ,izfc_vec(NULL)
{

}

TravTimeTab::TravTimeTab(const MigrationJob& MigJob)
    :coeff(NULL)
    ,coeffxy(NULL)
    ,coeffz(NULL)
    ,SrcRcv(NULL)
    ,SrcRcvLocal(NULL)
    ,SrcRcvdx(NULL)
    ,SrcRcvdy(NULL)
    ,izfc_vec(NULL)
    ,pTTData(NULL)
{

  //TTFile.Analyse(MigJob.RTFileName, GVol, GSrc);

  // Daniel Code
  const point3D<float> dxmig(MigJob.MigVol.dx_between_xlines,
			     MigJob.MigVol.dy_between_inlines,
			     MigJob.MigVol.dz);

  strcpy(TTFileBaseName,MigJob.RTFileName);

  // Dirk code

  InitGrids(MigJob);

  ix0 = (int) ((MigJob.MigVol.first_x_coord.v - GVol.getx0()) / MigJob.MigVol.dx_between_xlines);
  //std::cout << "ix0 = " << ix0 << std::endl;
  xfac = static_cast<short>(GVol.getdx()/dxmig[0]);
  if ( xfac * dxmig[0] != GVol.getdx() )
    std::cerr << "TravTimeTab: x-resolutions do not match; " << GVol.getdx() << " is not multiple of " << dxmig[0] << std::endl;

  iy0 = (int) ((MigJob.MigVol.first_y_coord.v - GVol.gety0()) / MigJob.MigVol.dy_between_inlines);
  yfac = static_cast<short>(GVol.getdy()/dxmig[1]);
  if ( yfac * dxmig[1] != GVol.getdy() )
    std::cerr << "TravTimeTab: y-resolutions do not match; " << GVol.getdy() << " is not multiple of " << dxmig[1] << std::endl;

  iz0 = (int) ((MigJob.MigVol.first_z_coord - GVol.getz0()) / MigJob.MigVol.dz);
  //std::cout << "iz0 = " << iz0 << std::endl;
  zfac = static_cast<short>(GVol.getdz()/dxmig[2]);
  if ( zfac * dxmig[2] != GVol.getdz() )
    std::cerr << "TravTimeTab: z-resolutions do not match; " << GVol.getdz() << " is not multiple of " << dxmig[2] << std::endl;
  zfac_inv=1.0/zfac;

  // Allocate the memory for pTTData if necessary and make the
  // required assignments
  MemAlloc(GVol.getNx(), GVol.getNy(), GVol.getNz());

  coeff = new float***[xfac];
  coeffxy = new float**[xfac];
  for (int ix = 0; ix < xfac; ix++)
  {
    coeff[ix] = new float**[yfac];
    coeffxy[ix] = new float*[yfac];
    for (int iy = 0; iy < yfac; iy++)
    {
      coeff[ix][iy] = new float*[8];
      coeffxy[ix][iy] = new float[4];
      for (int ic = 0; ic < 8; ic++)
        coeff[ix][iy][ic] = new float[zfac];
    }
  }
  coeffz = new float*[zfac];
  for (int iz = 0; iz < zfac; iz++)
    {
	coeffz[iz] = new float[2];
    }

}

TravTimeTab::TravTimeTab ( const MigrationJob & MigJob
                         , int NThreads
                         , char * _VMem
                         )
    :TTVMMem(MigJob,NThreads,_VMem)
    ,coeff(NULL)
    ,coeffxy(NULL)
    ,coeffz(NULL)
    ,SrcRcv(NULL)
    ,SrcRcvLocal(NULL)
    ,SrcRcvdx(NULL)
    ,SrcRcvdy(NULL)
    ,izfc_vec(NULL)
    ,pTTData(NULL)

{

  //TTFile.Analyse(MigJob.RTFileName, GVol, GSrc);

  // Daniel Code
  const point3D<float> dxmig(MigJob.MigVol.dx_between_xlines,
			     MigJob.MigVol.dy_between_inlines,
			     MigJob.MigVol.dz);

  strcpy(TTFileBaseName,MigJob.RTFileName);

  // Dirk code

  InitGrids(MigJob);

  ix0 = (int) ((MigJob.MigVol.first_x_coord.v - GVol.getx0()) / MigJob.MigVol.dx_between_xlines);
  //std::cout << "ix0 = " << ix0 << std::endl;
  xfac = static_cast<short>(GVol.getdx()/dxmig[0]);
  if ( xfac * dxmig[0] != GVol.getdx() )
    std::cerr << "TravTimeTab: x-resolutions do not match; " << GVol.getdx() << " is not multiple of " << dxmig[0] << std::endl;

  iy0 = (int) ((MigJob.MigVol.first_y_coord.v - GVol.gety0()) / MigJob.MigVol.dy_between_inlines);
  yfac = static_cast<short>(GVol.getdy()/dxmig[1]);
  if ( yfac * dxmig[1] != GVol.getdy() )
    std::cerr << "TravTimeTab: y-resolutions do not match; " << GVol.getdy() << " is not multiple of " << dxmig[1] << std::endl;

  iz0 = (int) ((MigJob.MigVol.first_z_coord - GVol.getz0()) / MigJob.MigVol.dz);
  //std::cout << "iz0 = " << iz0 << std::endl;
  zfac = static_cast<short>(GVol.getdz()/dxmig[2]);
  if ( zfac * dxmig[2] != GVol.getdz() )
    std::cerr << "TravTimeTab: z-resolutions do not match; " << GVol.getdz() << " is not multiple of " << dxmig[2] << std::endl;
  zfac_inv=1.0/zfac;

  // Allocate the memory for pTTData if necessary and make the
  // required assignments
  MemAlloc(GVol.getNx(), GVol.getNy(), GVol.getNz());

  coeff = new float***[xfac];
  coeffxy = new float**[xfac];
  for (int ix = 0; ix < xfac; ix++)
  {
    coeff[ix] = new float**[yfac];
    coeffxy[ix] = new float*[yfac];
    for (int iy = 0; iy < yfac; iy++)
    {
      coeff[ix][iy] = new float*[8];
      coeffxy[ix][iy] = new float[4];
      for (int ic = 0; ic < 8; ic++)
        coeff[ix][iy][ic] = new float[zfac];
    }
  }
  coeffz = new float*[zfac];
  for (int iz = 0; iz < zfac; iz++)
    {
	coeffz[iz] = new float[2];
    }

}


TravTimeTab::TravTimeTab(volatile char * _pTTData, const MigrationJob& MigJob)
    :coeff(NULL)
    ,coeffxy(NULL)
    ,coeffz(NULL)
    ,SrcRcv(NULL)
    ,SrcRcvLocal(NULL)
    ,SrcRcvdx(NULL)
    ,SrcRcvdy(NULL)
    ,izfc_vec(NULL)
    ,pTTData(_pTTData)
{

  //TTFile.Analyse(MigJob.RTFileName, GVol, GSrc);

  // Daniel Code
  const point3D<float> dxmig(MigJob.MigVol.dx_between_xlines,
			     MigJob.MigVol.dy_between_inlines,
			     MigJob.MigVol.dz);

  strcpy(TTFileBaseName,MigJob.RTFileName);

  // Dirk code

  InitGrids(MigJob);

  ix0 = (int) ((MigJob.MigVol.first_x_coord.v - GVol.getx0()) / MigJob.MigVol.dx_between_xlines);
  //std::cout << "ix0 = " << ix0 << std::endl;
  xfac = static_cast<short>(GVol.getdx()/dxmig[0]);
  if ( xfac * dxmig[0] != GVol.getdx() )
    std::cerr << "TravTimeTab: x-resolutions do not match; " << GVol.getdx() << " is not multiple of " << dxmig[0] << std::endl;

  iy0 = (int) ((MigJob.MigVol.first_y_coord.v - GVol.gety0()) / MigJob.MigVol.dy_between_inlines);
  yfac = static_cast<short>(GVol.getdy()/dxmig[1]);
  if ( yfac * dxmig[1] != GVol.getdy() )
    std::cerr << "TravTimeTab: y-resolutions do not match; " << GVol.getdy() << " is not multiple of " << dxmig[1] << std::endl;

  iz0 = (int) ((MigJob.MigVol.first_z_coord - GVol.getz0()) / MigJob.MigVol.dz);
  //std::cout << "iz0 = " << iz0 << std::endl;
  zfac = static_cast<short>(GVol.getdz()/dxmig[2]);
  if ( zfac * dxmig[2] != GVol.getdz() )
    std::cerr << "TravTimeTab: z-resolutions do not match; " << GVol.getdz() << " is not multiple of " << dxmig[2] << std::endl;
  zfac_inv=1.0/zfac;

  // Allocate the memory for pTTData if necessary and make the
  // required assignments
  MemAlloc(GVol.getNx(), GVol.getNy(), GVol.getNz());

  coeff = new float***[xfac];
  coeffxy = new float**[xfac];
  for (int ix = 0; ix < xfac; ix++)
  {
      coeff[ix] = new float**[yfac];
      coeffxy[ix] = new float*[yfac];
      const float t = ix / (float)xfac;
      for (int iy = 0; iy < yfac; iy++)
      {
          coeff[ix][iy] = new float*[8];
          coeffxy[ix][iy] = new float[4];
	  const float u = iy / (float)yfac;
	  const float u1 = 1-u;

	  coeffxy[ix][iy][0] = (1-t) * u1;
	  coeffxy[ix][iy][1] = (1-t) * u;
	  coeffxy[ix][iy][2] =  t * u1;
	  coeffxy[ix][iy][3] =  t * u;

          for (int ic = 0; ic < 8; ic++)
            coeff[ix][iy][ic] = new float[zfac];

	  for (int iz = 0; iz < zfac; iz++)
	  {
	      const float v = iz / (float)zfac;
	      const float v1 = 1-v;

	      coeff[ix][iy][0][iz] = (1-t) * u1 * v1;
	      coeff[ix][iy][1][iz] = (1-t) * u1 * v;
	      coeff[ix][iy][2][iz] = (1-t) * u * v1;
	      coeff[ix][iy][3][iz] = (1-t) * u * v;
	      coeff[ix][iy][4][iz] =  t * u1 * v1;
	      coeff[ix][iy][5][iz] =  t * u1 * v;
	      coeff[ix][iy][6][iz] =  t * u * v1;
	      coeff[ix][iy][7][iz] =  t * u * v;
	  }
      }
  }

  coeffz = new float*[zfac];
  for (int iz = 0; iz < zfac; iz++)
  {
      coeffz[iz] = new float[2];
      const float v = iz / (float)zfac;
      const float v1 = 1-v;
      coeffz[iz][0] = v1;
      coeffz[iz][1] = v;
  }

}

TravTimeTab::~TravTimeTab()
{
  // clean up the TravTimeTab object

  if (izfc_vec != NULL) delete[] izfc_vec;

  if(MemRqst)
  {
    if ( pTTData != NULL)
    {
      delete[] pTTData;
      SrcRcv = NULL;
      SrcRcvdx = NULL;
      SrcRcvdy = NULL;
      SrcRcvdetH = NULL;
      SrcRcvdetQ = NULL;
      SrcRcvps_plus_pr = NULL;
    }
  }

  if ( SrcRcvLocal != NULL)
    {
      delete[] SrcRcvLocal;
      SrcRcvLocal = NULL;
      delete[] SrcRcvdxLocal;
      SrcRcvdxLocal = NULL;
      delete[] SrcRcvdyLocal;
      SrcRcvdyLocal = NULL;
    }

  if ( coeff != NULL )
    {
      for (int ix = 0; ix < xfac; ix++)
        {
          for (int iy = 0; iy < yfac; iy++)
            {
              for (int ic = 0; ic < 8; ic++)
                delete[] coeff[ix][iy][ic];
              delete[] coeff[ix][iy];
	      delete[] coeffxy[ix][iy];
            }
          delete[] coeff[ix];
	  delete[] coeffxy[ix];
        }
      delete[] coeff;
      delete[] coeffxy;
      coeff = NULL;
      coeffxy = NULL;

      for (int iz = 0; iz < zfac; iz++)
        {
	    delete[] coeffz[iz];
	}
      coeffz = NULL;
    }

};

bool TravTimeTab::LoadTT(TraceData& Trace, MigSubVol3D& SubVol,const MigrationJob &Job,int mtid
                             , const fvmAllocHandle_t handle_TT
)
{
  // This routines loads the traveltime table for the given
  // trace and the given subvolume


  // Transform trace coordinates to CDP/Offset
  // I think this is redundant if one changes the interface
  // of init
  const MOD Mptx = (Trace.getsx() + Trace.getgx())/2.0f;
  const MOD Mpty = (Trace.getsy() + Trace.getgy())/2.0f;

  const MOD Offx = Trace.getgx() - Trace.getsx();
  const MOD Offy = Trace.getgy() - Trace.getsy();

  const int firstxpindex = SubVol.getix0();
  const int Nxpoints 	 = SubVol.getNx();
  const int firstypindex = SubVol.getiy0();
  const int Nypoints     = SubVol.getNy();

  return Init(Job,TTFileBaseName,
              Offx,Offy,Mptx,Mpty,
              firstxpindex,Nxpoints,
              firstypindex,Nypoints,
              mtid
             , handle_TT
);

}

bool TravTimeTab::Init(const MigrationJob &Job,
                       const char* Name, const MOD& Offx, const MOD& Offy,
		       const MOD& Mptx, const MOD& Mpty,const int firstxpindex,
		       const int Nxpoints, const int firstypindex, const int Nypoints,int mtid
                      , const fvmAllocHandle_t handle_TT
)
{

  // Name = file name base for the travel time table files
  // Offx = offset x-coordinate
  // Offy = offset y-coordinate
  // Mptx = cdp x-coordinate
  // Mpty = cdp y-coordinate
  // firstxpindex = first x index of the subvolume on the fine
  //		    lattice
  // Nxpoints     = number of fine lattice points of the subvolume
  //		    in x direction
  // firstypindex = first y index of the subvolume on the fine
  //		    lattice
  // Nypoints     = number of fine lattice points of the subvolume
  //		    in y direction

  // Load the traveltime tables from disk and perform the interpolation
  // to the actual src and receiver position

  const int GVolPart_x0 = (firstxpindex + ix0)/xfac;
  const int GVolPart_x1 = std::min((firstxpindex + Nxpoints + ix0)/xfac + 1, GVol.getNx()-1);
  const int GVolPart_nx = GVolPart_x1 - GVolPart_x0 + 1;

  const int GVolPart_y0 = (firstypindex + iy0)/yfac;
  const int GVolPart_y1 = std::min((firstypindex + Nypoints + iy0)/yfac + 1, GVol.getNy()-1);
  const int GVolPart_ny = GVolPart_y1 - GVolPart_y0 + 1;

  SegYHeader HeaderS00, HeaderS01, HeaderS10, HeaderS11;
  SegYHeader HeaderR00, HeaderR01, HeaderR10, HeaderR11;

  // Determine Source indicees
  const MOD SrcX = Mptx - 0.5*Offx;
  const MOD SrcY = Mpty - 0.5*Offy;

  int nxSrc, nySrc;
  GSrc.GetLowerIndex(SrcX.v, SrcY.v, nxSrc, nySrc);
  if ((nxSrc < 0) || (nySrc < 0)
      || (nxSrc >= GSrc.getNx()-1) || (nySrc >= GSrc.getNy()-1))
    return false;

  //cout<<"nxSrc="<<nxSrc<<endl;
  //cout<<"nySrc="<<nySrc<<endl;

  // Determine Receiver indicees
  const MOD RcvX = Mptx + 0.5*Offx;
  const MOD RcvY = Mpty + 0.5*Offy;

  int nxRcv, nyRcv;
  GSrc.GetLowerIndex(RcvX.v, RcvY.v, nxRcv, nyRcv);
  if ((nxRcv < 0) || (nyRcv < 0)
      || (nxRcv >= GSrc.getNx()-1) || (nyRcv >= GSrc.getNy()-1))
    return false;

  //cout<<"nxRcv="<<nxRcv<<endl;
  //cout<<"nyRcv="<<nyRcv<<endl;

  txSrc = (SrcX.v - (GSrc.getx0() + nxSrc*GSrc.getdx()))/GSrc.getdx();
  tySrc = (SrcY.v - (GSrc.gety0() + nySrc*GSrc.getdy()))/GSrc.getdy();

  txRcv = (RcvX.v - (GSrc.getx0() + nxRcv*GSrc.getdx()))/GSrc.getdx();
  tyRcv = (RcvY.v - (GSrc.gety0() + nyRcv*GSrc.getdy()))/GSrc.getdy();

  txSrc_1 = 1-txSrc;
  tySrc_1 = 1-tySrc;
  txRcv_1 = 1-txRcv;
  tyRcv_1 = 1-tyRcv;

  const float tdxSrc = 1.0f/GSrc.getdx();
  const float tdySrc = 1.0f/GSrc.getdy();

  const float tdxRcv = 1.0f/GSrc.getdx();
  const float tdyRcv = 1.0f/GSrc.getdy();

  const float tdxSrc_1 = -1.0f/GSrc.getdx();
  const float tdySrc_1 = -1.0f/GSrc.getdy();

  const float tdxRcv_1 = -1.0f/GSrc.getdx();
  const float tdyRcv_1 = -1.0f/GSrc.getdy();

  // Read Source and Receiver TT-files

//   std::cout << "mtid " << mtid << " TTVMEM.Init(...)" << std::endl;

  if (!TTVMMem.Init(Name, nxSrc, nySrc, nxRcv, nyRcv,
  		      Src00, Src01, Src10, Src11,
  		      Rcv00, Rcv01, Rcv10, Rcv11,
  		      HeaderS00, HeaderS01, HeaderS10, HeaderS11,
  		      HeaderR00, HeaderR01, HeaderR10, HeaderR11,
                     GVolPart_x0, GVolPart_nx, GVolPart_y0, GVolPart_ny, mtid, handle_TT
                                              , Job
))
      return false;

  double sx, sy, gx, gy;
    GSrc.GetCoord(nxSrc, nySrc, sx, sy);
    GSrc.GetCoord(nxRcv, nyRcv, gx, gy);

    const int NyVol = GVol.getNy();
    const int NzVol = GVol.getNz();

    int SRindex = 0;

    for (int ix = GVolPart_x0; ix < GVolPart_x0 + GVolPart_nx; ix++)
    {
      SRindex = (ix*NyVol + GVolPart_y0) * NzVol;
      for (int iy = GVolPart_y0; iy < GVolPart_y0 + GVolPart_ny; iy++)
      {
        for (int iz = 0; iz < NzVol; iz++)
        {
          if ( (Src00[SRindex*N_SIGNALS + 0] >= 0) && (Src01[SRindex*N_SIGNALS + 0] >= 0) && (Src10[SRindex*N_SIGNALS + 0] >= 0) && (Src11[SRindex*N_SIGNALS + 0] >= 0) &&
             (Rcv00[SRindex*N_SIGNALS + 0] >= 0) && (Rcv01[SRindex*N_SIGNALS + 0] >= 0) && (Rcv10[SRindex*N_SIGNALS + 0] >= 0) && (Rcv11[SRindex*N_SIGNALS + 0] >= 0) )
          {
            SrcRcv[SRindex] = txSrc_1*( tySrc_1*Src00[SRindex*N_SIGNALS + 0] + tySrc*Src01[SRindex*N_SIGNALS + 0] )
              + txSrc*( tySrc_1*Src10[SRindex*N_SIGNALS + 0] + tySrc*Src11[SRindex*N_SIGNALS + 0])
              + txRcv_1*( tyRcv_1*Rcv00[SRindex*N_SIGNALS + 0] + tyRcv*Rcv01[SRindex*N_SIGNALS + 0] )
              + txRcv*( tyRcv_1*Rcv10[SRindex*N_SIGNALS + 0] + tyRcv*Rcv11[SRindex*N_SIGNALS + 0] );
            SrcRcvdx[SRindex] = tdxSrc_1*( tySrc_1*Src00[SRindex*N_SIGNALS + 0] + tySrc*Src01[SRindex*N_SIGNALS + 0] ) + tdxSrc*( tySrc_1*Src10[SRindex*N_SIGNALS + 0] + tySrc*Src11[SRindex*N_SIGNALS + 0])
              + tdxRcv_1*( tyRcv_1*Rcv00[SRindex*N_SIGNALS + 0] + tyRcv*Rcv01[SRindex*N_SIGNALS + 0] ) + tdxRcv*( tyRcv_1*Rcv10[SRindex*N_SIGNALS + 0] + tyRcv*Rcv11[SRindex*N_SIGNALS + 0] );

            SrcRcvdy[SRindex] = txSrc_1*( tdySrc_1*Src00[SRindex*N_SIGNALS + 0] + tdySrc*Src01[SRindex*N_SIGNALS + 0] ) + txSrc*( tdySrc_1*Src10[SRindex*N_SIGNALS + 0] + tdySrc*Src11[SRindex*N_SIGNALS + 0])
              + txRcv_1*( tdyRcv_1*Rcv00[SRindex*N_SIGNALS + 0] + tdyRcv*Rcv01[SRindex*N_SIGNALS + 0] ) + txRcv*( tdyRcv_1*Rcv10[SRindex*N_SIGNALS + 0] + tdyRcv*Rcv11[SRindex*N_SIGNALS + 0] );

          }
          else
          {
            SrcRcv[SRindex] = -1000;
            SrcRcvdx[SRindex] = -1000;
            SrcRcvdy[SRindex] = -1000;
          }
          SRindex++;
        }

      }
    }

    for (int ix = 0; ix < xfac; ix++)
    {
      const float t = ix / (float)xfac;
      for (int iy = 0; iy < yfac; iy++)
      {
        const float u = iy / (float)yfac;
        const float u1 = 1-u;

        coeffxy[ix][iy][0] = (1-t) * u1;
        coeffxy[ix][iy][1] = (1-t) * u;
        coeffxy[ix][iy][2] =  t * u1;
        coeffxy[ix][iy][3] =  t * u;

        for (int iz = 0; iz < zfac; iz++)
        {
          const float v = iz / (float)zfac;
          const float v1 = 1-v;

          coeff[ix][iy][0][iz] = (1-t) * u1 * v1;
          coeff[ix][iy][1][iz] = (1-t) * u1 * v;
          coeff[ix][iy][2][iz] = (1-t) * u * v1;
          coeff[ix][iy][3][iz] = (1-t) * u * v;
          coeff[ix][iy][4][iz] =  t * u1 * v1;
          coeff[ix][iy][5][iz] =  t * u1 * v;
          coeff[ix][iy][6][iz] =  t * u * v1;
          coeff[ix][iy][7][iz] =  t * u * v;
        }
      }
    }
    for (int iz = 0; iz < zfac; iz++)
    {
      const float v = iz / (float)zfac;
      const float v1 = 1-v;
      coeffz[iz][0] = v1;
      coeffz[iz][1] = v;
    }
    return true;
};


void TravTimeTab::Init_ix(const int& ix)
{
  ixf = (ix+ix0)/xfac;
  ixc = (ix+ix0) % xfac;
  //std::cout << "x: " << ix0 << " " << ixf << " " << ixc << std::endl;
}

void TravTimeTab::Init_iy(const int& iy)
{

  iyf = (iy+iy0)/yfac;
  iyc = (iy+iy0) % yfac;
  const int NyVol = GVol.getNy();
  const int NzVol = GVol.getNz();
  TT00 = &SrcRcv[(ixf*NyVol + iyf) * NzVol];
  TT01 = &SrcRcv[(ixf*NyVol + iyf+1) * NzVol];
  TT10 = &SrcRcv[((ixf+1)*NyVol + iyf) * NzVol];
  TT11 = &SrcRcv[((ixf+1)*NyVol + iyf+1) * NzVol];

  TT00dx = &SrcRcvdx[(ixf*NyVol + iyf) * NzVol];
  TT01dx = &SrcRcvdx[(ixf*NyVol + iyf+1) * NzVol];
  TT10dx = &SrcRcvdx[((ixf+1)*NyVol + iyf) * NzVol];
  TT11dx = &SrcRcvdx[((ixf+1)*NyVol + iyf+1) * NzVol];

  TT00dy = &SrcRcvdy[(ixf*NyVol + iyf) * NzVol];
  TT01dy = &SrcRcvdy[(ixf*NyVol + iyf+1) * NzVol];
  TT10dy = &SrcRcvdy[((ixf+1)*NyVol + iyf) * NzVol];
  TT11dy = &SrcRcvdy[((ixf+1)*NyVol + iyf+1) * NzVol];

  detH00 = &SrcRcvdetH[(ixf*NyVol + iyf) * NzVol];
  detH01 = &SrcRcvdetH[(ixf*NyVol + iyf+1) * NzVol];
  detH10 = &SrcRcvdetH[((ixf+1)*NyVol + iyf) * NzVol];
  detH11 = &SrcRcvdetH[((ixf+1)*NyVol + iyf+1) * NzVol];

  detQ00 = &SrcRcvdetQ[(ixf*NyVol + iyf) * NzVol];
  detQ01 = &SrcRcvdetQ[(ixf*NyVol + iyf+1) * NzVol];
  detQ10 = &SrcRcvdetQ[((ixf+1)*NyVol + iyf) * NzVol];
  detQ11 = &SrcRcvdetQ[((ixf+1)*NyVol + iyf+1) * NzVol];

  ps_plus_pr00 = &SrcRcvps_plus_pr[(ixf*NyVol + iyf) * NzVol];
  ps_plus_pr01 = &SrcRcvps_plus_pr[(ixf*NyVol + iyf+1) * NzVol];
  ps_plus_pr10 = &SrcRcvps_plus_pr[((ixf+1)*NyVol + iyf) * NzVol];
  ps_plus_pr11 = &SrcRcvps_plus_pr[((ixf+1)*NyVol + iyf+1) * NzVol];

}

void TravTimeTab::Init_iz(const int& Nz)
{
  if (izfc_vec == NULL)
    izfc_vec = new short[2*Nz-1];
  int izc = -1;
  int izf = 0;
  for (int iz = 0; iz < Nz; iz++)
  {
    izc++;
    if (izc == zfac)
    {
      izf++;
      izc = 0;
    }
    izfc_vec[2*iz] = izf;
    izfc_vec[2*iz+1] = izc;
  }
}

void TravTimeTab::GetTT(float* T, const int& Nz )
{
  float** ctmp = coeff[ixc][iyc];
  int index = 0;
  int iza = iz0/zfac;
  int ize = (Nz+iz0)/zfac;
  for (int izf = iza; izf < ize; izf++)
  {
    y[0] = TT00[izf];
    y[1] = TT00[izf+1];
    y[2] = TT01[izf];
    y[3] = TT01[izf+1];
    y[4] = TT10[izf];
    y[5] = TT10[izf+1];
    y[6] = TT11[izf];
    y[7] = TT11[izf+1];

    for (int  i=0; i < zfac; i++)
      {
        T[index+i] = ctmp[0][i] * y[0] + ctmp[1][i] * y[1] + ctmp[2][i] * y[2] + ctmp[3][i] * y[3] +
          ctmp[4][i] * y[4] + ctmp[5][i] * y[5] + ctmp[6][i] * y[6] + ctmp[7][i] * y[7];
      }
    index+=zfac;
  }

  for (int  i=0; i < Nz-index; i++)
  {
      y[0] = TT00[ize];
      y[1] = TT00[ize+1];
      y[2] = TT01[ize];
      y[3] = TT01[ize+1];
      y[4] = TT10[ize];
      y[5] = TT10[ize+1];
      y[6] = TT11[ize];
      y[7] = TT11[ize+1];

      T[index+i] = ctmp[0][i] * y[0] + ctmp[1][i] * y[1] + ctmp[2][i] * y[2] + ctmp[3][i] * y[3] +
          ctmp[4][i] * y[4] + ctmp[5][i] * y[5] + ctmp[6][i] * y[6] + ctmp[7][i] * y[7];
  }

}

void TravTimeTab::GetTT(float* T, float* dTdx, float* dTdy, const int& Nz )
{
  int index = 0;
  int iza = iz0/zfac;
  int ize = (Nz+iz0)/zfac;

  float* Tval = new float[ize+2-iza];
  float* dTdxval = new float[ize+2-iza];
  float* dTdyval = new float[ize+2-iza];
  for (int izf = iza; izf < ize+2; izf++)
  {
    y[0] = TT00[izf];
    y[1] = TT00[izf+1];
    y[2] = TT01[izf];
    y[3] = TT01[izf+1];
    y[4] = TT10[izf];
    y[5] = TT10[izf+1];
    y[6] = TT11[izf];
    y[7] = TT11[izf+1];

    dtdx[0] = TT00dx[izf];
    dtdx[1] = TT00dx[izf+1];
    dtdx[2] = TT01dx[izf];
    dtdx[3] = TT01dx[izf+1];
    dtdx[4] = TT10dx[izf];
    dtdx[5] = TT10dx[izf+1];
    dtdx[6] = TT11dx[izf];
    dtdx[7] = TT11dx[izf+1];

    dtdy[0] = TT00dy[izf];
    dtdy[1] = TT00dy[izf+1];
    dtdy[2] = TT01dy[izf];
    dtdy[3] = TT01dy[izf+1];
    dtdy[4] = TT10dy[izf];
    dtdy[5] = TT10dy[izf+1];
    dtdy[6] = TT11dy[izf];
    dtdy[7] = TT11dy[izf+1];

    Tval[izf-iza] = coeffxy[ixc][iyc][0] * y[0] + coeffxy[ixc][iyc][1] * y[2] + coeffxy[ixc][iyc][2] * y[4] + coeffxy[ixc][iyc][3] * y[6];

    dTdxval[izf-iza] = coeffxy[ixc][iyc][0] * dtdx[0] + coeffxy[ixc][iyc][1] * dtdx[2] + coeffxy[ixc][iyc][2] * dtdx[4] + coeffxy[ixc][iyc][3] * dtdx[6];

    dTdyval[izf-iza] = coeffxy[ixc][iyc][0] * dtdy[0] + coeffxy[ixc][iyc][1] * dtdy[2] + coeffxy[ixc][iyc][2] * dtdy[4] + coeffxy[ixc][iyc][3] * dtdy[6];
  }

  for (int izf = 0; izf < ize-iza; izf++)
  {
    for (int  i=0; i < zfac; i++)
      {
        T[index+i] = coeffz[i][0] * Tval[izf] + coeffz[i][1] * Tval[izf+1];
        dTdx[index+i] = coeffz[i][0] * dTdxval[izf] + coeffz[i][1] * dTdxval[izf+1];
        dTdy[index+i] = coeffz[i][0] * dTdyval[izf] + coeffz[i][1] * dTdyval[izf+1];
      }
    index+=zfac;
  }

  for (int  i=0; i < Nz-index; i++)
  {
        T[index+i] = coeffz[i][0] * Tval[ize-iza] + coeffz[i][1] * Tval[ize-iza+1];
        dTdx[index+i] = coeffz[i][0] * dTdxval[ize-iza] + coeffz[i][1] * dTdxval[ize-iza+1];
        dTdy[index+i] = coeffz[i][0] * dTdyval[ize-iza] + coeffz[i][1] * dTdyval[ize-iza+1];
  }

  delete[] Tval;
  delete[] dTdxval;
  delete[] dTdyval;

}

void TravTimeTab::GetTT(float* T, float* dTdx, float* dTdy, float* ps_plus_pr,
			float* detQ, float* detH, const int& Nz )
{
  int index = 0;
  int iza = iz0/zfac;
  int ize = (Nz+iz0)/zfac;

  float* Tval = new float[ize+2-iza];
  float* dTdxval = new float[ize+2-iza];
  float* dTdyval = new float[ize+2-iza];
  float* detHval = new float[ize+2-iza];
  float* detQval = new float[ize+2-iza];
  float* ps_plus_prval = new float[ize+2-iza];
  for (int izf = iza; izf < ize+2; izf++)
  {
    y[0] = TT00[izf];
    y[1] = TT00[izf+1];
    y[2] = TT01[izf];
    y[3] = TT01[izf+1];
    y[4] = TT10[izf];
    y[5] = TT10[izf+1];
    y[6] = TT11[izf];
    y[7] = TT11[izf+1];

    dtdx[0] = TT00dx[izf];
    dtdx[1] = TT00dx[izf+1];
    dtdx[2] = TT01dx[izf];
    dtdx[3] = TT01dx[izf+1];
    dtdx[4] = TT10dx[izf];
    dtdx[5] = TT10dx[izf+1];
    dtdx[6] = TT11dx[izf];
    dtdx[7] = TT11dx[izf+1];

    dtdy[0] = TT00dy[izf];
    dtdy[1] = TT00dy[izf+1];
    dtdy[2] = TT01dy[izf];
    dtdy[3] = TT01dy[izf+1];
    dtdy[4] = TT10dy[izf];
    dtdy[5] = TT10dy[izf+1];
    dtdy[6] = TT11dy[izf];
    dtdy[7] = TT11dy[izf+1];

    detHy[0] = detH00[izf];
    detHy[1] = detH00[izf+1];
    detHy[2] = detH01[izf];
    detHy[3] = detH01[izf+1];
    detHy[4] = detH10[izf];
    detHy[5] = detH10[izf+1];
    detHy[6] = detH11[izf];
    detHy[7] = detH11[izf+1];

    detQy[0] = detQ00[izf];
    detQy[1] = detQ00[izf+1];
    detQy[2] = detQ01[izf];
    detQy[3] = detQ01[izf+1];
    detQy[4] = detQ10[izf];
    detQy[5] = detQ10[izf+1];
    detQy[6] = detQ11[izf];
    detQy[7] = detQ11[izf+1];

    ps_plus_pry[0] = ps_plus_pr00[izf];
    ps_plus_pry[1] = ps_plus_pr00[izf+1];
    ps_plus_pry[2] = ps_plus_pr01[izf];
    ps_plus_pry[3] = ps_plus_pr01[izf+1];
    ps_plus_pry[4] = ps_plus_pr10[izf];
    ps_plus_pry[5] = ps_plus_pr10[izf+1];
    ps_plus_pry[6] = ps_plus_pr11[izf];
    ps_plus_pry[7] = ps_plus_pr11[izf+1];

    Tval[izf-iza] = coeffxy[ixc][iyc][0] * y[0] + coeffxy[ixc][iyc][1] * y[2] + coeffxy[ixc][iyc][2] * y[4] + coeffxy[ixc][iyc][3] * y[6];

    dTdxval[izf-iza] = coeffxy[ixc][iyc][0] * dtdx[0] + coeffxy[ixc][iyc][1] * dtdx[2] + coeffxy[ixc][iyc][2] * dtdx[4] + coeffxy[ixc][iyc][3] * dtdx[6];

    dTdyval[izf-iza] = coeffxy[ixc][iyc][0] * dtdy[0] + coeffxy[ixc][iyc][1] * dtdy[2] + coeffxy[ixc][iyc][2] * dtdy[4] + coeffxy[ixc][iyc][3] * dtdy[6];

    detHval[izf-iza] = coeffxy[ixc][iyc][0] * detHy[0] + coeffxy[ixc][iyc][1] * detHy[2] + coeffxy[ixc][iyc][2] * detHy[4] + coeffxy[ixc][iyc][3] * detHy[6];

    detQval[izf-iza] = coeffxy[ixc][iyc][0] * detQy[0] + coeffxy[ixc][iyc][1] * detQy[2] + coeffxy[ixc][iyc][2] * detQy[4] + coeffxy[ixc][iyc][3] * detQy[6];

    ps_plus_prval[izf-iza] = coeffxy[ixc][iyc][0] * ps_plus_pry[0] + coeffxy[ixc][iyc][1] * ps_plus_pry[2] + coeffxy[ixc][iyc][2] * ps_plus_pry[4] + coeffxy[ixc][iyc][3] * ps_plus_pry[6];
  }

  for (int izf = 0; izf < ize-iza; izf++)
  {
    for (int  i=0; i < zfac; i++)
      {
        T[index+i] = coeffz[i][0] * Tval[izf] + coeffz[i][1] * Tval[izf+1];
        dTdx[index+i] = coeffz[i][0] * dTdxval[izf] + coeffz[i][1] * dTdxval[izf+1];
        dTdy[index+i] = coeffz[i][0] * dTdyval[izf] + coeffz[i][1] * dTdyval[izf+1];
        detH[index+i] = coeffz[i][0] * detHval[izf] + coeffz[i][1] * detHval[izf+1];
        detQ[index+i] = coeffz[i][0] * detQval[izf] + coeffz[i][1] * detQval[izf+1];
        ps_plus_pr[index+i] = coeffz[i][0] * ps_plus_prval[izf] + coeffz[i][1] * ps_plus_prval[izf+1];
      }
    index+=zfac;
  }


  for (int  i=0; i < Nz-index; i++)
  {
        T[index+i] = coeffz[i][0] * Tval[ize-iza] + coeffz[i][1] * Tval[ize-iza+1];
        dTdx[index+i] = coeffz[i][0] * dTdxval[ize-iza] + coeffz[i][1] * dTdxval[ize-iza+1];
        dTdy[index+i] = coeffz[i][0] * dTdyval[ize-iza] + coeffz[i][1] * dTdyval[ize-iza+1];
        detH[index+i] = coeffz[i][0] * detHval[ize-iza] + coeffz[i][1] * detHval[ize-iza+1];
        detQ[index+i] = coeffz[i][0] * detQval[ize-iza] + coeffz[i][1] * detQval[ize-iza+1];
        ps_plus_pr[index+i] = coeffz[i][0] * ps_plus_prval[ize-iza] + coeffz[i][1] * ps_plus_prval[ize-iza+1];
  }

  delete[] Tval;
  delete[] dTdxval;
  delete[] dTdyval;
  delete[] detHval;
  delete[] detQval;
  delete[] ps_plus_prval;
}

void TravTimeTab::Reset()
{
  //TTVMMem.Reset();
}

volatile char * TravTimeTab::getMemPtr()
{
  return pTTData;
}

void TravTimeTab::SetMemPtr(volatile char * _pTTData)
{
  if(pTTData!=NULL)
  {
    pTTData=_pTTData;
    MemAlloc(GVol.getNx(), GVol.getNy(), GVol.getNz());
  }
}

///////////////////////////////////////////////////////////////////////////////////
// 				private methods:				 //
///////////////////////////////////////////////////////////////////////////////////

void TravTimeTab::InitGrids(const MigrationJob& MigJob)
{
  // Read the surface grid of sources, the runtime has been calculated for
  double x0Srfc = MigJob.SrfcGridX0.v;
  double y0Srfc = MigJob.SrfcGridY0.v;

  double dxSrfc = MigJob.SrfcGriddx;
  double dySrfc = MigJob.SrfcGriddy;

  int NxSrc = MigJob.SrfcGridNx;
  int NySrc = MigJob.SrfcGridNy;

  GSrc.Init(x0Srfc, y0Srfc, NxSrc, NySrc, dxSrfc, dySrfc);

  // Read the sub-surface grid
  int Nx = MigJob.TTVol.nx_xlines;
  int Ny = MigJob.TTVol.ny_inlines;
  int Nz = MigJob.TTVol.nz;

  point3D<float> X0, dx;
  X0[0] = MigJob.TTVol.first_x_coord.v;
  X0[1] = MigJob.TTVol.first_y_coord.v;
  X0[2] = MigJob.TTVol.first_z_coord;
  dx[0] = MigJob.TTVol.dx_between_xlines;
  dx[1] = MigJob.TTVol.dy_between_inlines;
  dx[2] = MigJob.TTVol.dz;

  GVol.Init(X0, point3D<int>(Nx,Ny,Nz), dx);
}

void TravTimeTab::MemAlloc(const int Nx, const int Ny, const int Nz)
{
  if(pTTData==NULL)
  {
    pTTData=new char[6*Nx*Ny*Nz*sizeof(float)];
    MemRqst=true;
  }
  else
  {
    MemRqst=false;
  }

  SrcRcv = (float *) pTTData;
  SrcRcvdx = SrcRcv+Nx*Ny*Nz;
  SrcRcvdy = SrcRcvdx+Nx*Ny*Nz;

  SrcRcvdetH = SrcRcvdy+Nx*Ny*Nz;

  SrcRcvdetQ = SrcRcvdetH+Nx*Ny*Nz;

  SrcRcvps_plus_pr = SrcRcvdetQ+Nx*Ny*Nz;;

}

