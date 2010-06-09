#include <sdpa_loadalltraces.hpp>

bool SDPA_loadalltraces(int _oid, MigrationJob & _Job, ParallelEnvironment &_PE)
{

  _PE.Barrier();

  const int VMRank=_PE.GetRank();

  for(int pid=1;pid<=Npid_in_oid(_oid,_Job);pid++)
  {
    for(int bid=1;bid<=Nbid_in_pid(_oid,pid,_Job);bid++)
    {
      int relgloboff,VMRank2Store;
      SDPA_detbunchloc(_oid,pid,bid,_Job,_PE,relgloboff,VMRank2Store); 
      if( VMRank2Store == VMRank ) 
      {
        char * pBunchData = (char *)_PE.getMemPtr() + _Job.globbufoffset + relgloboff;
        TraceBunch Bunch(pBunchData,_oid,pid,bid,_Job);

        /* Do not forget to uncomment prepare Data in
           TraceData.LoadFromDisk !!! */
        bool retval=Bunch.LoadFromDisk_CO_MT(_Job);
      }
    } 
  }  

  _PE.Barrier();  

  return true;

}

bool SDPA_detbunchloc(const int &_oid, const int &_pid, const int &_bid, const MigrationJob &_Job, 
                      ParallelEnvironment &PE, int &_relgloboff, int &_VMStoreRank)
{

  const int VMSize=PE.GetNodeCount();

  int bunchcnt=0;

  for(int pid=1;pid<=_pid;pid++)
  {
    int bid_ub= ( (pid==_pid) ) ? _bid : Nbid_in_pid(_oid,pid,_Job);  ///upper bound for the bid
    for(int bid=1;bid<=bid_ub;bid++)
    {
      bunchcnt++;
    } 
  }

  bunchcnt--;

  _VMStoreRank=bunchcnt % VMSize;
  if(VMSize!=1)  
    _relgloboff=(bunchcnt/VMSize)*_Job.BunchMemSize;
  else
    _relgloboff=(bunchcnt/VMSize)*_Job.BunchMemSize;

  return true;

}