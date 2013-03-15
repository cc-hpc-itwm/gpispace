#include <TraceBunch.hpp>

TraceBunch::TraceBunch(int _oid, int _pid, int _bid, MigrationJob &_Job)
{
  oid =_oid;
  pid =_pid;
  bid =_bid;
  Nbip=Nbid_in_pid(_oid,_pid,_Job);//_Job.bunchesperpack; // Number of bunches in package
  Ntib=NTrace_in_bid(_oid,_pid,_bid,_Job);//_Job.tracesperbunch;

  Nt  =_Job.traceNt;

  SizeOfOneTD=getSizeOfOneTD(Nt);

  pTraceData=NULL;
  pTraceVec=NULL;

  TraceBunchMemRqst=true;

}

TraceBunch::TraceBunch(volatile char * _pTraceData,int _oid, int _pid, int _bid, MigrationJob &_Job)
{
  oid =_oid;
  pid =_pid;
  bid =_bid;
  Nbip=Nbid_in_pid(_oid,_pid,_Job);//_Job.bunchesperpack; // Number of bunches in package
  Ntib=NTrace_in_bid(_oid,_pid,_bid,_Job);//_Job.tracesperbunch;

  Nt=_Job.traceNt;

  pTraceData=NULL;
  pTraceVec=NULL;

  pTraceData=_pTraceData;
  pTraceVec=new TraceData*[Ntib];

  SizeOfOneTD=getSizeOfOneTD(Nt);

   // recover the trace data from the char array
   for(int i=0;i<Ntib;i++)
   {
     //std::cout<<"Reading Trace No "<<i+1<<"/"<<Ntib
     //         <<" in Offset class "<<oid<<" from delivered char string"<<std::endl;
     pTraceVec[i]= new TraceData(pTraceData+i*SizeOfOneTD,Nt);
   }

  TraceBunchMemRqst=false;
}

TraceBunch::~TraceBunch()
{
  /*std::cout<<"Hi, I am the destructor of TraceBunch and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pTraceVec<<std::endl;*/

  if( (pTraceData!=NULL) && TraceBunchMemRqst)
  {
    if(Ntib!=1)
      delete[] pTraceData;
    else
      delete   pTraceData;
  }
  if(pTraceVec!=NULL)
  {
    for(int i=0;i<Ntib;i++)
      delete pTraceVec[i];

    if(Ntib!=1)
      delete[] pTraceVec;
    else
      delete   pTraceVec;
  }
}

