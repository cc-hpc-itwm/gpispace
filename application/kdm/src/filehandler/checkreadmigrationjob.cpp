/***************************************************************************
                          checkreadmigrationjob.cpp  -  description

    Read the variables for the migration job from the given
    configuration file and check for consitency of the data.
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

/**
  *@author Dirk Merten
  */

#include "checkreadmigrationjob.h"

int CheckReadMigrationJob::ReadConfigFileXML(char* ConfigFileName, MigrationJob& Job)
{
  if ( ConfigFileName == NULL )
    {
      //
      return -1;
    }

  CfgFileName = ConfigFileName;
  //
  //

  if ( CheckExistence(CfgFileName) == -1)
  {
    //
      return -1;
  }
  XMLReader Reader;
  Reader.setFile(ConfigFileName);

  bool success = true;

  ReadXML(Reader, "common/Parameters/ProjectName", Job.ProjectName, false);
  success = success && ReadXML(Reader, "common/Geometry/one_inline_num", Job.geom.one_inline_num);
  success = success && ReadXML(Reader, "common/Geometry/one_xline_num", Job.geom.one_xline_num);
  success = success && ReadXML(Reader, "common/Geometry/one_x", Job.geom.x_one);
  success = success && ReadXML(Reader, "common/Geometry/one_y", Job.geom.y_one);
  success = success && ReadXML(Reader, "common/Geometry/dy_between_inlines", Job.geom.d_between_inlines);
  success = success && ReadXML(Reader, "common/Geometry/dx_between_xlines", Job.geom.d_between_xlines);
  success = success && ReadXML(Reader, "common/Geometry/angle_rot_deg", Job.geom.angle_rot_deg.v);
  success = success && ReadXML(Reader, "common/Geometry/inlinedir", Job.geom.inlinedir);

  if (!success)
    return -1;

  char LoggingDir[199];
  if (!ReadXML(Reader, "common/Parameters/LoggingDirectory", LoggingDir, false))
     sprintf( LoggingDir, "%s", "");

  if (!ReadXML(Reader, "Migration/Parameters/jobname_M", Job.JobName, false))
  {
    sprintf( Job.JobName, "%s", "");
  }

  if (strlen(LoggingDir) > 0)
  {
      if (strlen(Job.JobName) > 0)
	  sprintf(Job.LogFile, "%s/%s.log", LoggingDir, Job.JobName);
      else
	  sprintf(Job.LogFile, "%s/GRT.log", LoggingDir);
  }
  else
      sprintf(Job.LogFile, "%s", "");

  if (!ReadXML(Reader, "Migration/Parameters/LoggingLevel", Job.LogLevel, false))
      Job.LogLevel = 1;

  bool success_tmp;

  // Read dimensions of the output Volume
  success = success && ReadXML(Reader, "Migration/OutputVol/nx_along_const_inline", Job.MigVol.nx_xlines);
  success = success && ReadXML(Reader, "Migration/OutputVol/ny_along_const_xline", Job.MigVol.ny_inlines);
  success = success && ReadXML(Reader, "Migration/OutputVol/first_inline_num", Job.MigVol.first_inline_num);
  success = success && ReadXML(Reader, "Migration/OutputVol/first_xline_num", Job.MigVol.first_xline_num);
  success = success && ReadXML(Reader, "Migration/OutputVol/dx_along_const_inline", Job.MigVol.dx_between_xlines);
  success = success && ReadXML(Reader, "Migration/OutputVol/dy_along_const_xline", Job.MigVol.dy_between_inlines);
  success = success && ReadXML(Reader, "Migration/OutputVol/Nz", Job.MigVol.nz);
  success = success && ReadXML(Reader, "Migration/OutputVol/dz", Job.MigVol.dz);
  float X1z (0.0);
  success = success && ReadXML(Reader, "Migration/OutputVol/zMinDepth", X1z);
  Job.MigVol.first_z_coord = -(X1z + (Job.MigVol.nz-1) * Job.MigVol.dz);


  if (!success)
    return -1;

  success &= ReadXML(Reader, "Migration/OutputVol/Off0", Job.Off0Vol);
  success &= ReadXML(Reader, "Migration/OutputVol/NOff", Job.NOffVol);
  success &= ReadXML(Reader, "Migration/OutputVol/dOff", Job.dOffVol);
  if (!success)
      return -1;

  if (!ReadXML(Reader, "Migration/OutputVol/NtotOff", Job.NtotOffVol, false))
      Job.NtotOffVol = Job.NOffVol;
  if (!ReadXML(Reader, "Migration/OutputVol/N0Off", Job.N0OffVol, false))
      Job.N0OffVol = 0;


  int TraceOrder;
  if (!ReadXML(Reader, "Migration/SeismicData/TraceOrder", TraceOrder, false))
    {
      Job.TraceOrder = CDP_GATHER;
    }
  else
    Job.TraceOrder = (FILE_ORDER) TraceOrder;

  if (Job.TraceOrder == IRREGULAR)
    {
      success_tmp = ReadXML(Reader, "Migration/SeismicData/Off0", Job.Offx0);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/NOff", Job.NOffx);
      success = success_tmp && success;
      if (!ReadXML(Reader, "Migration/SeismicData/NtotOff", Job.NtotOffx, false))
        Job.NtotOffx = Job.NOffx;
      if (!ReadXML(Reader, "Migration/SeismicData/N0Off", Job.N0Offx, false))
        Job.N0Offx = 0;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/dOff", Job.Offdx);
      success = success_tmp && success;

      Job.Offy0 = 0;
      Job.NOffy = 1;
      Job.NtotOffy = 1;
      Job.N0Offy = 0;
      Job.Offdy = 100;

      Job.CDPx0 = 0;
      Job.NCDPx = 1;
      Job.NtotCDPx = 1;
      Job.N0CDPx = 0;
      Job.CDPdx = 100;

      Job.CDPy0 = 0;
      Job.NCDPy = 1;
      Job.NtotCDPy = 1;
      Job.N0CDPy = 0;
      Job.CDPdy = 100;

    }
  else
    {
      success_tmp = ReadXML(Reader, "Migration/SeismicData/first_offset", Job.first_offset);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/d_offset", Job.d_offset);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/n_offset", Job.n_offset);

      success_tmp = ReadXML(Reader, "Migration/SeismicData/first_inline_num", Job.first_inline_num_CDP);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/first_xline_num", Job.first_xline_num_CDP);
      success = success_tmp && success;

      success_tmp = ReadXML(Reader, "Migration/SeismicData/nx_along_const_inline", Job.n_xlines_CDP);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/ny_along_const_xline", Job.n_inlines_CDP);
      success = success_tmp && success;

      success_tmp = ReadXML(Reader, "Migration/SeismicData/dx_along_const_inline", Job.d_between_xlines_CDP);
      success = success_tmp && success;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/dy_along_const_xline", Job.d_between_inlines_CDP);
      success = success_tmp && success;

      success_tmp = ReadXML(Reader, "Migration/SeismicData/nx_start_along_const_inline", Job.n_start_use_xlines_CDP, false);
      if (!success_tmp)
	Job.n_start_use_xlines_CDP = 0;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/nx_use_along_const_inline", Job.n_use_xlines_CDP, false);
      if (!success_tmp)
	Job.n_use_xlines_CDP = Job.n_xlines_CDP - Job.n_start_use_xlines_CDP;

      success_tmp = ReadXML(Reader, "Migration/SeismicData/ny_start_along_const_xline", Job.n_start_use_inlines_CDP, false);
      if (!success_tmp)
	Job.n_start_use_inlines_CDP = 0;
      success_tmp = ReadXML(Reader, "Migration/SeismicData/ny_use_along_const_xline", Job.n_use_inlines_CDP, false);
      if (!success_tmp)
	Job.n_use_inlines_CDP = Job.n_inlines_CDP - Job.n_start_use_inlines_CDP;
    }


  success_tmp = ReadXML(Reader, "Migration/Parameters/ApertureAngle", Job.CDPAperture_deg.v);
  success = success_tmp && success;

  success_tmp = ReadXML(Reader, "Migration/SeismicData/TraceFileName", Job.TraceFileName);
  success = success_tmp && success;
  success_tmp = ReadXML(Reader, "Migration/OutputVol/MigDirectoryName", Job.MigDirName);
  success = success_tmp && success;
  success = success && ReadXML(Reader, "Migration/Parameters/TTMasterFile", Job.TTCfgFileName);
  success = success_tmp && success;

  int TFMode (1);
  success_tmp = ReadXML(Reader, "Migration/SeismicData/TraceFileMode", TFMode);

  if (!success_tmp)
    TFMode = 1;
  switch (TFMode)
    {
    case static_cast<int>(SEGY_BIGENDIAN):
      Job.TraceFileMode = SEGY_BIGENDIAN;
    break;
    case static_cast<int>(SU_LITENDIAN):
      Job.TraceFileMode = SU_LITENDIAN;
    break;
    default:
      Job.TraceFileMode = UNDEFINED_FILE_MODE;

      success = false;
    }

  int MFMode (0);
  success_tmp = ReadXML(Reader, "Migration/OutputVol/MigFileMode", MFMode, false);

  if (!success_tmp)
    MFMode = 0;

  std::cout << "MFMode " << MFMode << " " << success_tmp << std::endl;


  switch (MFMode)
    {
    case static_cast<int>(SEGY_BIGENDIAN):
      Job.MigFileMode = SEGY_BIGENDIAN;
      break;
    case static_cast<int>(SU_LITENDIAN):
      Job.MigFileMode = SU_LITENDIAN;
      break;
    case static_cast<int>(SVF):
      Job.MigFileMode = SVF;
      break;
    default:
      Job.MigFileMode = UNDEFINED_FILE_MODE;

      success = false;
    }

  int GMode (1);

  if (!ReadXML(Reader, "Migration/OutputVol/GatherMode", GMode, false))
    GMode = 1;
  Job.GatherMode = (GATHER_MODE) GMode;

  float X0Velx, X0Vely, X0Velz;
  ReadXML(Reader, "RayTracing/VelocityModel/X0Velx", X0Velx, false);
  ReadXML(Reader, "RayTracing/VelocityModel/X0Vely", X0Vely, false);
  ReadXML(Reader, "RayTracing/VelocityModel/X0Velz", X0Velz, false);
  Job.X0Vel = point3D<float>(X0Velx, X0Vely, X0Velz);

  int NVelx, NVely, NVelz;
  ReadXML(Reader, "RayTracing/VelocityModel/NxVel", NVelx, false);
  ReadXML(Reader, "RayTracing/VelocityModel/NyVel", NVely, false);
  ReadXML(Reader, "RayTracing/VelocityModel/NzVel", NVelz, false);
