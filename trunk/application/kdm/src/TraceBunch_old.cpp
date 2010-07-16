#include <TraceBunch.hpp>

TraceBunch::TraceBunch(TraceMem& Trace)
{
  Ntib=1;

  pTraceVec= new TraceMem[Ntib]; 
 
  pTraceVec[0]=Trace;

}

TraceBunch::TraceBunch(int _oid, int _pid, int _bid, int _Ntib)
{
  oid =_oid;
  pid =_pid;
  bid =_bid;
  Nbip=1; // Number of bunches in package 
  Ntib=_Ntib;
  
  pTraceVec=NULL;

}

TraceBunch::~TraceBunch()
{
  /*std::cout<<"Hi, I am the destructor of TraceBunch and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pTraceVec<<std::endl;*/

  if(pTraceVec!=NULL)
  {
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
  if(pTraceVec!=NULL)
  {
    if(Ntib!=1)
      delete[] pTraceVec;
    else
      delete   pTraceVec;
  }
 
  // allocate the memory for the new trace data
  pTraceVec = new TraceMem[Ntib];



  // create a trace file handler
  int ierr=0;
  TraceFileHandler TFHandler(Job.TraceFileName,Job.TraceFileMode,ierr);

  int ifirst=(oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib ) * Job.n_offset;

  TFHandler.Seek(ifirst);
  // load the trace from the disk
  for(int i=0;i<Ntib;i++)
  {
    std::cout<<"Reading Trace No "<<i+1<<"/"<<Ntib
             <<" in Offset class "<<oid<<std::endl;
    pTraceVec[i].Init(TFHandler,Job);
    const MOD CDPx_real = (pTraceVec[i].getsx() + pTraceVec[i].getgx())/2.0f;
    const MOD CDPy_real = (pTraceVec[i].getsy() + pTraceVec[i].getgy())/2.0f;

    const MOD Offx_real = pTraceVec[i].getgx() - pTraceVec[i].getsx();
    const MOD Offy_real = pTraceVec[i].getgy() - pTraceVec[i].getsy();

    Coords_cdp_offset<MOD> CDP_Offset_xy(CDPx_real, CDPy_real,
    	    			         Offx_real, Offy_real);

    std::cout<<"CDPx="<<CDP_Offset_xy.CDPx.v<<std::endl;
    std::cout<<"CDPy="<<CDP_Offset_xy.CDPy.v<<std::endl;
    std::cout<<"Offx="<<CDP_Offset_xy.Offx.v<<std::endl;
    std::cout<<"Offy="<<CDP_Offset_xy.Offx.v<<std::endl;
    std::cout<<"Off ="<<sqrt(CDP_Offset_xy.Offx.v*CDP_Offset_xy.Offx.v+
                             CDP_Offset_xy.Offy.v*CDP_Offset_xy.Offy.v)
                      <<std::endl;

    // go the next trace in offset class
    if(i!=Ntib-1)
      TFHandler.Rewind(-(Job.n_offset-1));

  }

  return true;
}

void *LoadFromDisk_CO_ST(void *_param)
{

  thread_param_tb_t * thread_param = (thread_param_tb_t *)_param;
  
  TraceMem* _pTraceVec=thread_param->pTraceVec;
  const int oid=thread_param->oid;; 	
  const int pid=thread_param->pid;;	
  const int bid=thread_param->bid;;	
  const int Nbip=thread_param->Nbip;;     
  const int Ntib=thread_param->Ntib;;	
  const int Ntid=thread_param->Ntid;
  const int mtid=thread_param->mtid;
  const int n_offset=thread_param->Job->n_offset;

  // create a trace file handler
  int ierr=0;
  TraceFileHandler TFHandler(thread_param->Job->TraceFileName,
                             thread_param->Job->TraceFileMode,ierr);

  int ifirst=(oid-1)+( ( ( pid - 1 ) * Nbip + bid - 1 ) * Ntib + mtid ) * n_offset;

  TFHandler.Seek(ifirst);
  // load the trace from the disk
  int cnt=0;
  for(int i=0;i<Ntib;i++)
  {
    if( (i%Ntid) == mtid )
    {
      std::cout<<"Thread "<<mtid<<": Reading Trace No "<<i+1<<"/"<<Ntib
               <<" in Offset class "<<oid<<std::endl;
      _pTraceVec[i].Init(TFHandler,*(thread_param->Job));
      cnt++;
      const MOD CDPx_real = (_pTraceVec[i].getsx() + _pTraceVec[i].getgx())/2.0f;
      const MOD CDPy_real = (_pTraceVec[i].getsy() + _pTraceVec[i].getgy())/2.0f;

      const MOD Offx_real = _pTraceVec[i].getgx() - _pTraceVec[i].getsx();
      const MOD Offy_real = _pTraceVec[i].getgy() - _pTraceVec[i].getsy();

      Coords_cdp_offset<MOD> CDP_Offset_xy(CDPx_real, CDPy_real,
    	      			           Offx_real, Offy_real);

      std::cout<<"CDPx="<<CDP_Offset_xy.CDPx.v<<std::endl;
      std::cout<<"CDPy="<<CDP_Offset_xy.CDPy.v<<std::endl;
      std::cout<<"Offx="<<CDP_Offset_xy.Offx.v<<std::endl;
      std::cout<<"Offy="<<CDP_Offset_xy.Offx.v<<std::endl;
      std::cout<<"Off ="<<sqrt(CDP_Offset_xy.Offx.v*CDP_Offset_xy.Offx.v+
                               CDP_Offset_xy.Offy.v*CDP_Offset_xy.Offy.v)
                        <<std::endl;

      // go the next trace in offset class
      if(i+Ntid<Ntib)
        TFHandler.Rewind(-(n_offset*Ntid-1));
    }

  }
  
  std::cout<<"Hi, I am process "<<mtid<<" and I have read "<<cnt<<" traces"<<std::endl;

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
  if(pTraceVec!=NULL)
  {
    if(Ntib!=1)
      delete[] pTraceVec;
    else
      delete   pTraceVec;
  }
 
  // allocate the memory for the new trace data
  pTraceVec = new TraceMem[Ntib];

  // define the number of threads to be used 
  const int NThreads = 4; 
  pthread_t threads[NThreads];
  thread_param_tb_t thread_params[NThreads];

  for(int mtid=0;mtid<NThreads;mtid++)
  {
    thread_params[mtid].pTraceVec = pTraceVec;
    thread_params[mtid].Job       = &Job;
    thread_params[mtid].oid       = oid; 	
    thread_params[mtid].pid       = pid;	
    thread_params[mtid].bid       = bid;	
    thread_params[mtid].Nbip      = Nbip;     
    thread_params[mtid].Ntib      = Ntib;	
    thread_params[mtid].Ntid      = NThreads;
    thread_params[mtid].mtid      = mtid;

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


TraceMem TraceBunch::operator[] (const int i)
{
  return pTraceVec[i];
}

TraceMem* TraceBunch::getTrace(const int i)
{
  return &pTraceVec[i];
}