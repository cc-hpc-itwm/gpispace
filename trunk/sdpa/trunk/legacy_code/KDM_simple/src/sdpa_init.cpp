#include <sdpa_init.hpp>

/* Print welcome message */
void welcome()
{











}

/* Print error message to stdout */
void errormessage(char* binary_name, const int mode = 0)
{
  switch (mode)
    {
    case 0 :
      std::cerr << binary_name << " : invalid option\n";
      std::cerr << "Try `" << binary_name << " -h' for more information.\n";
      break;

    case 1 :
      std::cerr << binary_name << " : no configuration file specified\n";
      std::cerr << "Try `" << binary_name << " -h' for more information.\n\n";
      break;
    }
}

/* Print usage description to stdout */
void helpmessage(char* binary_name)
{










}


/* Exit program */
void fatalexit()
{
  int errcode = 1;
  exit(errcode);
}

// Return
// char* CfgFileName, MigrationJob &Job
bool SDPA_init(int argc, char *argv[],MigrationJob &Job, ParallelEnvironment &PE, int NThreads)
{

  int VMMemSize=1024*1024*1024; // Request 1 GByte of VM Mem

  welcome();

  char* CfgFileName = NULL;

  bool PV4D_VM_WORKER = false;
  bool VMEXPERT =  false;
  unsigned long MemSize = 0;
  unsigned int VMport = 0;

  // Read command line
  opterr = 0;  // suppress any error message about unknown options
  optind = 0;  // start with the search at the beginning

  int c;
  int argval;
  char* pointer;
  int i;
  while ((c = getopt(argc, argv, "+hf:s:m:t:p:x")) != -1)
    {
      switch (c) {
	  case 'h':
	      helpmessage (basename(argv[0]));
	      fatalexit();
	      break;
	  case 'f':
	  {
	      if ( CfgFileName == NULL)
		  CfgFileName = new char[199];
	      i = 0;
	      pointer = (char*) optarg;
	      while ( (*pointer != (char)0) && (i < 199-1) )
	      {
		  CfgFileName[i] = *pointer;
		  i++;
		  pointer++;
	      }
	      CfgFileName[i] = (char)0;
	      break;
	  }
	  case 't':
	      PV4D_VM_WORKER = true;
	      break;
	  case 'm':
	  {

	      break;
	  }
	  case 's':
	  {
	      MemSize = (unsigned long) (atof(optarg));
	      if (MemSize*1024 < 1024*1024)
		  MemSize = (MemSize*1024) * 1024*1024;
	      break;
	  }
	  case 'p':
	  {
	      VMport = (unsigned int) (atof(optarg));
	      break;
	  }
	  case 'x':
	  {
	      VMEXPERT = true;
	      break;
	  }
	  default:
	      break;
      }
    }
  optind = 0; // Set back to first parameter so that vm can read it.

  // Initialization on head node
  if ( !PV4D_VM_WORKER )
    {
      if (CfgFileName == NULL) // Check for configuration file
	{

	  helpmessage (basename(argv[0]));
	  exit(1);
	}

      // Read the configuration file
      CheckReadMigrationJob JobReader;
      if (JobReader.ReadConfigFileXML(CfgFileName, Job) != 0)
	{
	  exit(-1);
	}

      char JobFile[2*199 + 16];
      sprintf(JobFile, "%s/%s_mig.xml", Job.MigDirName, Job.JobName);

      if (JobReader.WriteConfigFileXML(JobFile, Job) == -1)
	fatalexit();

     // Check whether the output volume is totally covered by the
     // travel time tables
     // if not, leave the program !
     {
       int ierr;
       BlockVolume BoxVolume(Job.MigVol,Job.TTVol,ierr);
       if(ierr==-1)
       {


         exit(-1);
       }
     }

     // determine required size of localbuffers ->
     // the local buffer must hold one trace bunch
     {
       Job.locbufsize=getSizeofTD(Job); // local buffer size in bytes

       // determine required size of the global buffer
       // holding the entire trace data of an offset class
       int NBtotMax=0;
       for(int oid=1;oid<=Job.n_offset;oid++)
       {
         int NBtotInOC=0; // Total number of bunches in offset class
         for(int pid=1;pid<=Npid_in_oid(oid,Job);pid++)
           NBtotInOC+=Nbid_in_pid(oid,pid,Job);
         if(NBtotInOC>NBtotMax) NBtotMax=NBtotInOC;
       }
       int globbufNofB=NBtotMax; // maximal number of bunches in global buffer
       int globbufNofBlocal; // maximal number of bunches in global buffer on
                           // the local machine
       if(NBtotMax%ParallelEnvironment::GetAllocatedNodeCount()==0)
         globbufNofBlocal=NBtotMax/ParallelEnvironment::GetAllocatedNodeCount();
       else
         globbufNofBlocal=NBtotMax/ParallelEnvironment::GetAllocatedNodeCount()+1;

       Job.globbufsizelocal=globbufNofBlocal*Job.locbufsize; // global buffer size in bytes
                                                             // on the local machine
     }

     // determine the required size for the traveltime tables
     {
       TTVMMemHandler TTVMMem;
       grid2D GSrc;
       grid3D GVol;

       // Create the 2-D surface grid description
       double x0Srfc = Job.SrfcGridX0.v;
       double y0Srfc = Job.SrfcGridY0.v;

       double dxSrfc = Job.SrfcGriddx;
       double dySrfc = Job.SrfcGriddy;

       int NxSrc = Job.SrfcGridNx;
       int NySrc = Job.SrfcGridNy;

       GSrc.Init(x0Srfc, y0Srfc, NxSrc, NySrc, dxSrfc, dySrfc);

       // Create the 3-D subsurface grid description
       int Nx = Job.TTVol.nx_xlines;
       int Ny = Job.TTVol.ny_inlines;
       int Nz = Job.TTVol.nz;

       point3D<float> X0, dx;
       X0[0] = Job.TTVol.first_x_coord.v;
       X0[1] = Job.TTVol.first_y_coord.v;
       X0[2] = Job.TTVol.first_z_coord;
       dx[0] = Job.TTVol.dx_between_xlines;
       dx[1] = Job.TTVol.dy_between_inlines;
       dx[2] = Job.TTVol.dz;

       GVol.Init(X0, point3D<int>(Nx,Ny,Nz), dx);

       // Load the entire travel time table data into memory
       int NodeCount=ParallelEnvironment::GetAllocatedNodeCount();
       Job.globTTbufsizelocal=TTVMMem.GetMem(Job.RTFileName,GSrc,GVol,NodeCount,NThreads)/NodeCount;
     }

     // determine required size of subvolumina
     // First construct the total volume
     {
       point3D<float> X0(Job.MigVol.first_x_coord.v,
	    	         Job.MigVol.first_y_coord.v,
                         Job.MigVol.first_z_coord);
       point3D<int>   Nx(Job.MigVol.nx_xlines,
	                 Job.MigVol.ny_inlines,
                         Job.MigVol.nz);
       point3D<float> dx(Job.MigVol.dx_between_xlines,
	  	         Job.MigVol.dy_between_inlines,
                         Job.MigVol.dz);
       MigVol3D MigVol(X0,Nx,dx);

       // Then, define the Subvolume
       MigSubVol3D MigSubVol(MigVol,1,Job.NSubVols);

       // At last, determine the size of the subvolume in memory
       Job.SubVolMemSize=MigSubVol.getSizeofSVD(); //subvol mem size
     }

     Job.ReqVMMemSize    = Job.globbufsizelocal+Job.globTTbufsizelocal+2*Job.locbufsize+Job.SubVolMemSize;
     Job.ReqVMMemSize    = (int)(1.1*(float)(Job.ReqVMMemSize));
     Job.globTTbufoff	 = 0;
     Job.globbufoffset   = Job.globTTbufoff+Job.globTTbufsizelocal;
     Job.locbuf1off      = Job.globbufoffset+Job.globbufsizelocal; // VMOffset for first local buffer
     Job.locbuf2off      = Job.locbuf1off+Job.locbufsize;
     Job.SubVolMemOff    = Job.locbuf2off+Job.locbufsize;
     Job.BunchMemSize    = getSizeofTD(Job);

     // Check whether the travel time table covers the entire output volume

     // Touch the offset gather
     MigrationFileHandler MFHandler;
     MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                                 Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);
  }

  // Now, initialize the virtual machine
  int ierr;
  PE.StartUp(argc,argv,NULL,VMMemSize,ierr,VMport,!VMEXPERT);
  if (ierr!=0)
  {

    exit(-1);
  }
  PE.ExitOnError(ierr);


  // Broadcast the Job description
  char * SerializedJob;
  int SizeOfJob=sizeof(MigrationJob);
  if(PE.GetRank()==0)
  {
    SerializedJob=new char[SizeOfJob];
    memcpy(SerializedJob,(char *)&Job,SizeOfJob)    ;
  }
  PE.Broadcast(SerializedJob,SizeOfJob,0);
  if(PE.GetRank()!=0)
  {
    memcpy((char *)&Job,SerializedJob,SizeOfJob);
  }
  delete[] SerializedJob;

  // Check whether the requested VM Size fits into the allocated Mem
  if(Job.ReqVMMemSize>VMMemSize)
  {


    exit(-1);
  }

  // Check whether the number of allocated nodes is equal or larger than the
  // number of subvolumes
  if(Job.NSubVols>PE.GetNodeCount())
  {

    exit(-1);
  }

  return true;

}