bool TraceBunch::LoadFromDisk_CO(MigrationJob &Job)
{

  // formula to load the i-th trace out of an offset
  // class (i=1 .. NtpOC):
  // (oid-1)+(i-1)*Job.n_offset
  // or to load the i-th trace out of a bunch with and offset
  // id oid, and an package id pid  (i=1 .. Ntib)
  // (oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib + i - 1 ) * Job.n_offset;


  // first, reinitialize the vector containing the traces
  // if necessray
  if(pTraceData!=NULL && TraceBunchMemRqst)
  {
    if(Ntib!=1)
      delete[] pTraceData;
    else
      delete   pTraceData;

    pTraceData=NULL;
  }

  if(pTraceVec!=NULL)
  {

    for(int i=0;i<Ntib;i++)
      delete pTraceVec[i];

    if(Ntib!=1)
      delete[] pTraceVec;
    else
      delete   pTraceVec;
  }

  // create a trace file handler
  int ierr=0;
  TraceFileHandler TFHandler(Job.TraceFileName,Job.TraceFileMode,ierr);

  // allocate the memory for the new trace data
  // here, we assume that the trace data is absolutely
  // regular
  //if(Nt==0)
  //  Nt=TFHandler.getNt();

  if(pTraceData==NULL)
    pTraceData = new volatile char[Ntib*SizeOfOneTD];
  //if(pTraceVec==NULL)
  pTraceVec  = new TraceData*[Ntib];

  if(pTraceData==NULL)
    std::cerr<<"TraceBunch::LoadFromDisk_CO: Could not allocate memory for Trace data!!!"<<std::endl;

  //std::cout<<"TraceBunch::LoadFromDisk_CO: pTraceData="<<(void *)pTraceData<<std::endl;

  //int ifirst=(oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib ) * Job.n_offset;
  //const int Nbipmax=Nbid_in_pid(oid,1,Job);
  //const int Ntibmax=NTrace_in_bid(oid,1,1,Job);
  //int ifirst=(oid-1)+( ( ( pid - 1 ) * Nbipmax + bid - 1 ) * Ntibmax ) * Job.n_offset;
  int file_offset=getfile_offset(oid,pid,bid,1,Job);
  int file_offset_old=file_offset;
  int file_offset_diff;

  //TFHandler.Seek(ifirst);
  TFHandler.Seek(file_offset);
  // load the trace from the disk
  //TraceData TraceWithEstablishedFilter;
  for(int i=0;i<Ntib;i++)
  {
    //std::cout<<"Reading Trace No "<<i+1<<"/"<<Ntib
    //         <<" in bid "<<bid<<" pid "<<pid<<" oid "<<oid<<std::endl;
    //pTraceVec[i].Init(TFHandler,Job);
    //TraceWithEstablishedFilter.SetNewCharPtr(pTraceData+i*SizeOfOneTD,Nt);
    //TraceWithEstablishedFilter.LoadFromDisk(TFHandler,Job);
    pTraceVec[i]= new TraceData(pTraceData+i*SizeOfOneTD,Nt);
    pTraceVec[i]->LoadFromDisk(TFHandler,Job);
    //std::cout<<"Dumping Trace Data after load"<<std::endl;
    //TraceData Test(pTraceData+i*SizeOfOneTD,Nt);
    //Test.Dump();
    //const MOD CDPx_real = (pTraceVec[i]->getsx() + pTraceVec[i]->getgx())/2.0f;
    //const MOD CDPy_real = (pTraceVec[i]->getsy() + pTraceVec[i]->getgy())/2.0f;

    //const MOD Offx_real = pTraceVec[i]->getgx() - pTraceVec[i]->getsx();
    //const MOD Offy_real = pTraceVec[i]->getgy() - pTraceVec[i]->getsy();

    //Coords_cdp_offset<MOD> CDP_Offset_xy(CDPx_real, CDPy_real,
    //	    			         Offx_real, Offy_real);

    //std::cout<<"CDPx="<<CDP_Offset_xy.CDPx.v<<std::endl;
    //std::cout<<"CDPy="<<CDP_Offset_xy.CDPy.v<<std::endl;
    //std::cout<<"Offx="<<CDP_Offset_xy.Offx.v<<std::endl;
    //std::cout<<"Offy="<<CDP_Offset_xy.Offx.v<<std::endl;
    //std::cout<<"Off ="<<sqrt(CDP_Offset_xy.Offx.v*CDP_Offset_xy.Offx.v+
    //                         CDP_Offset_xy.Offy.v*CDP_Offset_xy.Offy.v)
    //                  <<std::endl;

    // go the next trace in offset class
    if(i!=Ntib-1)
    {
      file_offset_old=file_offset;
      file_offset=getfile_offset(oid,pid,bid,i+2,Job);
      file_offset_diff=file_offset-file_offset_old;
      //TFHandler.Rewind(-(Job.n_offset-1));
      TFHandler.Rewind(-(file_offset_diff-1));
    }

  }
  return true;
}

