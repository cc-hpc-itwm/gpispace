#include <sdpa_migrate.hpp>
#include "sinc_mod.hpp"

// bool SDPA_mig_and_pref( int _oid
//                       , int _pid2mig
//                       , int _bid2mig
//                       , int _pid2load
//                       , int _bid2load
//                       , int actbuf
//                       , MigrationJob &_Job
//                       , SincInterpolator ** _SincIntA
//                       , int NThreads
//                       )
// {
//   const int VMRank=_PE.GetRank();

//   char * pVMMemglob  =(char *)_PE.getMemPtr()+_Job.globbufoffset;
//   char * pVMMemloc1  =(char *)_PE.getMemPtr()+_Job.locbuf1off;
//   char * pVMMemloc2  =(char *)_PE.getMemPtr()+_Job.locbuf2off;
//   char * pVMMemSubVol=(char *)_PE.getMemPtr()+_Job.SubVolMemOff;


//   // determine prefetch and migration buffer offset
//   char *migbuf;
//   int prebufoff;

//   if (actbuf==1) prebufoff=_Job.locbuf2off;
//   else  	 prebufoff=_Job.locbuf1off;
//   if (actbuf==1) migbuf=pVMMemloc1;
//   else 		 migbuf=pVMMemloc2;

//   /// 1.) prefetch the trace bunch
//   if( _pid2load != 0 && _bid2load != 0 )
//   {
//     // determine the global offset and the machine on which the
//     // trace bunch to be loaded resists;
//     int relgloboff,VMRank2Load;
//     SDPA_detbunchloc(_oid,_pid2load,_bid2load,_Job,_PE,relgloboff,VMRank2Load);

//     // copy the tracebunch into the local buffer
//     //memcpy((void *)prebuf,(void *)(pVMMemglob+relgloboff),_Job.BunchMemSize);
//     _PE.readMem(prebufoff,_Job.globbufoffset+relgloboff,_Job.BunchMemSize,VMRank2Load,VMQueue7);
//   }

//   /// 2.) migrate the other bunch
//   if ( _pid2mig != 0  && _bid2mig != 0 )
//   {
//     // Reconstruct the subvolume out of memory
//     point3D<float> X0(_Job.MigVol.first_x_coord.v,
// 	   	      _Job.MigVol.first_y_coord.v,
//                       _Job.MigVol.first_z_coord);
//     point3D<int>   Nx(_Job.MigVol.nx_xlines,
//    		      _Job.MigVol.ny_inlines,
//                       _Job.MigVol.nz);
//     point3D<float> dx(_Job.MigVol.dx_between_xlines,
//    	              _Job.MigVol.dy_between_inlines,
//                       _Job.MigVol.dz);
//     MigVol3D MigVol(X0,Nx,dx);

//     // create the subvolume
//     MigSubVol3D MigSubVol(MigVol,(VMRank%_Job.NSubVols)+1,_Job.NSubVols);

//     // Attach the mem ptr to the subvolume
//     MigSubVol.setMemPtr((float *)pVMMemSubVol,_Job.SubVolMemSize);

//     // Reconstruct the tracebunch out of memory
//     TraceBunch Bunch(migbuf,_oid,_pid2mig,_bid2mig,_Job);

//     // migrate the bunch to the subvolume
//     MigBunch2SubVol(_Job,_PE,Bunch,MigSubVol,_SincIntA,NThreads);
//   }

//   /// 3.) Wait until the data is read
//   if( _pid2load != 0 && _bid2load != 0 )
//   {
//     _PE.WaitOnQueue(VMQueue7);
//   }

//   return true;

// }

bool MigBunch2SubVol(const MigrationJob &Job, TraceBunch &Bunch,
		     MigSubVol3D &SubVol,int NThreads
                    , char * _VMem
                    , const fvmAllocHandle_t handle_TT
                    )
{
  // MigrationJob 	= Job description of the migration job
  // TraceBunch		= Bunch of traces to be migrated
  // MigSubVol3D	= Subvolume to be migrated to

  // define the number of threads to be used
  const int Ntid = min(NThreads,SubVol.getNx());
  pthread_t threads[Ntid];
  thread_param_t thread_params[Ntid];

  resizeSincInterpolatorArray (Ntid);

  for(int mtid=0;mtid<Ntid;mtid++)
  {
    thread_params[mtid].Job      = &Job;
    thread_params[mtid].Bunch    = &Bunch;
    thread_params[mtid].SubVol   = &SubVol;
    thread_params[mtid].SincInt  = NULL; // getSincInterpolator (mtid, Job.tracedt);
    thread_params[mtid].Ntid     = Ntid;
    thread_params[mtid].mtid     = mtid;
    thread_params[mtid].NThreads = NThreads;
    thread_params[mtid]._VMem    = _VMem;
    thread_params[mtid].handle_TT = handle_TT;

    int rc=pthread_create(&threads[mtid],NULL,
                          MigBunch2SubVol_ST, (void *) &thread_params[mtid]);

    if (rc)
      printf("failed to create a thread!\n");
  }

//   std::cout << "MigBunch2SubVol: threads created " << std::endl;

  for(int mtid=0;mtid<Ntid;mtid++)
  {
    int rc=pthread_join(threads[mtid],NULL);
    if (rc)
      printf("failed to join a thread\n");
  }

//   std::cout << "MigBunch2SubVol: joined" << std::endl;

  return true;

}