//   success_tmp = ReadXML(Reader, NVelx, "NxVel");
//   success_tmp = ReadXML(Reader, NVely, "NyVel");
//   success_tmp = ReadXML(Reader, NVelz, "NzVel");
  Job.NVel = point3D<int>(NVelx, NVely, NVelz);

  float dxVel, dyVel, dzVel;
  ReadXML(Reader, "RayTracing/VelocityModel/dxVel", dxVel, false);
  ReadXML(Reader, "RayTracing/VelocityModel/dyVel", dyVel, false);
  ReadXML(Reader, "RayTracing/VelocityModel/dzVel", dzVel, false);
//   success_tmp = ReadXML(Reader, dxVel, "dxVel");
//   success_tmp = ReadXML(Reader, dyVel, "dyVel");
//   success_tmp = ReadXML(Reader, dzVel, "dzVel");
  Job.dxVel = point3D<float>(dxVel, dyVel, dzVel);


  float dxBnd, dyBnd, dzBnd;
  ReadXML(Reader, "RayTracing/VelocityModel/dxBnd", dxBnd, false);
  ReadXML(Reader, "RayTracing/VelocityModel/dyBnd", dyBnd, false);
  ReadXML(Reader, "RayTracing/VelocityModel/dzBnd", dzBnd, false);