void *LoadFromDisk_CO_ST(void *_param)
{

  thread_param_tb_t * thread_param = (thread_param_tb_t *)_param;

  volatile char *       _pTraceData = thread_param->pTraceData;
  int 	       SizeOfOneTD = thread_param->SizeOfOneTD;
  const int   		Nt = thread_param->Nt;
  TraceData** 	_pTraceVec = thread_param->pTraceVec;
  const int 	       oid = thread_param->oid;;
  const int 	       pid = thread_param->pid;;
  const int 	       bid = thread_param->bid;;
  const int 	      Ntib = thread_param->Ntib;;
  const int 	      Ntid = thread_param->Ntid;
  const int 	      mtid = thread_param->mtid;

  // create a trace file handler
  int ierr=0;
  TraceFileHandler TFHandler(thread_param->Job->TraceFileName,
                             thread_param->Job->TraceFileMode,ierr);

  //int ifirst=(oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib + mtid ) * n_offset;
  //const int Nbipmax=Nbid_in_pid(oid,1,*(thread_param->Job));
  //const int Ntibmax=NTrace_in_bid(oid,1,1,*(thread_param->Job));
  //int ifirst=(oid-1)+ ( ( ( ( pid - 1 ) * Nbipmax ) + bid - 1 ) * Ntibmax + mtid ) * n_offset;

  int file_offset=getfile_offset(oid,pid,bid,1+mtid,*(thread_param->Job));
  int file_offset_old=file_offset;
  int file_offset_diff;

  TFHandler.Seek(file_offset);

  //std::cout<<"TraceBunch::LoadFromDisk_CO_ST: _pTraceData"<<(void *)_pTraceData<<std::endl;

  //TraceData TraceWithEstablishedFilter;
  // load the trace from the disk
  int cnt=0;
  for(int i=0;i<Ntib;i++)
  {
    if( (i%Ntid) == mtid )
    {
      //std::cout<<"Thread "<<mtid<<": Reading Trace No "<<i+1<<"/"<<Ntib
      //       <<" in bid "<<bid<<" pid "<<pid<<" oid "<<oid<<std::endl;
      //_pTraceVec[i]->Init(TFHandler,*(thread_param->Job));
      //TraceWithEstablishedFilter.SetNewCharPtr(_pTraceData+i*SizeOfOneTD,Nt);
      //TraceWithEstablishedFilter.LoadFromDisk(TFHandler,*(thread_param->Job));
      _pTraceVec[i]= new TraceData(_pTraceData+i*SizeOfOneTD,Nt);
      _pTraceVec[i]->LoadFromDisk(TFHandler,*(thread_param->Job));
      cnt++;
      /*const MOD CDPx_real = (_pTraceVec[i]->getsx() + _pTraceVec[i]->getgx())/2.0f;
      const MOD CDPy_real = (_pTraceVec[i]->getsy() + _pTraceVec[i]->getgy())/2.0f;

      const MOD Offx_real = _pTraceVec[i]->getgx() - _pTraceVec[i]->getsx();
      const MOD Offy_real = _pTraceVec[i]->getgy() - _pTraceVec[i]->getsy();

      Coords_cdp_offset<MOD> CDP_Offset_xy(CDPx_real, CDPy_real,
    	      			           Offx_real, Offy_real);

      std::cout<<"CDPx="<<CDP_Offset_xy.CDPx.v<<std::endl;
      std::cout<<"CDPy="<<CDP_Offset_xy.CDPy.v<<std::endl;
      std::cout<<"Offx="<<CDP_Offset_xy.Offx.v<<std::endl;
      std::cout<<"Offy="<<CDP_Offset_xy.Offx.v<<std::endl;
      std::cout<<"Off ="<<sqrt(CDP_Offset_xy.Offx.v*CDP_Offset_xy.Offx.v+
                               CDP_Offset_xy.Offy.v*CDP_Offset_xy.Offy.v)
                        <<std::endl;*/

      // go the next trace in offset class
      if(i+Ntid<Ntib)
      {
        file_offset_old=file_offset;
        file_offset=getfile_offset(oid,pid,bid,i+Ntid+1,*(thread_param->Job));
        file_offset_diff=file_offset-file_offset_old;
        //TFHandler.Rewind(-(n_offset*Ntid-1));
        TFHandler.Rewind(-(file_offset_diff-1));

      }
    }

  }

  //std::cout<<"Hi, I am process "<<mtid<<" and I have read "<<cnt<<" traces"<<std::endl;

  return NULL;

}