// bool MigBunch2SubVol2(const MigrationJob &Job, TraceBunch &Bunch,
// 		      MigSubVol3D &SubVol,SincInterpolator &SincInt)
//  {

//   // MigrationJob 	= Job description of the migration job
//   // TraceBunch		= Bunch of traces to be migrated
//   // MigSubVol3D	= Subvolume to be migrated to

//   // define the number of threads to be used
//   const int MaxNThreads=1;
//   const int NThreads = min(MaxNThreads,SubVol.getNx());
//   pthread_t threads[NThreads];
//   thread_param2_t thread_params[NThreads];

//   TravTimeTab TT(Job,PE);

//   const int NTB=Bunch.getNTB();

//   for(int i=0;i<NTB;i++)
//   {

//      // get the trace out of the bunch
//      TraceData* Trace = Bunch.getTrace(i);

//      // load the traveltime table for the given subvolume
//      // and the given trace from disk
//      TT.LoadTT(*Trace,SubVol,Job);

//      volatile char * TTData=TT.getMemPtr();

//      for(int mtid=0;mtid<NThreads;mtid++)
//      {
//        thread_params[mtid].Job    = &Job;
//        thread_params[mtid].Trace  = Trace;
//        thread_params[mtid].TTData = TTData;
//        thread_params[mtid].SubVol = &SubVol;
//        thread_params[mtid].SincInt= &SincInt;
//        thread_params[mtid].Ntid   = NThreads;
//        thread_params[mtid].mtid   = mtid;

//        int rc=pthread_create(&threads[mtid],NULL,
//                              MigTrace2SubVol_ST, (void *) &thread_params[mtid]);

//        if (rc)
//          printf("failed to create a thread!\n");
//      }

//      for(int mtid=0;mtid<NThreads;mtid++)
//      {
//        int rc=pthread_join(threads[mtid],NULL);
//        if (rc)
//          printf("failed to join a thread\n");
//      }


//    } // end loop over all traces in bunch

//   return true;

// }

void *MigBunch2SubVol_ST(void *_param)
{

  thread_param_t * thread_param = (thread_param_t *)_param;

  const int NTB=thread_param->Bunch->getNTB();
  const int Ntid=thread_param->Ntid;
  const int mtid=thread_param->mtid;
  const int NThreads=thread_param->NThreads;

//   std::cout<<"mtid="<<mtid<<std::endl;

   // Generate subvolume to migrate to
   MigSubVol3D SubVol(*(thread_param->SubVol),mtid+1,Ntid);

   // loop over all traces in bunch
   // each threads starts at a different position

   int traceidx=(NTB/Ntid)*mtid;

   //cout<<"The trace idx is given by:"<<traceidx<<endl;

   TravTimeTab TT(*(thread_param->Job),NThreads, thread_param->_VMem);

//    SincInterpolator SincInt;
//
//    //std::cout<<"The dtbin is given by "<<thread_param->Bunch->getTrace(0)->getdtbin()<<std::endl;
//
//    SincInt.init(thread_param->Bunch->getTrace(0)->getdtbin());

   //thread_param->Bunch->getTrace(0)->Dump();

   thread_param->SincInt = getSincInterpolator ( mtid
					       , thread_param->Job->tracedt
					       );

   for(int i=0;i<NTB;i++)
   {
//      std::cout << "mtid " << mtid
//                << " i " << i << " of " << NTB
//                << std::endl;


     // get the trace out of the bunch
     //TraceData* Trace = thread_param->Bunch->getTrace(traceidx);
     volatile char * pTraceData = thread_param->Bunch->getTrace(traceidx)->getDataPtr();
     TraceData Trace(pTraceData,thread_param->Bunch->getTrace(traceidx)->getNt());

     // load the traveltime table for the given subvolume
     // and the given trace from disk
     if(TT.LoadTT(Trace,SubVol,*(thread_param->Job),mtid, thread_param->handle_TT))
     {
       // migrate the trace to the subvolume
       MigTrace2SubVol(*(thread_param->Job),Trace,TT,SubVol,*(thread_param->SincInt));
     }

     traceidx++;

     if(traceidx==NTB) traceidx=0;

     // reset the traveltime table
     TT.Reset();

   } // end loop over all traces in bunch

//    std::cout << "mtid DONE " << mtid
//              << std::endl;


   return NULL;

}