//   success_tmp = ReadXML(Reader, dxBnd, "dxBnd");
//   success_tmp = ReadXML(Reader, dyBnd, "dyBnd");
//   success_tmp = ReadXML(Reader, dzBnd, "dzBnd");
  Job.dxBnd = point3D<float>(dxBnd, dyBnd, dzBnd);

  ReadXML(Reader, "RayTracing/Parameters/MAXTSTEP", Job.g_MAXTSTEP, false);
  ReadXML(Reader, "RayTracing/Parameters/TSTEPSIZE", Job.g_TSTEPSIZE, false);
  ReadXML(Reader, "RayTracing/Parameters/InitialAngleResolution", Job.g_InitAngle.v, false);
  ReadXML(Reader, "RayTracing/Parameters/REF_LEN", Job.g_REF_LEN, false);
  ReadXML(Reader, "RayTracing/Parameters/TRACINGSTEPS", Job.g_TRACINGSTEPS, false);
//   success_tmp = ReadXML(Reader, Job.g_MAXTSTEP, "MAXTSTEP");
//   success_tmp = ReadXML(Reader, Job.g_TSTEPSIZE, "TSTEPSIZE");
//   success_tmp = ReadXML(Reader, Job.g_RAY_MIN, "RAY_MIN");
//   success_tmp = ReadXML(Reader, Job.g_REF_LEN, "REF_LEN");
//   success_tmp = ReadXML(Reader, Job.g_TRACINGSTEPS, "TRACINGSTEPS");


  success_tmp = (ReadTTTConfigFileXML(Job.TTCfgFileName, Job) == 0);
  success = success_tmp && success;

//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGridX0", Job.SrfcGridX0);
//   //success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGridY0", Job.SrfcGridY0);
//   //success = success_tmp && success;

//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGriddx", Job.SrfcGriddx);
//   //success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGriddy", Job.SrfcGriddy);
//   //success = success_tmp && success;

//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGridNx", Job.SrfcGridNx);
//   //success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "Migration/TTTables/SrfcGridNy", Job.SrfcGridNy);
//   //success = success_tmp && success;

//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridX0", Job.VolGridX0);
//   //success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridY0", Job.VolGridY0);
//   //success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridZ0", Job.VolGridZ0);
//   //success = success_tmp && success;

//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGriddx", Job.VolGriddx);
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGriddy", Job.VolGriddy);
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGriddz", Job.VolGriddz);


//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridNx", Job.VolGridNx);
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridNy", Job.VolGridNy);
//   success_tmp = ReadXML(Reader, "Migration/TTTables/VolGridNz", Job.VolGridNz);
//   //success = success_tmp && success;

  char frac[10];
  Job.frac = true;
  if (ReadXML(Reader, "Migration/Parameters/frac", frac, false))
    Job.frac = (strcmp(frac,"no") != 0);

  char anti_aliasing[10];
  Job.anti_aliasing = true;
  if (ReadXML(Reader, "Migration/Parameters/anti_aliasing", anti_aliasing, false))
    Job.anti_aliasing = (strcmp( anti_aliasing,"no") != 0);

  char* FrequencyString = new char[10000];
  FrequencyString[0] = 0;

  if (!ReadXML(Reader, "Migration/SeismicData/clip", Job.clip, false))
    Job.clip = -1.f;
  if (!ReadXML(Reader, "Migration/SeismicData/trap", Job.trap, false))
    Job.trap = -1.f;
  if (!ReadXML(Reader, "Migration/SeismicData/tpow", Job.tpow, false))
    Job.tpow = 0.f;

  if (ReadXML(Reader, "Migration/SeismicData/BandPass", FrequencyString, false))
  {
    printf("frequency string: <%s>\n", FrequencyString);

      if ((strlen(FrequencyString) > 0))
      {
	const char delimiter[] = ",";

	  errno = 0;
	  Job.frequ1 = strtod(strsep(&FrequencyString, delimiter), NULL);
	  Job.frequ2 = strtod(strsep(&FrequencyString, delimiter), NULL);
	  Job.frequ3 = strtod(strsep(&FrequencyString, delimiter), NULL);
	  Job.frequ4 = strtod(strsep(&FrequencyString, delimiter), NULL);

	  //std::cout << Job.frequ1 << " " << Job.frequ2 << " " << Job.frequ3 << " " << Job.frequ4 << std::endl;
	  if ( (errno != 0)
	       || ((Job.frequ2 - Job.frequ1) < 1)
	       || ((Job.frequ3 - Job.frequ2) < 1)
	       || ((Job.frequ4 - Job.frequ3) < 1))
	  {
	      Job.frequ1 = -1;
	      Job.frequ2 = -1;
	      Job.frequ3 = -1;
	      Job.frequ4 = -1;
	      //
	  }

	  //std::cout << Job.frequ1 << " " << Job.frequ2 << " " << Job.frequ3 << " " << Job.frequ4 << std::endl;
      }
  }
