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
***************************************************************************/
#define VERSION "1.0"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/types.h"
#include "include/tracingtypes.h"
#include "utils/file_read.h"
#include "utils/ParallelMode.h"
#include "structures/tracingjob.h"
#include "filehandler/checkreadtracingjob.h"
#include "raytracer.h"
#include "wavefronttracer.h"
#include "outputclass.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "include/defs.h"



/* Print welcome message */
void welcome()
{
  
  
  
  
  
}

/* Print usage description to stdout */
void helpmessage(char* binary_name)
{
  std::cout << "Usage: " << binary_name << "  [-options]\n";
  std::cout << "\n\n";
  std::cout << "-h                              print this help message\n";
  std::cout << "-f 'CfgFile'                    read parameters from File CfgFile\n";  
}

/* Exit program */
void checkexit(const int PRank, int errcode)
{
  if (PRank == 0)
    {
      MPI_Bcast ( &errcode, 1, MPI_INT, 0, MPI_COMM_WORLD );
      if (errcode != 0)
        exit(errcode);
    }
  else
    {
	int errcode0;
      MPI_Bcast ( &errcode0, 1, MPI_INT, 0, MPI_COMM_WORLD );
      if (errcode0 != 0)
        exit(errcode0);
    }
}

/* Exit program */
void fatalexit()
{
  int errcode = 1;
  MPI_Bcast ( &errcode, 1, MPI_INT, 0, MPI_COMM_WORLD );
  exit(errcode);
}

/* Print fatal error message and exit */
void fatalerrorexit(char* message)
{
  std::cerr << "\nFATAL ERROR: " << message << std::endl; 
  fatalexit();
}



int main(int argc, char *argv[])
{
    LOGGING_MODE = 3;
    int ierr = 0;

  /* Starting the parallel execution */
  ParallelMode P(argc, argv);
  int PSize = P.get_nproc(); 
  int PRank = P.get_rank(); 

  char* CfgFileName = NULL;
  TracingJob Job;

  /* Read the command line and the configuration file on Rank 0 */
  if (PRank == 0)
    {
      welcome();

      // Read command line
      opterr = 0;  // suppress any error message about unknown options
      optind = 0;  // start with the search at the beginning

      int c;
      int argval;
      while ((c = getopt(argc, argv, "hf:")) != -1)
        {
          switch (c) {
          case 'h':
            helpmessage ((argv[0]));
            ierr = 1;
          case 'f':
            delete[] CfgFileName;
            CfgFileName = optarg;
            break;
          default:
            break;
          }
        }

	if (CfgFileName == NULL) // Check for configuration file
	{
	    
	    helpmessage (basename(argv[0]));
	    ierr = 1;
	}
    }
  checkexit(PRank, ierr);


  if (PRank == 0)
    {
      //##################################################################################
      //#
      //# Initializing numbers for the geometry  
      //#

      CheckReadTracingJob JobReader;
      if ( JobReader.ReadConfigFileXML(CfgFileName, Job) != 0)
        {
          
	  ierr = 1;
        };
      JobReader.WriteConfigFileXML("CfgFileTest.xml", Job);

      // Check if all Sources are inside of the Velocity Model
        bool outside = false;
        for (int i = 0; i < 3; i++)
          {
            if (Job.dxVel[i] > 0)
              for (int iSrc = 0; iSrc < Job.NSrc[i]; iSrc++)
                {
                  if (   ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) < (Job.X0Vel[i] - 0.1))
                       || ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) >= (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i] + 0.1)) )
                    outside = true;
		  if (outside)
		  {
		      std::cout << i << ", " << iSrc << " : " << Job.X0Vel[i] << " < " << Job.X0Src[i] + iSrc*Job.dxSrc[i] << " < " << Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i] << std::endl;
		      break;
		  }
                }      
            else        
              for (int iSrc = 0; iSrc < Job.NSrc[i]; iSrc++)
                {
                  if (   ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) >= (Job.X0Vel[i]))
                       || ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) < (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]))  )
                    outside = true;
                }
	    if (outside)
		break;
          }
      if (outside)
        {
          
	  ierr = 1;
        }

      if (Job.RunMode == WAVEFRONTTRACER)
      {
        // Check if all Receivers are inside of the Velocity Model
        outside = false;
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
      }
      if (outside)
        {
          
	  ierr = 1;
        }


    }

  checkexit(PRank, ierr);

  /* Broadcast the Tracer Job Description */
  MPI_Bcast(&Job, sizeof(Job), MPI_CHAR, 0, MPI_COMM_WORLD);
  
  LOGGING_MODE = Job.LogLevel;
  OutputClass Output;

  switch(Job.RunMode)
  {
  case WAVEFRONTTRACER:
    {
      WaveFrontTracer Tracer(Job);

      int proc = 0;
      TracingJob SubJob(Job);
      for (int nx = Job.N0Src[0]; nx < Job.N0Src[0]+Job.NSrc[0]; nx++)
        for (int ny = Job.N0Src[1]; ny < Job.N0Src[1]+Job.NSrc[1]; ny++)
          for (int nz = Job.N0Src[2]; nz < Job.N0Src[2]+Job.NSrc[2]; nz++)
            {
              if (proc == PRank)
                {
                  SubJob.N0Src[0] = nx;
                  SubJob.N0Src[1] = ny;
                  SubJob.N0Src[2] = nz;
                  
                  SubJob.NSrc[0] = 1;
                  SubJob.NSrc[1] = 1;
                  SubJob.NSrc[2] = 1;
                  
                  RecSig* Signals = new RecSig[Job.NRcv[0]*Job.NRcv[1]*Job.NRcv[2]];
                  //Tracer.run_1s(Job, Signals);
                  //Tracer.run_1ray(SubJob);
                  Tracer.run(SubJob);
                }
              
              if (++proc == PSize)
                proc = 0;
            }
      break;
    }
  case RAYTRACER:
    {
      RayTracer Tracer(Job);
      
      if (PRank == 0)
      {
	  TTAngFileHandler TTAFH(Job.TTDirName, Job.TTFilePrefix, ierr);
	  ierr = TTAFH.WriteXML(Job);
	  for (int nz = Job.N0Src[2]; nz < Job.N0Src[2]+Job.NSrc[2]; nz++)
	      TTAFH.TouchAngSig(0, 0, nz, Job.NtotSrc[1]);
      }
      MPI_Barrier(MPI_COMM_WORLD);      

      
      int proc = 0;
      TracingJob SubJob(Job);
      for (int nx = Job.N0Src[0]; nx < Job.N0Src[0]+Job.NSrc[0]; nx++)
        for (int ny = Job.N0Src[1]; ny < Job.N0Src[1]+Job.NSrc[1]; ny++)
          for (int nz = Job.N0Src[2]; nz < Job.N0Src[2]+Job.NSrc[2]; nz++)
            {
              if (proc == PRank)
                {
                  SubJob.N0Src[0] = nx;
                  SubJob.N0Src[1] = ny;
                  SubJob.N0Src[2] = nz;
                  
                  SubJob.NSrc[0] = 1;
                  SubJob.NSrc[1] = 1;
                  SubJob.NSrc[2] = 1;
                  
                  //Output.PrintJob(SubJob);
                  
                  Tracer.run_old(SubJob);
                 }
              
              if (++proc == PSize)
                proc = 0;
             }
      
      break;
    }
  default:
    std::cerr << "RunMode " << Job.RunMode << " not known.\n" << std::flush;
  }
      
  return EXIT_SUCCESS;
}