// void *MigBunch2SubVol_Test_ST(void *_param)
// {

//   thread_param_t * thread_param = (thread_param_t *)_param;
//   pthread_t thread;
//   thread_param_load_t thread_params_load;

//   const int NTB=thread_param->Bunch->getNTB();
//   const int Ntid=thread_param->Ntid;
//   const int mtid=thread_param->mtid;
//   const int NThreads=thread_param->NThreads;

//    //std::cout<<"mtid="<<mtid<<std::endl;

//    // Generate subvolume to migrate to
//    MigSubVol3D SubVol(*(thread_param->SubVol),mtid+1,Ntid);

//    // loop over all traces in bunch
//    // each threads starts at a different position

//    TravTimeTab TT1(*(thread_param->Job),*(thread_param->PE),NThreads);
//    TravTimeTab TT2(*(thread_param->Job),*(thread_param->PE),NThreads);
//    TravTimeTab * TT2mig;
//    TravTimeTab * TT2load;

//    TraceData* Trace2mig;
//    TraceData* Trace2load;

//    // get the trace out of the bunch
//    int nexttraceidx=(NTB/Ntid)*mtid;
//    Trace2mig = thread_param->Bunch->getTrace(nexttraceidx);
//    nexttraceidx++;
//    if(nexttraceidx==NTB) nexttraceidx=0;
//    Trace2load= thread_param->Bunch->getTrace(nexttraceidx);

//    TT1.LoadTT(*Trace2mig,SubVol,*(thread_param->Job),mtid);

//    TT2mig=&TT1;
//    TT2load=&TT2;

//    for(int i=0;i<NTB;i++)
//    {

//      // Fork the thread;
//      thread_params_load.Job = thread_param->Job;
//      thread_params_load.Trace = Trace2load;
//      thread_params_load.TT = TT2load;
//      thread_params_load.SubVol = &SubVol;
//      thread_params_load.mtid=mtid;
//      {
//        int rc=pthread_create(&thread,NULL,
//                               LoadTT_ST, (void *) &thread_params_load);

//        if (rc)
//          printf("failed to create TT load thread!\n");
//      }

//      // migrate the trace to the subvolume
//      MigTrace2SubVol(*(thread_param->Job),*Trace2mig,*TT2mig,SubVol,*(thread_param->SincInt));

//      // Join the thread
//      {
//        int rc=pthread_join(thread,NULL);
//        if (rc)
//            printf("failed to join a thread\n");
//      }

//      // Prepare Data for the next TT
//      nexttraceidx++;
//      if(nexttraceidx==NTB) nexttraceidx=0;

//      Trace2mig=Trace2load;
//      Trace2load=thread_param->Bunch->getTrace(nexttraceidx);

//      TT2mig=TT2load;
//      if(i%2==0)
//        TT2load=&TT1;
//      else
//        TT2load=&TT2;

//    } // end loop over all traces in bunch

//    return NULL;

// }

// void *LoadTT_ST(void *_param)
// {

//   thread_param_load_t * thread_param = (thread_param_load_t *)_param;


//   thread_param->TT->LoadTT(*(thread_param->Trace),*(thread_param->SubVol),*(thread_param->Job),thread_param->mtid);

//   return NULL;

// }

void *MigTrace2SubVol_ST(void *_param)
{

  thread_param2_t * thread_param = (thread_param2_t *)_param;

  const int Ntid=thread_param->Ntid;
  const int mtid=thread_param->mtid;

   //std::cout<<"mtid="<<mtid<<std::endl;

   // Generate subvolume to migrate to
   MigSubVol3D SubVol(*(thread_param->SubVol),mtid+1,Ntid);

   // Recover the travtime tab
   TravTimeTab TT(thread_param->TTData,*(thread_param->Job));

     // get the trace
   TraceData* Trace = thread_param->Trace;


     // migrate the trace to the subvolume
   MigTrace2SubVol(*(thread_param->Job),*Trace,TT,SubVol,*(thread_param->SincInt));

   return NULL;

}