//  else
//      std::cout << "BandPass not read\n";
  delete[] FrequencyString;

  // SDPA control parameters
//   success_tmp = ReadXML(Reader, "common/Control/NBunchesPerPackage", Job.bunchesperpack);
//   success = success_tmp && success;
  success_tmp = ReadXML(Reader, "common/Control/NTracesPerBunch", Job.tracesperbunch);
  success = success_tmp && success;
//   success_tmp = ReadXML(Reader, "common/Control/NPackagesInNet", Job.NpackInNet);
//   success = success_tmp && success;
  success_tmp = ReadXML(Reader, "common/Control/NSubVols", Job.NSubVols);
  success = success_tmp && success;

  int ierr;
  TraceFileHandler TFHandler(Job.TraceFileName,Job.TraceFileMode,ierr);
  int Nt_tmp=0;
  Nt_tmp=TFHandler.getNt();
  if(Nt_tmp!=0) Job.traceNt=Nt_tmp;
  float dt_tmp=0.;
  dt_tmp=TFHandler.getdt();
  if(dt_tmp!=0.) Job.tracedt=dt_tmp;

  // Check the migration job
  success_tmp = Check(Job);
  success = success_tmp && success;

  Scale(Job);

//   std::cout << Job.SrfcGridX0 << " " << Job.SrfcGridY0 << " " << Job.SrfcGriddx << " " << Job.SrfcGriddy << " " << Job.SrfcGridNx << " " << Job.SrfcGridNy << " " << std::endl;
//   std::cout << Job.VolGridX0 << " " << Job.VolGridY0 << " " << Job.VolGridZ0 << " " << Job.VolGriddx << " " << Job.VolGriddy << " " << Job.VolGriddz << " " << Job.VolGridNx << " " << Job.VolGridNy << " " << Job.VolGridNz << " " << std::endl;
//   std::cout << Job.X0[0] << " " << Job.X0[1] << " " << Job.X0[2] << " " << Job.NX[0] << " " << Job.NX[1] << " " << Job.NX[2] << " " << Job.dx[0] << " " << Job.dx[1] << " " << Job.dx[2] << " " << std::endl;

  if (!success)
    return -1;
  return 0;
}