bool TraceBunch::LoadFromDisk_CO_MT(MigrationJob &Job)
{

  // Multi threaded version of
  // TraceBunch::LoadFromDisk_CO

  // formula to load the i-th trace out of an offset
  // class (i=1 .. NtpOC):
  // (oid-1)+(i-1)*Job.n_offset
  // or to load the i-th trace out of a bunch with and offset
  // id oid, and an package id pid  (i=1 .. Ntib)
  // (oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib + i - 1 ) * Job.n_offset;


  // first, reinitialize the vector containing the traces
  // if necessray
  if(pTraceData!=NULL && TraceBunchMemRqst)
  {
    if(Ntib!=1)
      delete[] pTraceData;
    else
      delete   pTraceData;
  }
   if(pTraceVec!=NULL)
  {

    for(int i=0;i<Ntib;i++)
      delete pTraceVec[i];

    if(Ntib!=1)
      delete[] pTraceVec;
    else
      delete   pTraceVec;
  }

  // allocate the memory for the new trace data
  // here, we assume that the trace data is absolutely
  // regular
  //if(Nt==0)
  //  Nt=TFHandler.getNt();
  //const int SizeOfOneTD=(5*Nt+9)*sizeof(float)+sizeof(int);
  if(pTraceData==NULL)
    pTraceData = new volatile char[Ntib*SizeOfOneTD];
  pTraceVec  = new TraceData*[Ntib];


  // define the number of threads to be used
  const int NThreads = 4;
  pthread_t threads[NThreads];
  thread_param_tb_t thread_params[NThreads];

  for(int mtid=0;mtid<NThreads;mtid++)
  {
    thread_params[mtid].pTraceData = pTraceData;
    thread_params[mtid].SizeOfOneTD=SizeOfOneTD;
    thread_params[mtid].Nt         =Nt;
    thread_params[mtid].pTraceVec  = pTraceVec;
    thread_params[mtid].Job        = &Job;
    thread_params[mtid].oid        = oid;
    thread_params[mtid].pid        = pid;
    thread_params[mtid].bid        = bid;
    thread_params[mtid].Nbip       = Nbip;
    thread_params[mtid].Ntib       = Ntib;
    thread_params[mtid].Ntid       = NThreads;
    thread_params[mtid].mtid       = mtid;

    int rc=pthread_create(&threads[mtid],NULL,
                          LoadFromDisk_CO_ST, (void *) &thread_params[mtid]);

    if (rc)
      printf("failed to create a thread!\n");
  }

  for(int mtid=0;mtid<NThreads;mtid++)
  {
    int rc=pthread_join(threads[mtid],NULL);
    if (rc)
      printf("failed to join a thread\n");
  }

  return true;
}


TraceData* TraceBunch::operator[] (const int i)
{
  return getTrace(i);
}

TraceData* TraceBunch::getTrace(const int i)
{
  return pTraceVec[i];
}

volatile char * TraceBunch::getDataPtr()
{
  return pTraceData;
}

int getSizeOfOneTD(const int &_Nt)
{
  // Returns the size of one trace data set
  // We have the size of one int for Nt
  // We have the size of seven floats for
  //      1.) dtbin
  //      2.) T0
  //      3.) Tmax
  //      4.) sx.v
  //      5.) sy.v
  //      6.) gx.v
  //      7.) gy.v
  // We have the size of 5*Nt floats for the trace data itself
  // and its frequency filtered counter parts. Cross check this
  // with TraceData.cpp
  return (5*_Nt+7)*sizeof(float)+sizeof(int);
}

int getSizeofTD(const MigrationJob &_Job)
{
  // Determine the maximal number of traces in bunch
  int tibmax=0;
  for(int _oid=1;_oid<=_Job.n_offset;_oid++)
  {
    int tibcur=NTrace_in_bid(_oid,1,1,_Job);
    if(tibcur>tibmax) tibmax=tibcur;
  }
  //std::cout<<"SizeofTD="<<tibmax*getSizeOfOneTD(_Job.traceNt)<<std::endl;
  return tibmax*getSizeOfOneTD(_Job.traceNt);
}

int NTrace_in_oid(int oid,const MigrationJob &Job)
{
  float dOffVol=Job.dOffVol,dOffDat=Job.d_offset;
  float Off0Vol=Job.Off0Vol,Off0Dat=Job.first_offset;
  int                       NOffDat=Job.n_offset;

  float OffVol,OffDat;

  int fac=0;

  if(dOffVol<=dOffDat) fac=1;
  else
  {
    OffVol=Off0Vol+(oid-1)*dOffVol;
    for(int i=1;i<=NOffDat;i++)
    {
      OffDat=Off0Dat+(i-1)*dOffDat;
      if( (OffDat >= OffVol-0.5*dOffVol) &&
          (OffDat < OffVol+0.5*dOffVol) )
         fac++;
    }
  }

  return fac*Job.n_xlines_CDP*Job.n_inlines_CDP;

}

