/**************************************************************************
                          main.cpp  -  description
                             -------------------

    Main for the MPI parallel Version of the wavefront ray tracing code.
    All necessary parameters are passed in a configuration file given as
    command line option.
    The computational task is distributed per shot one by one to the compute
    nodes. Thereto the corresponding job descriptions are generated and the
    tracer is started on these jobs.

                              -------------------
    begin                : Fri Nov 11 10:07:59 CET 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2009-11-17
		allow for specification/check of port and xpert mode

       merten | 2009-11-10
		machine file no longer needed for vm4

       merten | 2009-11-05
                new variables for velicity file pre-processing included

      micheld | 2009-10-27
		startPv4dVM uses NULL as cmdline -> non-master nodes start the same program as node 0
		note that the cmdline could start other binaries than node 0, but this feature is not needed

      micheld | 2009-10-26
		syntax changes for VM4

***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/types.h"
#include "include/tracingtypes.h"
#include "structures/tracingjob.h"
#include "filehandler/checkreadtracingjob.h"
#include "isotracingoperator.h"
#include "vtitracingoperator.h"
#include "ttitracingoperator.h"
#include "raytracer.h"
#include "wavefronttracer.h"
#include "outputclass.h"
#include "smoothingclass.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "include/defs.h"
#include <sys/sysinfo.h>

#ifdef PARALLEL_VM
#include "utils/VM_ParallelEnvironment.h"
#endif
#ifdef PARALLEL_DUMMY
#include "3D-GRT/Dummy_ParallelEnvironment.h"
#endif




/* Print welcome message */
void welcome()
{
  
  
  
  
  
  
  
}

/* Print welcome message to log file*/
void filewelcome()
{
  
  
  
  
  
  
  
}

/* Print usage description to stdout */
void helpmessage(char* binary_name)
{
  
  
  
  
  
  
  
  
}