int CheckReadMigrationJob::WriteConfigFileXML(char* ConfigFileName, MigrationJob& Job)
{
  if ( ConfigFileName == NULL )
    {
      //
      return -1;
    }

  chmod(ConfigFileName,S_IWUSR | S_IWGRP | S_IWOTH );
  std::ofstream ConfigFile(ConfigFileName);
  if ( ConfigFile.fail())
    {
      //
      return -1;
    }

  ConfigFile << "<?xml version = '1.0'?>\n";
  ConfigFile << "<Kirchhoff3D>\n";
  ConfigFile << "  <common>\n";
  ConfigFile << "     <Parameters>\n";
  ConfigFile << "        <ProjectName>" << Job.ProjectName << "</ProjectName>\n";
  char LogFile[199];
  strcpy(LogFile, Job.LogFile);
  ConfigFile << "        <LoggingDirectory>" << dirname(LogFile) << "</LoggingDirectory>\n";
  if (Job.frac)
      ConfigFile << "        <frac>yes</frac>\n";
  else
      ConfigFile << "        <frac>no</frac>\n";
  if (Job.anti_aliasing)
      ConfigFile << "        <anti_aliasing>yes</anti_aliasing>\n";
  else
      ConfigFile << "        <anti_aliasing>no</anti_aliasing>\n";
  ConfigFile << "     </Parameters>\n";
  ConfigFile << "     <Geometry>\n";
  ConfigFile << "        <one_inline_num>" << Job.geom.one_inline_num << "</one_inline_num>\n";
  ConfigFile << "        <one_xline_num>" << Job.geom.one_xline_num << "</one_xline_num>\n";
  ConfigFile << "        <one_x>" << static_cast<int>(Job.geom.x_one) << "</one_x>\n";
  ConfigFile << "        <one_y>" << static_cast<int>(Job.geom.y_one) << "</one_y>\n";
  ConfigFile << "        <dx_between_xlines unit=\"[m]\">" << Job.geom.d_between_xlines << "</dx_between_xlines>\n";
  ConfigFile << "        <dy_between_inlines unit=\"[m]\">" << Job.geom.d_between_inlines << "</dy_between_inlines>\n";
  ConfigFile << "        <angle_rot_deg unit=\"[deg]\">" << Job.geom.angle_rot_deg.v << "</angle_rot_deg>\n";
  ConfigFile << "        <inlinedir values=\"-1,+1\">" << Job.geom.inlinedir << "</inlinedir>\n";
  ConfigFile << "     </Geometry>\n";
  ConfigFile << "  </common>\n";

  ConfigFile << std::endl;

  ConfigFile << "  <Migration>\n";
  ConfigFile << "    <OutputVol>\n";
  ConfigFile << "      <MigDirectoryName>" << Job.MigDirName << "</MigDirectoryName>" << std::endl;
  ConfigFile << "      <MigFileMode>" << (int) Job.MigFileMode << "</MigFileMode>" << std::endl;
  ConfigFile << "      <zMinDepth unit=\"[m]\">" << -(Job.MigVol.first_z_coord + (Job.MigVol.nz-1)*Job.MigVol.dz) << "</zMinDepth>\n";
  ConfigFile << "      <Nz>" << Job.MigVol.nz << "</Nz>\n";
  ConfigFile << "      <dz unit=\"[m]\">" << Job.MigVol.dz << "</dz>\n";
  ConfigFile << "      <nx_along_const_inline>" << Job.MigVol.nx_xlines << "</nx_along_const_inline>\n";
  ConfigFile << "      <ny_along_const_xline>" << Job.MigVol.ny_inlines << "</ny_along_const_xline>\n";
  ConfigFile << "      <first_inline_num>" << Job.MigVol.first_inline_num << "</first_inline_num>\n";
  ConfigFile << "      <first_xline_num>" << Job.MigVol.first_xline_num << "</first_xline_num>\n";
  ConfigFile << "      <dx_along_const_inline unit=\"[m]\">" << Job.MigVol.dx_between_xlines << "</dx_along_const_inline>\n";
  ConfigFile << "      <dy_along_const_xline unit=\"[m]\">" << Job.MigVol.dy_between_inlines << "</dy_along_const_xline>\n";
  ConfigFile << "      <Off0>" << Job.Off0Vol << "</Off0>\n";
  ConfigFile << "      <NOff>" << Job.NOffVol << "</NOff>\n";
  ConfigFile << "      <dOff>" << Job.dOffVol << "</dOff>\n";
  ConfigFile << "      <NtotOff>" << Job.NtotOffVol << "</NtotOff>\n";
  ConfigFile << "      <N0Off>" << Job.N0OffVol << "</N0Off>\n";

  ConfigFile << "    </OutputVol>\n";

  ConfigFile << std::endl;

  ConfigFile << "    <SeismicData>\n";
  ConfigFile << "      <TraceFileName>" << Job.TraceFileName << "</TraceFileName>" << std::endl;
  ConfigFile << "      <TraceFileMode>" << (int) Job.TraceFileMode << "</TraceFileMode>" << std::endl;

  ConfigFile << std::endl;

  ConfigFile << "      <first_offset>" << Job.first_offset << "</first_offset>" << std::endl;
  ConfigFile << "      <n_offset>" << Job.n_offset << "</n_offset>" << std::endl;
  ConfigFile << "      <d_offset>" << Job.d_offset << "</d_offset>" << std::endl;

  ConfigFile << std::endl;

  ConfigFile << "      <nx_along_const_inline>" << Job.n_xlines_CDP << "</nx_along_const_inline>\n";
  ConfigFile << "      <ny_along_const_xline>" << Job.n_inlines_CDP << "</ny_along_const_xline>\n";
  ConfigFile << "      <first_inline_num>" << Job.first_inline_num_CDP << "</first_inline_num>\n";
  ConfigFile << "      <first_xline_num>" << Job.first_xline_num_CDP << "</first_xline_num>\n";
  ConfigFile << "      <dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_CDP << "</dx_along_const_inline>\n";
  ConfigFile << "      <dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_CDP << "</dy_along_const_xline>\n";
  ConfigFile << "      <nx_start_along_const_inline>" << Job.n_start_use_xlines_CDP << "</nx_start_along_const_inline>\n";
  ConfigFile << "      <ny_start_along_const_xline>" << Job.n_start_use_inlines_CDP << "</ny_start_along_const_xline>\n";
  ConfigFile << "      <nx_use_along_const_inline>" << Job.n_use_xlines_CDP << "</nx_use_along_const_inline>\n";
  ConfigFile << "      <ny_use_along_const_xline>" << Job.n_use_inlines_CDP << "</ny_use_along_const_xline>\n";

  ConfigFile << std::endl;

  ConfigFile << "      <Offx0>" << Job.Offx0 << "</Offx0>" << std::endl;
  ConfigFile << "      <NtotOffx>" << Job.NtotOffx << "</NtotOffx>" << std::endl;
  ConfigFile << "      <N0Offx>" << Job.N0Offx << "</N0Offx>" << std::endl;
  ConfigFile << "      <NOffx>" << Job.NOffx << "</NOffx>" << std::endl;
  ConfigFile << "      <Offdx>" << Job.Offdx << "</Offdx>" << std::endl;

  ConfigFile << std::endl;

  ConfigFile << "      <Offy0>" << Job.Offy0 << "</Offy0>" << std::endl;
  ConfigFile << "      <NtotOffy>" << Job.NtotOffy << "</NtotOffy>" << std::endl;
  ConfigFile << "      <N0Offy>" << Job.N0Offy << "</N0Offy>" << std::endl;
  ConfigFile << "      <NOffy>" << Job.NOffy << "</NOffy>" << std::endl;
  ConfigFile << "      <Offdy>" << Job.Offdy << "</Offdy>" << std::endl;

  ConfigFile << std::endl;

  ConfigFile << "      <CDPx0>" << Job.CDPx0 << "</CDPx0>" << std::endl;
  ConfigFile << "      <NtotCDPx>" << Job.NtotCDPx << "</NtotCDPx>" << std::endl;
  ConfigFile << "      <N0CDPx>" << Job.N0CDPx << "</N0CDPx>" << std::endl;
  ConfigFile << "      <NCDPx>" << Job.NCDPx << "</NCDPx>" << std::endl;
  ConfigFile << "      <CDPdx>" << Job.CDPdx << "</CDPdx>" << std::endl;

  ConfigFile << std::endl;

  ConfigFile << "      <CDPy0>" << Job.CDPy0 << "</CDPy0>" << std::endl;
  ConfigFile << "      <NtotCDPy>" << Job.NtotCDPy << "</NtotCDPy>" << std::endl;
  ConfigFile << "      <N0CDPy>" << Job.N0CDPy << "</N0CDPy>" << std::endl;
  ConfigFile << "      <NCDPy>" << Job.NCDPy << "</NCDPy>" << std::endl;
  ConfigFile << "      <CDPdy>" << Job.CDPdy << "</CDPdy>" << std::endl;
  ConfigFile << "      <clip>" << Job.clip << "</clip>\n";
  ConfigFile << "      <trap>" << Job.trap << "</trap>\n";
  ConfigFile << "      <tpow>" << Job.tpow << "</tpow>\n";
  ConfigFile << "      <BandPass>" << Job.frequ1 << "," << Job.frequ2 << "," << Job.frequ3 << "," << Job.frequ4 << "</BandPass>\n";
  ConfigFile << "    </SeismicData>\n";

  ConfigFile << std::endl;

  ConfigFile << "     <TTTables>\n";
  ConfigFile << "        <TTFileName>" << Job.RTFileName << "</TTFileName>" << std::endl;
  ConfigFile << "\n";
  ConfigFile << "        <SrfcGridX0>" << Job.SrfcGridX0.v << "</SrfcGridX0>\n";
  ConfigFile << "        <SrfcGridY0>" << Job.SrfcGridY0.v << "</SrfcGridY0>\n";
  ConfigFile << "        <SrfcGridNx>" << Job.SrfcGridNx << "</SrfcGridNx>\n";
  ConfigFile << "        <SrfcGridNy>" << Job.SrfcGridNy << "</SrfcGridNy>\n";
  ConfigFile << "        <SrfcGriddx>" << Job.SrfcGriddx << "</SrfcGriddx>\n";
  ConfigFile << "        <SrfcGriddy>" << Job.SrfcGriddy << "</SrfcGriddy>\n";
  ConfigFile << "\n";
  ConfigFile << "        <VolGridX0>" << Job.TTVol.first_x_coord.v << "</VolGridX0>\n";
  ConfigFile << "        <VolGridY0>" << Job.TTVol.first_y_coord.v << "</VolGridY0>\n";
  ConfigFile << "        <VolGridZ0>" << Job.TTVol.first_z_coord << "</VolGridZ0>\n";
  ConfigFile << "        <VolGridNx>" << Job.TTVol.nx_xlines << "</VolGridNx>\n";
  ConfigFile << "        <VolGridNy>" << Job.TTVol.ny_inlines << "</VolGridNy>\n";
  ConfigFile << "        <VolGridNz>" << Job.TTVol.nz << "</VolGridNz>\n";
  ConfigFile << "        <VolGriddx>" << Job.TTVol.dx_between_xlines << "</VolGriddx>\n";
  ConfigFile << "        <VolGriddy>" << Job.TTVol.dy_between_inlines << "</VolGriddy>\n";
  ConfigFile << "        <VolGriddz>" << Job.TTVol.dz << "</VolGriddz>\n";
  ConfigFile << "\n";
  ConfigFile << "    </TTTables>\n";

  ConfigFile << "    <Parameters>\n";
  ConfigFile << "      <jobname_M>" << Job.JobName << "</jobname_M>" << std::endl;
  ConfigFile << "      <LoggingLevel>" << Job.LogLevel << "</LoggingLevel>" << std::endl;
  ConfigFile << "      <TTMasterFile>" << Job.TTCfgFileName << "</TTMasterFile>" << std::endl;
  ConfigFile << "      <ApertureAngle>" << Job.CDPAperture_deg.v << "</ApertureAngle>" << std::endl;
  ConfigFile << "    </Parameters>\n";
  ConfigFile << "  </Migration>\n";

  ConfigFile << "</Kirchhoff3D>\n";
  ConfigFile.close();

  chmod(ConfigFileName,S_IRUSR | S_IRGRP | S_IROTH );
  return 0;
}