int Npid_in_oid(int oid,const MigrationJob &Job)
{
  return 1;
}

int Nbid_in_pid(int oid, int pid, const MigrationJob &Job)
{
   int Npid=Npid_in_oid(oid,Job);

   if( (pid<1) || (pid>Npid) )
     std::cerr<<"pid is out of range!!!"<<std::endl;

   int NTraceTot=NTrace_in_oid(oid,Job);

   int Nbid_in_pid;

   if(pid!=Npid)
   {
     Nbid_in_pid=1;
   }
   else
   {
     Nbid_in_pid=(NTraceTot-(Npid-1)*Job.tracesperbunch)/Job.tracesperbunch;
     if( (NTraceTot-(Npid-1)*Job.tracesperbunch)%Job.tracesperbunch !=0)
       Nbid_in_pid++;
   }

   return Nbid_in_pid;

}

int NTrace_in_bid(int oid, int pid, int bid, const MigrationJob &Job)
{
  int Npid=Npid_in_oid(oid,Job);
  int Nbid=Nbid_in_pid(oid,pid,Job);

   if( (bid<1) || (bid>Nbid) )
     std::cerr<<"pid is out of range!!!"<<std::endl;

   int NTraceTot=NTrace_in_oid(oid,Job);

   int NTrace_in_bid;

   if(bid==Nbid && pid==Npid)
   {
     NTrace_in_bid=NTraceTot-( (pid-1)+(Nbid-1) )*Job.tracesperbunch;
   }
   else
   {
     NTrace_in_bid=Job.tracesperbunch;
   }

   return NTrace_in_bid;
}

int getfile_offset(int _oid, int _pid,int _bid,int _tid,const MigrationJob &_Job)
{
   // _oid = Offset class id of the output volume
   // _pid = Package id in offset class
   // _bid = Bunch id in package
   // _tid = trace id in bunch
   const int Nbipmax=Nbid_in_pid(_oid,1,_Job);
   const int Ntibmax=NTrace_in_bid(_oid,1,1,_Job);

   int SDTinOC=_Job.n_xlines_CDP*_Job.n_inlines_CDP; // SeismicData  : Number of traces in one offset class
   //int OVTinOC=NTrace_in_oid(_oid,_Job);             // Output Volume: Number of trace in the offset class

   const int OVTIinOC=( ( ( _pid - 1 ) * Nbipmax) + _bid - 1 ) * Ntibmax + _tid; // OutputVolume: absolute trace id in
                                                                                //               offset class

   // Compute the offset_id in the seismic data
   const float &dOffVol=_Job.dOffVol,&dOffDat=_Job.d_offset;
   const float &Off0Vol=_Job.Off0Vol,&Off0Dat=_Job.first_offset;
   const int                         &NOffDat=_Job.n_offset;

   float OffVol,OffDat;

   int SDTIinOC=0;
   int i=1;
   OffVol=Off0Vol+(_oid-1)*dOffVol;

   if(dOffVol<=dOffDat)
   {
     //! \note UNSAFE conversion from float to int
     int imin (static_cast<int>((OffVol-Off0Dat)/dOffDat));
     if(imin<-.5)
       std::cout<<"get_file_offset: ERROR !!!"<<std::endl;

     if( fabs(Off0Dat + imin*dOffDat-OffVol) <= (0.5 * dOffDat) )
       i=imin+1;
     else
       i=imin+2;
     SDTIinOC=OVTIinOC;
   }
   else
   {
     int success=0;
     while(i<=NOffDat && !success)
     {
       OffDat=Off0Dat+(i-1)*dOffDat;
       if( (OffDat >= OffVol-0.5*dOffVol) &&
           (OffDat < OffVol+0.5*dOffVol) )
       {
         SDTIinOC+=SDTinOC;
         if(SDTIinOC>OVTIinOC)
         {
           success=1;
           SDTIinOC=OVTIinOC-(SDTIinOC-SDTinOC);
         }
       }
       i++;
     }
     i--;
   }
   int oid_data=i;

   return ( (oid_data-1)+(SDTIinOC-1)*_Job.n_offset );

}