bool MigTrace2SubVol(const MigrationJob &Job, TraceData &Trace,
		     TravTimeTab &TT, MigSubVol3D &SubVol, SincInterpolator &SincInt)
{
  // Migrates a trace to the given subvolume

  const int    Nx(SubVol.getNx());
  const int    Ny(SubVol.getNy());
  const int    Nz(SubVol.getNz());
  const int   ix0(SubVol.getix0());
  const int   iy0(SubVol.getiy0());
  const float  dz(SubVol.getdz());

//   std::cout<<"Nx ="<<Nx<<std::endl;
//   std::cout<<"Ny ="<<Ny<<std::endl;
//   std::cout<<"Nz ="<<Nz<<std::endl;
//   std::cout<<"ix0="<<ix0<<std::endl;
//   std::cout<<"iy0="<<iy0<<std::endl;
//   std::cout<<"x0="<<SubVol.getx0()<<std::endl;
//   std::cout<<"y0="<<SubVol.gety0()<<std::endl;

  const MOD CDPx_real = (Trace.getsx() + Trace.getgx())/2.0f;
  const MOD CDPy_real = (Trace.getsy() + Trace.getgy())/2.0f;

  const MOD Offx_real = Trace.getgx() - Trace.getsx();
  const MOD Offy_real = Trace.getgy() - Trace.getsy();

  Coords_cdp_offset<MOD> CDP_Offset_xy(CDPx_real, CDPy_real,
				       Offx_real, Offy_real);
  Coords_src_recv<MOD> Src_Recv_xy=    CDP_Offset_xy.Trafo();

  float sqAperAng= tan(deg2rad(Job.CDPAperture_deg)*
		       deg2rad(Job.CDPAperture_deg));
  float taper_width=Job.CDPApertureTaperWidth_mtr;
  float weight = 1.;

  //std::cout << "migrate:: dz = " << dz << std::endl;

  TT.Init_iz(Nz);

   // Allocate the arrays which we need during the course
   // of the computation

   float* M=SubVol.getMemPtr();

   float* T;
   posix_memalign((void**) &T, 16, 2*Nz*sizeof(float));

   float* dTdx;
   posix_memalign((void**) &dTdx, 16, 2*Nz*sizeof(float));

   float* dTdy;
   posix_memalign((void**) &dTdy, 16, 2*Nz*sizeof(float));

   float* Amp;
   posix_memalign((void**) &Amp, 16, 4*sizeof(float));


   // Perform the actual computation, i.e. loop over the output
   // volume
   //   std::cout << "migrate: Nx = " << Nx << ", Ny = " << Ny << ", Nz = " << Nz << std::endl;
   int index=0;
   for (int ix = 0; ix < Nx; ix++)
   {
       TT.Init_ix(ix+ix0);

       const float x = SubVol.getx0() + ix*SubVol.getdx();

       const float dxCDP_sqr = (x - CDP_Offset_xy.CDPx.v)*(x - CDP_Offset_xy.CDPx.v);

       //int index = (ix*Ny + iy0)*Nz;
       for (int iy = 0; iy < Ny; iy++)
         {
 	  const float y = SubVol.gety0() + iy*SubVol.getdy();
          const float dxyCDP_sqr = dxCDP_sqr + (y - CDP_Offset_xy.CDPy.v)*(y - CDP_Offset_xy.CDPy.v);

 	  const float zmin = sqrt(dxyCDP_sqr / sqAperAng);
 	  const int Nzmax = std::max(0, 4 * (std::min(Nz, (int)((fabs(SubVol.getz0()) - zmin) / dz))/4));
 	  const float ztaper_width = taper_width / sqAperAng;

 	  Taper CDPtaper;

 	  CDPtaper.Init(zmin, fabs(SubVol.getz0()) + taper_width, taper_width);

           float z = SubVol.getz0();

           TT.Init_iy(iy+iy0);
           TT.GetTT(T, dTdx, dTdy, Nzmax);

           z=SubVol.getz0();

           for (int iz = 0; iz < Nzmax; iz+=4)
             {
  		const float Tape = CDPtaper(fabs(z));

  //		const float maxfreq = fabs(1.0f/(2.0f * (80.0f * dTdy[iz])));
  //		if (maxfreq < 60.0f)
  //		    Trace.GetFilteredData(&T[iz], Amp, maxfreq);
  //		else
 		    Trace.GetData(&T[iz], Amp, SincInt);

 //#pragma vector always
               for (int i = 0; i < 4; i++)
                 {
                    M[index] += Amp[i]*Tape * weight;
                    index++;
                    z += dz;
                 }
             }
           index += (Nz-Nzmax);
         }
     }

   // free the allocated arrays

   free((void*) T);
   free((void*) dTdx);
   free((void*) dTdy);
   free((void*) Amp);

   return true;

}