int CheckReadMigrationJob::ReadTTTConfigFileXML(char* ConfigFileName, MigrationJob& Job)
{
   if ( ConfigFileName == NULL )
     {
       //
       return(-1);
     }

   //   ConfigFileName;
   //
   //

   if ( CheckExistence(ConfigFileName) == -1)
     {
       //
       return -1;
     }
   XMLReader Reader;
   Reader.setFile(ConfigFileName);

   bool success = true;
   bool success_tmp;

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/first_inline_num", Job.first_inline_num_TT_Srfc);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/first_xline_num", Job.first_xline_num_TT_Srfc);

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/nx_along_const_inline", Job.n_xlines_TT_Srfc);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/ny_along_const_xline", Job.n_inlines_TT_Srfc);

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/dx_along_const_inline", Job.d_between_xlines_TT_Srfc);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/dy_along_const_xline", Job.d_between_inlines_TT_Srfc);


   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_first_inline_num", Job.TTVol.first_inline_num);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_first_xline_num", Job.TTVol.first_xline_num);

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_nx_along_const_inline", Job.TTVol.nx_xlines);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_ny_along_const_xline", Job.TTVol.ny_inlines);

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dx_along_const_inline", Job.TTVol.dx_between_xlines);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dy_along_const_xline", Job.TTVol.dy_between_inlines);

   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_Nz", Job.TTVol.nz);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dz", Job.TTVol.dz);
   float minDepth_TT_SubSrfc;
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_zMinDepth", minDepth_TT_SubSrfc);
   Job.TTVol.first_z_coord = -(minDepth_TT_SubSrfc + (Job.TTVol.nz-1) * Job.TTVol.dz);



   //success_tmp = ReadXML(Reader, "RayTracing/TTTables/TTFileName", Job.RTFileName);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/TTDirectoryName", Job.TTDirName);
   success_tmp = ReadXML(Reader, "RayTracing/TTTables/TTFilePrefix", Job.TTFilePrefix);
   sprintf(Job.RTFileName, "%s/%s_LZ", Job.TTDirName, Job.TTFilePrefix);

   success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGridX0", Job.SrfcGridX0);
//    success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGridY0", Job.SrfcGridY0);
//    success = success_tmp && success;

