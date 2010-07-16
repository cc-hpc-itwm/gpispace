#define VERSION "1.1"

#include <sdpa_init.hpp>
#include <sdpa_initsubvol.hpp>
#include <sdpa_loadallTT.hpp>
#include <sdpa_loadalltraces.hpp>
#include <sdpa_migrate.hpp>
#include <sdpa_writeoutvol2disk.hpp>

//#include <stdlib.h>

//#include "structures/migrationjob.h"
//#include "filehandler/checkreadmigrationjob.h"
//#include "filehandler/migrationfilehandler.h"
//#include "utils/loggingclass.h"
//#include <unistd.h>
//#include <string.h>
//#include <libgen.h>
#include <iostream>
//#include <stdlib.h>





int main(int argc, char *argv[])
{

  int NThreads=4;

  // Instantiate the ParallelEnvironment and the job description variables
  ParallelEnvironment PE;
  MigrationJob        Job;

  // SDPA init
  SDPA_init(argc,argv,Job,PE,NThreads);
  

//   // Reconstruct the subvolume out of memory
//   point3D<float> X0(Job.MigVol.first_x_coord.v,
//         	    Job.MigVol.first_y_coord.v,
//                     Job.MigVol.first_z_coord);
//   point3D<int>   Nx(Job.MigVol.nx_xlines,
//    		    Job.MigVol.ny_inlines,
//                     Job.MigVol.nz);
//   point3D<float> dx(Job.MigVol.dx_between_xlines,
//    	            Job.MigVol.dy_between_inlines,
//                     Job.MigVol.dz);
//   MigVol3D MigVol(X0,Nx,dx);
// 
//   // create the subvolume
//   MigSubVol3D ** SubVol1, ** SubVol2;
//   SubVol1 = new MigSubVol3D*[5];
//   SubVol2 = new MigSubVol3D*[25];
// 
//   for(int i=1;i<=5;i++)
//   {
//     SubVol1[i-1]= new MigSubVol3D(MigVol,i,5);
//     for(int j=1;j<=5;j++)
//     {
//       SubVol2[(i-1)*5+(j-1)] = new MigSubVol3D(*SubVol1[i-1],j,5); 
//     }    
//     //delete SubVol1[i];
//   }
//   //delete[] SubVol1;
//  
//   for(int i=0;i<25;i++)
//   {
//     std::cout<<"i="<<i<<std::endl;
//     std::cout<<"ix0="<<SubVol2[i]->getix0()<<std::endl;
//     std::cout<<"x0="<<SubVol2[i]->getx0()<<std::endl;
//     std::cout<<"Nx="<<SubVol2[i]->getNx()<<std::endl;
//     std::cout<<"iy0="<<SubVol2[i]->getiy0()<<std::endl;
//     std::cout<<"Ny="<<SubVol2[i]->getNy()<<std::endl;
//     std::cout<<"y0="<<SubVol2[i]->gety0()<<std::endl;
//     std::cout<<"iz0="<<SubVol2[i]->getiz0()<<std::endl;
//     std::cout<<"Nz="<<SubVol2[i]->getNz()<<std::endl;
//     std::cout<<"z0="<<SubVol2[i]->getz0()<<std::endl;
//     std::cout<<std::endl;
//     //delete SubVol2[i];
//   }
//   //delete[] SubVol2;
//   
//   return 0;

  // Initialize the sinc interpolator
  //SincInterpolator SincInt;
  //SincInt.init(Job.tracedt);
  SincInterpolator ** SincIntA = new SincInterpolator*[NThreads];
  for(int i=0;i<NThreads;i++)
  {
    SincIntA[i]=new SincInterpolator;
    SincIntA[i]->init(Job.tracedt);
  }      

  // load all travel time tables into memory
  
  SDPA_loadallTT(Job,PE,NThreads);
  

  // loop over the offset classes
  for(int oid=1;oid<=Job.n_offset;oid++)
  {

    

    // initialize the subvolume
    
    SDPA_initsubvol(Job,PE);

    // load the trace bunches into memory
    
    SDPA_loadalltraces(oid,Job,PE);

    int pid2mig=0;
    int bid2mig=0;
    int pid2load;
    int bid2load;
    int actbuf=1;

    if(PE.GetRank()+1<=Job.NSubVols)
    {
      
      for(int pid=1;pid<=Npid_in_oid(oid,Job);pid++)
      {
        for(int bid=1;bid<=Nbid_in_pid(oid,pid,Job);bid++)
        {

          pid2load=pid;
          bid2load=bid;

          SDPA_mig_and_pref(oid,pid2mig,bid2mig,pid2load,bid2load,actbuf,
                            Job,PE,SincIntA,NThreads);

          pid2mig=pid2load;
          bid2mig=bid2load;
          pid2load=0;
          bid2load=0;

          actbuf= (actbuf==1) ? 2 : 1;        

        } 
      }

      // Migrate the last trace bunch which was prefetched
      SDPA_mig_and_pref(oid,pid2mig,bid2mig,pid2load,bid2load,actbuf,
                        Job,PE,SincIntA,NThreads);

      
      SDPA_writeoc2disk(oid,Job,PE);

    }       
 
  }       
      
  
  PE.Barrier();
  for(int i=0;i<NThreads;i++)
  {
    delete SincIntA[i];
  }
  delete[] SincIntA;

  return EXIT_SUCCESS;

}
