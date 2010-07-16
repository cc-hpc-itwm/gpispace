/**************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Fri Nov 11 10:07:59 CET 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <math.h>
#include "../types/types.h"
#include "../utils/file_read.h"
#include "../structures/tracingjob.h"
#include "wavefronttracer.h"
#include "outputclass.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>


void output(rayMem& rArray, int n, int index)
{
  char* filename = new char[199];
  sprintf(filename, "Rays_%d.gnu", index);
  std::ofstream outfile(filename);

  for (int i = 0; i < n; i++)
    {
      outfile << rArray[i].x[0] << "  " << rArray[i].x[1] << "  " << rArray[i].x[2] << std::endl;
      outfile << rArray[i].x[0]+rArray[i].p[0] << "  "
              << rArray[i].x[1]+rArray[i].p[1] << "  "
              << rArray[i].x[2]+rArray[i].p[2] << std::endl;
      outfile << "\n\n";
    }
  outfile.close();
    
}


void output1(rayMem& rArray, int n)
{
  char* filename = new char[199];
  sprintf(filename, "Rays.gnu");
  std::ofstream outfile(filename, std::ios::app);

  for (int i = 0; i < n; i++)
    {
      outfile << i << " : " << rArray[i].x[0] << "  " << rArray[i].x[1] << "  " << rArray[i].x[2] << std::endl;
    }
  outfile.close();

}

void helpmessage(char* binary_name)
{
  printf ("Usage: %s [-options]\n", binary_name);
}


int main(int argc, char *argv[])
{
  //##################################################################################
  //#
  //# Initializing numbers for the geometry  
  //#

  TracingJob Job;


  char* CfgFileName = NULL;

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
        return 0;
      case 'f':
        delete[] CfgFileName;
        CfgFileName = optarg;
      break;
      default:
        break;
    }
  }


  // Read the configuration file
  if (CfgFileName == NULL)
  {
    std::cerr << "FATAL ERROR: No configuration file has been specified.\n";
    exit(1);
  }  
  std::cout << "Reading configuration file " << CfgFileName << std::endl;

  // Read the velocity grid
  const float X0Velx = file_read_float(CfgFileName, "X0Velx");
  const float X0Vely = file_read_float(CfgFileName, "X0Vely");
  const float X0Velz = file_read_float(CfgFileName, "X0Velz");
  if ( (X0Velx == FILE_KEY_NOT_FOUND) || (X0Vely == FILE_KEY_NOT_FOUND) || (X0Velz == FILE_KEY_NOT_FOUND) ) 
  {
    std::cout << X0Velx << " " << X0Vely << " " << X0Velz << std::endl;
    std::cerr << "FATAL ERROR: X0Velx, X0Vely, X0Velz have not been found in the configuration file.\n";
    exit(1);
  }
  Job.X0Vel = point3D<float>(X0Velx, X0Vely, X0Velz);

  const int NxVel = file_read_int(CfgFileName, "NxVel");
  const int NyVel = file_read_int(CfgFileName, "NyVel");
  const int NzVel = file_read_int(CfgFileName, "NzVel");
  if ( (NxVel == FILE_KEY_NOT_FOUND) || (NyVel == FILE_KEY_NOT_FOUND) || (NzVel == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: NxVel, NyVel, NzVel have not been found in the configuration file.\n";
    exit(1);
  }
  Job.NVel = point3D<int>(NxVel, NyVel, NzVel);

  const float dxVel = file_read_float(CfgFileName, "dxVel");
  const float dyVel = file_read_float(CfgFileName, "dyVel");
  const float dzVel = file_read_float(CfgFileName, "dzVel");
  if ( (dxVel == FILE_KEY_NOT_FOUND) || (dyVel == FILE_KEY_NOT_FOUND) || (dzVel == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: dxVel, dyVel, dzVel have not been found in the configuration file.\n";
    exit(1);
  }
  Job.dxVel = point3D<float>(dxVel, dyVel, dzVel);

  // Read file names
  if ( file_read_string(CfgFileName, "VelFileName", Job.VelFileName) == FILE_KEY_NOT_FOUND)
  {
    std::cerr << "FATAL ERROR: VelFileName has not been found in the configuration file.\n";
    exit(1);
  }

  // Read the source grid
  const float X0Srcx = file_read_float(CfgFileName, "X0Srcx");
  const float X0Srcy = file_read_float(CfgFileName, "X0Srcy");
  const float X0Srcz = file_read_float(CfgFileName, "X0Srcz");
  if ( (X0Srcx == FILE_KEY_NOT_FOUND) || (X0Srcy == FILE_KEY_NOT_FOUND) || (X0Srcz == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: X0Srcx, X0Srcy, X0Srcz have not been found in the configuration file.\n";
    exit(1);
  }
  Job.X0Src = point3D<float>(X0Srcx, X0Srcy, X0Srcz);

  const int NxSrc = file_read_int(CfgFileName, "NxSrc");
  const int NySrc = file_read_int(CfgFileName, "NySrc");
  const int NzSrc = file_read_int(CfgFileName, "NzSrc");
  if ( (NxSrc == FILE_KEY_NOT_FOUND) || (NySrc == FILE_KEY_NOT_FOUND) || (NzSrc == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: NxSrc, NySrc, NzSrc have not been found in the configuration file.\n";
    exit(1);
  }
  Job.NSrc = point3D<int>(NxSrc, NySrc, NzSrc);

  const float dxSrc = file_read_float(CfgFileName, "dxSrc");
  const float dySrc = file_read_float(CfgFileName, "dySrc");
  const float dzSrc = file_read_float(CfgFileName, "dzSrc");
  if ( (dxSrc == FILE_KEY_NOT_FOUND) || (dySrc == FILE_KEY_NOT_FOUND) || (dzSrc == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: dxSrc, dySrc, dzSrc have not been found in the configuration file.\n";
    exit(1);
  }
  Job.dxSrc = point3D<float>(dxSrc, dySrc, dzSrc);


  // Read the receiver grid
  const float X0Rcvx = file_read_float(CfgFileName, "X0Rcvx");
  const float X0Rcvy = file_read_float(CfgFileName, "X0Rcvy");
  const float X0Rcvz = file_read_float(CfgFileName, "X0Rcvz");
  if ( (X0Rcvx == FILE_KEY_NOT_FOUND) || (X0Rcvy == FILE_KEY_NOT_FOUND) || (X0Rcvz == FILE_KEY_NOT_FOUND) )
  {
    std::cout << X0Rcvx << " " << X0Rcvy << " " << X0Rcvz << std::endl;
    std::cerr << "FATAL ERROR: X0Rcvx, X0Rcvy, X0Rcvz have not been found in the configuration file.\n";
    exit(1);
  }
  Job.X0Rcv = point3D<float>(X0Rcvx, X0Rcvy, X0Rcvz);

  const int NxRcv = file_read_int(CfgFileName, "NxRcv");
  const int NyRcv = file_read_int(CfgFileName, "NyRcv");
  const int NzRcv = file_read_int(CfgFileName, "NzRcv");
  std::cout << NzRcv << std::endl;
  if ( (NxRcv == FILE_KEY_NOT_FOUND) || (NyRcv == FILE_KEY_NOT_FOUND) || (NzRcv == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: NxRcv, NyRcv, NzRcv have not been found in the configuration file.\n";
    exit(1);
  }
  Job.NRcv = point3D<int>(NxRcv, NyRcv, NzRcv);

  const float dxRcv = file_read_float(CfgFileName, "dxRcv");
  const float dyRcv = file_read_float(CfgFileName, "dyRcv");
  const float dzRcv = file_read_float(CfgFileName, "dzRcv");
  if ( (dxRcv == FILE_KEY_NOT_FOUND) || (dyRcv == FILE_KEY_NOT_FOUND) || (dzRcv == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: dxRcv, dyRcv, dzRcv have not been found in the configuration file.\n";
    exit(1);
  }
  Job.dxRcv = point3D<float>(dxRcv, dyRcv, dzRcv);

  // Read some constants
  Job.g_MAXTSTEP = file_read_int(CfgFileName, "MAXTSTEP");
  if (Job.g_MAXTSTEP == FILE_KEY_NOT_FOUND)
  {
      std::cerr << "FATAL ERROR: MAXTSTEP has not been found in the configuration file.\n";
    exit(1);
  }

  Job.g_RAY_MIN = file_read_int(CfgFileName, "RAY_MIN");
  if (Job.g_RAY_MIN == FILE_KEY_NOT_FOUND)
  {
      std::cerr << "FATAL ERROR: RAY_MIN has not been found in the configuration file.\n";
    exit(1);
  }

  Job.g_REF_LEN = file_read_int(CfgFileName, "REF_LEN");
  if (Job.g_REF_LEN == FILE_KEY_NOT_FOUND)
  {
      std::cerr << "FATAL ERROR: REF_LEN has not been found in the configuration file.\n";
    exit(1);
  }

  Job.g_TSTEPSIZE = file_read_float(CfgFileName, "TSTEPSIZE");
  if (Job.g_TSTEPSIZE == FILE_KEY_NOT_FOUND)
  {
      std::cerr << "FATAL ERROR: TSTEPSIZE has not been found in the configuration file.\n";
    exit(1);
  }

  Job.g_TRACINGSTEPS = file_read_int(CfgFileName, "TRACINGSTEPS");
  if (Job.g_TRACINGSTEPS == FILE_KEY_NOT_FOUND)
  {
      std::cerr << "FATAL ERROR: TRACINGSTEPS has not been found in the configuration file.\n";
    exit(1);
  }

  // Check if all Sources are inside of the Velocity Model
  bool outside = false;
  for (int i = 0; i < 3; i++)
  {
    if (Job.dxVel[i] > 0)
      for (int iSrc = 0; iSrc < Job.NSrc[i]; iSrc++)
      {
        if (   ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) < (Job.X0Vel[i]))
            || ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) >= (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i])) )
            outside = true;

        std::cout << (Job.X0Src[i] + iSrc*Job.dxSrc[i]) << " <? " << (Job.X0Vel[i]) << std::endl;     
        std::cout << (Job.X0Src[i] + iSrc*Job.dxSrc[i]) << " >=? " << (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]) << std::endl;
      }      
    else        
      for (int iSrc = 0; iSrc < Job.NSrc[i]; iSrc++)
      {
        if (   ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) >= (Job.X0Vel[i]))
            || ((Job.X0Src[i] + iSrc*Job.dxSrc[i]) < (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]))  )
            outside = true;

        std::cout << (Job.X0Src[i] + iSrc*Job.dxSrc[i]) << " >=? " << (Job.X0Vel[i]) << std::endl;
        std::cout << (Job.X0Src[i] + iSrc*Job.dxSrc[i]) << " <? " << (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]) << std::endl;
      }
  }
  if (outside)
  {
    std::cerr << "FATAL ERROR: Some Sources are outside of the velocity model .\n";
    exit(1);
  }

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

        std::cout << (Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) << " <? " << (Job.X0Vel[i]) << std::endl;
        std::cout << (Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) << " >=? " << (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]) << std::endl;
      }      
    else        
      for (int iRcv = 0; iRcv < Job.NRcv[i]; iRcv++)
      {
        if (   ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) >= (Job.X0Vel[i]))
            || ((Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) < (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]))   )
            outside = true;

        std::cout << (Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) << " >=? " << (Job.X0Vel[i]) << std::endl;
        std::cout << (Job.X0Rcv[i] + iRcv*Job.dxRcv[i]) << " <? " << (Job.X0Vel[i] + (Job.NVel[i])*Job.dxVel[i]) << std::endl;
      }      
  }
  if (outside)
  {
    std::cerr << "FATAL ERROR: Some Receivers are outside of the velocity model .\n";
    exit(1);
  }


//  // Init the volume grid to the MinMax of the Source and Receiver Coordinates
//  point3D<float> XMinSrc, XMaxSrc;
//  point3D<float> XMinRcv, XMaxRcv;
//  point3D<float> XMinVol, XMaxVol;
//  for (int i = 0; i < 3; i++)
//  {
//    if (Job.dxSrc[i] > 0)
//      {
//        XMaxSrc[i] = Job.X0Src[i] + (Job.NSrc[i]-1)*Job.dxSrc[i];
//        XMinSrc[i] = Job.X0Src[i];
//      }
//    else
//      {
//        XMinSrc[i] = Job.X0Src[i] + (Job.NSrc[i]-1)*Job.dxSrc[i];
//        XMaxSrc[i] = Job.X0Src[i];
//      }
//    if (Job.dxRcv[i] > 0)
//      {
//        XMaxRcv[i] = Job.X0Rcv[i] + (Job.NRcv[i]-1)*Job.dxRcv[i];
//        XMinRcv[i] = Job.X0Rcv[i];
//      }
//    else
//      {
//        XMinRcv[i] = Job.X0Rcv[i] + (Job.NRcv[i]-1)*Job.dxRcv[i];
//        XMaxRcv[i] = Job.X0Rcv[i];
//      }
//
//    XMinVol[i] = (XMinRcv[i]<XMinSrc[i])?XMinRcv[i]:XMinSrc[i];
//    XMaxVol[i] = (XMaxRcv[i]>XMaxSrc[i])?XMaxRcv[i]:XMaxSrc[i];
//
//    Job.X0Vol[i] = XMinVol[i] - 2*abs(Job.dxVel[i]);
//    Job.X1Vol[i] = XMaxVol[i] + 2*abs(Job.dxVel[i]);
//  }

    // Read the volume grid
  const float X0x = file_read_float(CfgFileName, "X0Volx");
  const float X0y = file_read_float(CfgFileName, "X0Voly");
  const float X0z = file_read_float(CfgFileName, "X0Volz");
  if ( (X0x == FILE_KEY_NOT_FOUND) || (X0y == FILE_KEY_NOT_FOUND) || (X0z == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: X0x, X0y, X0z have not been found in the configuration file.\n";
    exit(1);
  }
  Job.X0Vol = point3D<float>(X0x, X0y, X0z);

  const float X1x = file_read_float(CfgFileName, "X1Volx");
  const float X1y = file_read_float(CfgFileName, "X1Voly");
  const float X1z = file_read_float(CfgFileName, "X1Volz");
  if ( (X1x == FILE_KEY_NOT_FOUND) || (X1y == FILE_KEY_NOT_FOUND) || (X1z == FILE_KEY_NOT_FOUND) )
  {
    std::cerr << "FATAL ERROR: X0x, X0y, X0z have not been found in the configuration file.\n";
    exit(1);
  }
  Job.X1Vol = point3D<float>(X1x, X1y, X1z);

  
  OutputClass Output;
  Output.PrintJob(Job);

  WaveFrontTracer Tracer(Job);

  RecSig* Signals = new RecSig[Job.NRcv[0]*Job.NRcv[1]*Job.NRcv[2]];
  Tracer.run_1s(Job, Signals);
  //Tracer.run(Job);
//  for (int i = 0; i < Job.NRcv[0]*Job.NRcv[1]*Job.NRcv[2]; i++)
//    std::cout << (Signals[i].arrtime) << std::endl;
    
  return EXIT_SUCCESS;
}