//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGriddx", Job.SrfcGriddx);
//    success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGriddy", Job.SrfcGriddy);
//    success = success_tmp && success;

//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGridNx", Job.SrfcGridNx);
//    success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/SrfcGridNy", Job.SrfcGridNy);
//    success = success_tmp && success;

//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridX0", Job.VolGridX0);
//    success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridY0", Job.VolGridY0);
//    success = success_tmp && success;
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridZ0", Job.VolGridZ0);
//    success = success_tmp && success;

//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGriddx", Job.VolGriddx);
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGriddy", Job.VolGriddy);
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGriddz", Job.VolGriddz);


//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridNx", Job.VolGridNx);
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridNy", Job.VolGridNy);
//    success_tmp = ReadXML(Reader, "RayTracing/TTTables/VolGridNz", Job.VolGridNz);
//    success = success_tmp && success;

  if (!success)
    return -1;
  else
    return 0;
}

int CheckReadMigrationJob::CheckExistence(const char *FileName)
{
  FILE * pFile = fopen(FileName, "r");
  if ( pFile == NULL )
    return -1;
  else
    {
      fclose(pFile);
      return 0;
    }
}

int CheckReadMigrationJob::CreateFile(const char *FileName)
{
  FILE * pFile = fopen(FileName, "w+");
  if ( pFile == NULL )
    return -1;
  else
    {
      fclose(pFile);
      return 0;
    }
}

bool CheckReadMigrationJob::Check(MigrationJob& Job)
{
#ifdef PARALLEL_PHASTGRID
    Job.SrfcGridX0 = 1000;
    Job.SrfcGridY0 = 0;

    Job.SrfcGriddx = 400;
    Job.SrfcGriddy = 100;

    Job.SrfcGridNx = 10;
    Job.SrfcGridNy = 40;

  // TT table Volume grid
    Job.TTVol.X0[0] = 1000;
    Job.TTVol.X0[1] = 1000;
    Job.TTVol.X0[2] = -5000;

    Job.TTVol.dx[0] = 80;
    Job.TTVol.dx[1] = 40;
    Job.TTVol.dx[2] = 25;

    if ((Job.TTVol.NX[0] != 2) && (Job.TTVol.NX[0] != 8))
	Job.TTVol.NX[0] = 2;
    Job.TTVol.NX[1] = 50;
    Job.TTVol.NX[2] = 200;


    Job.Offx0 = 0;
    Job.NOffx = 1;
    Job.N0Offx = 0;
    Job.NtotOffx = 1;
    Job.Offdx = 20;

    Job.Offy0 = 0;
    if ( (Job.NOffy < 1) )
	Job.NOffy = 1;
    if ( (Job.NOffy > 20) )
	Job.NOffy = 20;
    Job.N0Offy = 0;
    Job.NtotOffy = Job.NOffy;
    Job.Offdy = 20;

    Job.CDPx0 = 1000;
    Job.NCDPx = 90;
    Job.N0CDPx = 0;
    Job.NtotCDPx = 90;
    Job.CDPdx = 40;

    Job.CDPy0 = 1000;
    Job.NCDPy = 101;
    Job.N0CDPy = 0;
    Job.NtotCDPy = 101;
    Job.CDPdy = 20;


    if ( fabs(Job.TTVol.dx[0] - ((int)(Job.TTVol.dx[0] / Job.dx[0])) * Job.dx[0]) > 1e-4)
    {
	int factor = Job.TTVol.dx[0] / Job.dx[0] + 1;
	Job.dx[0] =  Job.TTVol.dx[0] / factor;
    }
    if ( fabs(Job.TTVol.dx[1] - ((int)(Job.TTVol.dx[1] / Job.dx[1])) * Job.dx[1]) > 1e-4)
    {
	int factor = Job.TTVol.dx[1] / Job.dx[1] + 1;
	Job.dx[1] =  Job.TTVol.dx[1] / factor;
    }
    if ( fabs(Job.TTVol.dx[2] - ((int)(Job.TTVol.dx[2] / Job.dx[2])) * Job.dx[2]) > 1e-4)
    {
	int factor = Job.TTVol.dx[2] / Job.dx[2] + 1;
	Job.dx[2] =  Job.TTVol.dx[2] / factor;
    }

    Job.NX[0] = 4;
    {
	int mult = (int) (Job.TTVol.dx[1] / Job.dx[1]);
	if ( (Job.NX[1] % mult) != 0)
	{
	    int factor = Job.NX[1] / mult;
	    Job.NX[1] = factor * mult;
	}
    }
    //Job.NX[1] = 392;

    if ( (Job.NX[2] % 4) != 0)
    {
	int factor = Job.NX[2] / 4;
	Job.NX[2] = factor * 4;
    }

  if ( (Job.CDPAperture_deg > 90) || (Job.CDPAperture_deg < 0) )
    {
      //
	return false;
    }

#else
// Check if Trace data file can be accessed
    if (CheckExistence(Job.TraceFileName) != 0)
    {
      //
	return false;
    }

// Analyse travel time tables
    {
// 	TTFileHandler TTFile;
// 	grid3D GVol; grid2D GSrc;
// 	if (!TTFile.Analyse(Job.RTFileName, GVol, GSrc))
// 	{
//
// 	    return false;
// 	}
// 	Job.SrfcGridX0 = GSrc.getx0(); Job.SrfcGridY0 = GSrc.gety0();
// 	Job.SrfcGridNx = GSrc.getNx(); Job.SrfcGridNy = GSrc.getNy();
// 	Job.SrfcGriddx = GSrc.getdx(); Job.SrfcGriddy = GSrc.getdy();

// 	Job.VolGridX0 = GVol.getx0(); Job.VolGridY0 = GVol.gety0(); Job.VolGridZ0 = GVol.getz0();
// 	Job.VolGridNx = GVol.getNx(); Job.VolGridNy = GVol.getNy(); Job.VolGridNz = GVol.getNz();
// 	Job.VolGriddx = GVol.getdx(); Job.VolGriddy = GVol.getdy(); Job.VolGriddz = GVol.getdz();
    }

    if ( Job.MigVol.first_x_coord.v < Job.TTVol.first_x_coord.v )
    {
	int n0 = (int) ((Job.TTVol.first_x_coord.v - Job.MigVol.first_x_coord.v)/Job.MigVol.dx_between_xlines + 0.5f);
	Job.MigVol.first_x_coord.v += n0 * Job.MigVol.dx_between_xlines;
	Job.MigVol.nx_xlines -= n0;
    }

//     if ( Job.X0[0] + NX[0] * dx[0] >= Job.TTVol.X0[0] + Job.TTVol.NX[0] * Job.TTVol.dx[0] )
//     {
// 	int n0 = (int) ((Job.TTVol.X0[0] - Job.X0[0])/Job.dx[0] + 0.5f);
// 	Job.X0[0] += n0 * Job.dx[0];
// 	Job.NX[0] -= n0;
//     }

  if ( (Job.MigVol.nz % 4) != 0)
    {
      std::cerr << "CheckReadMigrationJob: NzVol must be multiple of 4 but is " << Job.MigVol.nz << std::endl;
      return false;
    }

  if ( (Job.CDPAperture_deg.v > 90) || (Job.CDPAperture_deg.v < 0) )
    {
      //
	return false;
    }

#endif

  return true;
}