int main(int argc, char *argv[])
{
    welcome();
    int ierr = 0;

    char* CfgFileNameTmp = NULL;

    char CurrentDirectory[199];
    getcwd(CurrentDirectory, 199);

    char ExecutableName[199];
    sprintf(ExecutableName, "%s", argv[0]);
	
    bool PV4D_VM_WORKER = false;
    bool VMEXPERT =  false;
    unsigned int VMport = 0;

    unsigned long MemSize = (unsigned long) (1024*1024);

    TracingJob Job;
    CheckReadTracingJob CheckRead;

    // Read command line
    opterr = 0;  // suppress any error message about unknown options
    optind = 0;  // start with the search at the beginning
    int c;
    while ((c = getopt(argc, argv, "+hf:s:t:m:p:x")) != -1)
    {
	switch (c) {
	    case 'h':
		helpmessage ((argv[0]));
		exit(0);
		break;
	    case 'f':
	    {
		if ( CfgFileNameTmp == NULL)
		    CfgFileNameTmp = new char[199];
		int i = 0;
		char* pointer = (char*) optarg;
		while ( (*pointer != (char)0) && (i < 199-1) )
		{
		    CfgFileNameTmp[i] = *pointer;
		    i++;
		    pointer++;
		}
		CfgFileNameTmp[i] = (char)0;
		break;
	    }
	    case 't':
		PV4D_VM_WORKER = true;
		break;
 	    case 'm':
 	    {
		
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

   
    /* Initialization phase on the headnode */
    if ( !PV4D_VM_WORKER )
    {
	// Check for configuration file
	if (CfgFileNameTmp == NULL)
	{
	    
	    helpmessage (basename(argv[0]));
	    exit(1);
	}
	
	// Add full path to configuration file name
	if ( strncmp("/", CfgFileNameTmp, 1) != 0)
	{
	    char TmpFileName[199];
	    sprintf(TmpFileName, "%s/%s", CurrentDirectory, CfgFileNameTmp);
	    sprintf(CfgFileNameTmp, "%s", TmpFileName);
	}
	
	// Add full path to executable name
	if ( strncmp("/", argv[0], 1) != 0)
	{
	    char TmpFileName[199];
	    sprintf(ExecutableName, "%s/%s", CurrentDirectory, argv[0]);
	}
	
	// Read and check the job description in the configuration file
	if ( CheckRead.ReadConfigFileXML(CfgFileNameTmp, Job) != 0 )
	{
	    exit(-1);
	}

	CheckRead.AddWorkingDir(CurrentDirectory, Job);
	CheckRead.Scale(Job);
	LOGGING_MODE = Job.LogLevel;
	LOGGING_FILE_CREATE(Job.LogFile);
	filewelcome();
	
	

	if (Job.PreprocessingVel || Job.PreprocessingAni)
	{
	    int ierr = 0;
	    
	    SmoothingClass Smoother;
	    point3D<int> Coarsening(Job.CoarseningFactoralongInline, Job.CoarseningFactoralongXline, Job.CoarseningFactoralongDepth);
	    if (Job.PreprocessingVel)
		ierr += Smoother.SmoothInverse(Job.VelFileName, Job.SmoothedVelFileName, Job.PropFileMode, 
					       Job.NVel, Job.dxVel, 
					       Job.SmoothingLengthVel, Coarsening);

	    if (Job.PreprocessingAni && ((Job.IsoMode == VTI) || (Job.IsoMode == TTI)) )
	    {
		ierr += Smoother.SmoothLinear(Job.VTI_EpsilonFileName, Job.SmoothedVTI_EpsilonFileName, Job.PropFileMode, 
					      Job.NVel, Job.dxVel, 
					      Job.SmoothingLengthAni, Coarsening);
		ierr += Smoother.SmoothLinear(Job.VTI_DeltaFileName, Job.SmoothedVTI_DeltaFileName, Job.PropFileMode, 
					      Job.NVel, Job.dxVel, 
					      Job.SmoothingLengthAni, Coarsening);
	    }
	    if (ierr != 0)
	    {
		
		exit(ierr);
	    }
	    else
		

	    if (Job.ModelPreprocessingOnly)
		exit(0);
	    else
	    {
		// copy file names of smoothed models to file names of models
		if (Job.PreprocessingVel)
		    sprintf(Job.VelFileName, "%s", Job.SmoothedVelFileName);
		if (Job.PreprocessingAni)
		{
		    sprintf(Job.VTI_EpsilonFileName, "%s", Job.SmoothedVTI_EpsilonFileName);
		    sprintf(Job.VTI_DeltaFileName, "%s", Job.SmoothedVTI_DeltaFileName);
		}
		Job.n_inlines_Vel = (int)((Job.n_inlines_Vel + Job.CoarseningFactoralongXline - 1)/Job.CoarseningFactoralongXline);
		Job.n_xlines_Vel = (int)((Job.n_xlines_Vel + Job.CoarseningFactoralongInline - 1)/Job.CoarseningFactoralongInline);
		Job.NVel[0] = (int)((Job.NVel[0] + Coarsening[0] - 1)/Coarsening[0]);
		Job.NVel[1] = (int)((Job.NVel[1] + Coarsening[1] - 1)/Coarsening[1]);
		Job.NVel[2] = (int)((Job.NVel[2] + Coarsening[2] - 1)/Coarsening[2]);

		Job.d_between_inlines_Vel *= Job.CoarseningFactoralongXline;
		Job.d_between_xlines_Vel *= Job.CoarseningFactoralongInline;
		Job.dxVel[0] *= Coarsening[0];
		Job.dxVel[1] *= Coarsening[1];
		Job.dxVel[2] *= Coarsening[2];
	    }
	}

	// Check Machine File and count the nodes
	const int NodeCount = ParallelEnvironment::GetAllocatedNodeCount();
	if (NodeCount < 1)
	{
	    
	    exit(-1);
	}

	switch(Job.RunMode)
	{
	    case WAVEFRONTTRACER:
	    {
		// Check if all Receivers are inside of the Velocity Model
		bool outside = false;
		for (int i = 0; i < 3; i++)
		{
		    if (Job.dxVel[i] > 0)
			for (int iRcv = 0; iRcv < Job.NRcv[i]; iRcv++)
			{
			    if (   ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) < (Job.X0Vel[i]))
				   || ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) >= (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]))  )
				outside = true;
			}      
		    else        
			for (int iRcv = 0; iRcv < Job.NRcv[i]; iRcv++)
			{
			    if (   ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) >= (Job.X0Vel[i]))
				   || ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) < (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]))   )
				outside = true;
			}      
		}
		if (outside)
		{
		    
		    ierr = 0;
		}

		
		break;
	    }
	    case RAYTRACER:
	    {
		// Memory check
	        unsigned long MemoryNeeded(0);
                bool ValidGrid = false;
		switch(Job.IsoMode)
		  {
		  case ISOTROPIC:
		    {
		      RayTracer< isoTracingOperator<PropModel<VelGridPointBSpline> > > Tracer;
		      MemoryNeeded = Tracer.GetMemoryNeeded(Job);
		      ValidGrid = Tracer.CheckTTGrid(Job);
		      break;
		    }
		  case VTI:
		    {
		      RayTracer< vtiTracingOperator<PropModel<VelGridPointBSpline> > > Tracer;
		      MemoryNeeded = Tracer.GetMemoryNeeded(Job);
		      ValidGrid = Tracer.CheckTTGrid(Job);
		      break;
		    }
		  case TTI:
		    {
		      RayTracer< ttiTracingOperator<PropModel<VelGridPointBSpline> > > Tracer;
		      MemoryNeeded = Tracer.GetMemoryNeeded(Job);
		      ValidGrid = Tracer.CheckTTGrid(Job);
		      break;
		    }
		  default:
		    
		  }

		{
		    struct sysinfo info;
		    sysinfo(&info);
		    unsigned long TotalRam = info.totalram;
		    
		    if ( MemoryNeeded > TotalRam)
		    {
			
			
			
			
			
			
			exit(-1);
		    }
		}
		
		// Check if all Sources are inside of the Velocity Model
		{
		    if (!ValidGrid)
			exit(-1);
		}
		
		
		break;
	    }
	    default:
		
	}

	
    }

    
  /* Starting the parallel execution */
    ParallelEnvironment PE(argc, argv, NULL, MemSize, ierr, VMport, !VMEXPERT);
    if (ierr != 0)
    {
	
	exit(-1);
    }
    PE.ExitOnError(ierr);
    
  
    int PSize = PE.GetNodeCount(); 
    int PRank = PE.GetRank(); 
    
    /* Broadcast the Tracer Job Description */
    if ( PE.GetRank() == 0)
    {
	for (int idest = 1; idest < PE.GetNodeCount(); idest++)
	    PE.Send((char*) &Job, sizeof(Job), idest);
    }
    else
    {
	PE.Receive((char*) &Job, sizeof(Job), 0);

	// Set Logging
	LOGGING_MODE = -1;
	LOGGING_FILE(Job.LogFile);
    }
    PE.Barrier();
  



    PE.ExitOnError(ierr);

  //LOGGING_MODE = Job.LogLevel;
    OutputClass Output;

    switch(Job.RunMode)
    {
	case WAVEFRONTTRACER:
	{
	    PropModel< VelGridPointBSpline >* VelModel = new PropModel< VelGridPointBSpline >(Job.VelFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
	    WaveFrontTracer< isoTracingOperator<PropModel<VelGridPointBSpline> > > Tracer(Job, ierr);
	    isoTracingOperator<PropModel<VelGridPointBSpline> >::VelModelRep_t VMod(VelModel);
	    Tracer.BindVelModel(VMod);
	    PE.ExitOnError(ierr);
	
	    Tracer.execute(Job, &PE, ierr);
//       int proc = 0;
//       TracingJob SubJob(Job);
//       for (int nx = Job.N0Src[0]; nx < Job.N0Src[0]+Job.NSrc[0]; nx++)
//         for (int ny = Job.N0Src[1]; ny < Job.N0Src[1]+Job.NSrc[1]; ny++)
//           for (int nz = Job.N0Src[2]; nz < Job.N0Src[2]+Job.NSrc[2]; nz++)
//             {
//               if (proc == PRank)
//                 {
//                   SubJob.N0Src[0] = nx;
//                   SubJob.N0Src[1] = ny;
//                   SubJob.N0Src[2] = nz;
                  
//                   SubJob.NSrc[0] = 1;
//                   SubJob.NSrc[1] = 1;
//                   SubJob.NSrc[2] = 1;
                  
//                   //Tracer.run_1s(Job, Signals);
//                   //Tracer.run_1ray(SubJob);
//                   //Tracer.run(SubJob);

// 		  Tracer.execute(SubJob, &PE, ierr);
//                 }
              
//               if (++proc == PSize)
//                 proc = 0;
//             }
	    delete VelModel;
	    break;
	}
	case RAYTRACER:
	{
	    bool commerror = false;
	    bool first_try = true;
	    while (commerror || first_try)
	    {
	      switch(Job.IsoMode)
		{
		case ISOTROPIC:
		  {
		    PropModel<VelGridPointBSpline>* VelModel = new PropModel<VelGridPointBSpline>(Job.VelFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    RayTracer< isoTracingOperator<PropModel<VelGridPointBSpline> > > Tracer(Job, ierr);
		    isoTracingOperator<PropModel<VelGridPointBSpline> >::VelModelRep_t VMod(VelModel);
		    Tracer.BindVelModel(VMod);
		    PE.ExitOnError(ierr);
		    
		    if (PRank == 0)
		      
		    
		    ierr = Tracer.execute(Job, &PE);
		    
		    first_try = false;
		    
		    commerror = !PE.Recover();
		    PE.Barrier();
		    if (commerror)
		      {
			
			Job.g_restart = true;
		      }
		    delete VelModel;
		    break;
		  }
		case VTI:
		  {
		    PropModel<VelGridPointBSpline>* VelModel = new PropModel<VelGridPointBSpline>(Job.VelFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointBSpline>* VhModel = new PropModel<VelGridPointBSpline>(1, Job.VelFileName, Job.VTI_EpsilonFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointBSpline>* ExModel = new PropModel<VelGridPointBSpline>(Job.VelFileName, Job.VTI_EpsilonFileName, Job.VTI_DeltaFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    RayTracer< vtiTracingOperator<PropModel<VelGridPointBSpline> > > Tracer(Job, ierr);
		    vtiTracingOperator<PropModel<VelGridPointBSpline> >::VelModelRep_t VMod(VelModel, VhModel, ExModel);
		    Tracer.BindVelModel(VMod);
		    PE.ExitOnError(ierr);
		    
		    if (PRank == 0)
		      
		    
		    ierr = Tracer.execute(Job, &PE);
		    
		    first_try = false;
		    
		    commerror = !PE.Recover();
		    PE.Barrier();
		    if (commerror)
		      {
			
			Job.g_restart = true;
		      }
		    delete VelModel;
		    delete VhModel;
		    delete ExModel;
		    break;
		  }
		case TTI:
		  {
		    PropModel<VelGridPointBSpline>* VelModel = new PropModel<VelGridPointBSpline>(Job.VelFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointBSpline>* VhModel = new PropModel<VelGridPointBSpline>(1, Job.VelFileName, Job.VTI_EpsilonFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointBSpline>* ExModel = new PropModel<VelGridPointBSpline>(Job.VelFileName, Job.VTI_EpsilonFileName, Job.VTI_DeltaFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointLinear>* TTIAlphaModel = new PropModel<VelGridPointLinear>(Job.TTI_AlphaFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    PropModel<VelGridPointLinear>* TTIBetaModel = new PropModel<VelGridPointLinear>(Job.TTI_BetaFileName, Job.PropFileMode, Job.X0Vel, Job.NVel, Job.dxVel, Job.geom, ierr);
		    RayTracer< ttiTracingOperator<PropModel<VelGridPointBSpline> > > Tracer(Job, ierr);
		    ttiTracingOperator<PropModel<VelGridPointBSpline> >::VelModelRep_t VMod(VelModel, VhModel, ExModel, TTIAlphaModel, TTIBetaModel);
		    Tracer.BindVelModel(VMod);
		    PE.ExitOnError(ierr);
		    
		    if (PRank == 0)
		      
		    
		    ierr = Tracer.execute(Job, &PE);
		    
		    first_try = false;
		    
		    commerror = !PE.Recover();
		    PE.Barrier();
		    if (commerror)
		      {
			
			Job.g_restart = true;
		      }
		    delete VelModel;
		    delete VhModel;
		    delete ExModel;
		    delete TTIAlphaModel;
		    delete TTIBetaModel;
		    break;
		  }
		default:
		  
		}
	    }
	    break;
	}
	default:
	    
    }
      
    PE.Barrier();

    if (PRank == 0)
	

    return EXIT_SUCCESS;
}