bool CheckReadMigrationJob::ReadXML(XMLReader& Reader, const char* VarName, int& Var, bool message)
{
    if (!Reader.getInt(VarName, Var))
    {
//       if (message)
//         ;
      return false;
    }
  return true;
}

bool CheckReadMigrationJob::ReadXML(XMLReader& Reader, const char* VarName, float& Var, bool message)
{
  if (!Reader.getFloat(VarName, Var))
    {
//       if (message)
//         ;
      return false;
    }
  return true;
}

bool CheckReadMigrationJob::ReadXML(XMLReader& Reader, const char* VarName, char* Var, bool message)
{
  if (!Reader.getChar(VarName, Var))
    {
//       if (message)
//         ;
      return false;
    }
  return true;
}

void CheckReadMigrationJob::Scale(MigrationJob& Job)
{
    Job.geom.first_inline_num = Job.MigVol.first_inline_num;
    Job.geom.first_xline_num = Job.MigVol.first_xline_num;
    Job.geom.n_inlines = Job.MigVol.ny_inlines;
    Job.geom.n_xlines = Job.MigVol.nx_xlines;

    Acq_geometry<float> Geom(Job.geom);

    Job.MigVol.first_x_coord = 0.0f;
    Job.MigVol.first_y_coord = 0.0f;


    Job.CDPx0 = (Job.first_xline_num_CDP - Job.geom.first_xline_num) * Job.geom.d_between_xlines;
    Job.CDPy0 = (Job.first_inline_num_CDP - Job.geom.first_inline_num) * Job.geom.d_between_inlines;

    Job.NtotCDPx = Job.n_xlines_CDP;
    Job.NtotCDPy = Job.n_inlines_CDP;

    Job.CDPdx = Job.d_between_xlines_CDP;
    Job.CDPdy = Job.d_between_inlines_CDP;


    Job.N0CDPx = Job.n_start_use_xlines_CDP;
    Job.N0CDPy = Job.n_start_use_inlines_CDP;

    Job.NCDPx = Job.n_use_xlines_CDP;
    Job.NCDPy = Job.n_use_inlines_CDP;

    Job.Offx0 = Job.first_offset;
    Job.NtotOffx = Job.n_offset;
    Job.Offdx = Job.d_offset;
    Job.N0Offx = 0;
    Job.NOffx = Job.NtotOffx;

    Job.Offy0 = 0;
    Job.NtotOffy = 1;
    Job.Offdy = 1.0;
    Job.N0Offy = 0;
    Job.NOffy = Job.NtotOffy;

    Job.SrfcGridX0 = (Job.first_xline_num_TT_Srfc - Job.geom.first_xline_num) * Job.geom.d_between_xlines;
    Job.SrfcGridY0 = (Job.first_inline_num_TT_Srfc - Job.geom.first_inline_num) * Job.geom.d_between_inlines;

    Job.SrfcGriddx = Job.d_between_xlines_TT_Srfc;
    Job.SrfcGriddy = Job.d_between_inlines_TT_Srfc;
    Job.SrfcGridNx = Job.n_xlines_TT_Srfc;
    Job.SrfcGridNy = Job.n_inlines_TT_Srfc;

    Job.TTVol.first_x_coord = (Job.TTVol.first_xline_num - Job.geom.first_xline_num) * Job.geom.d_between_xlines;
    Job.TTVol.first_y_coord = (Job.TTVol.first_inline_num - Job.geom.first_inline_num) * Job.geom.d_between_inlines;

    char MigFileExt[10];
    if (Job.MigFileMode == SU_LITENDIAN)
	sprintf(MigFileExt, "su");
    else
	sprintf(MigFileExt, "sgy");

    if (strlen(Job.JobName) > 0)
	sprintf(Job.MigFileName, "%s/%s_migout.%s", Job.MigDirName, Job.JobName, MigFileExt);
    else
	sprintf(Job.MigFileName, "%s/migout.%s", Job.MigDirName, MigFileExt);
}
