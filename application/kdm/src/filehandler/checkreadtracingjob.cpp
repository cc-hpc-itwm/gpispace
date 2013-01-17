/***************************************************************************
                          checkreadtracingjob.cpp  -  description

    Read the variables for the tracing job from the given
    configuration file and check for consitency of the data.
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

/**
  *@author Dirk Merten
  */

#include "checkreadtracingjob.h"

int CheckReadTracingJob::ReadConfigFileXML(char* ConfigFileName, TracingJob& Job)
{
  if ( ConfigFileName == NULL )
  {
      
      return -1;
  }

  CfgFileName = ConfigFileName;
  
  

 if ( CheckExistence() == -1)
  {
      
      return -1;
  }
  XMLReader Reader;
  Reader.setFile(ConfigFileName);
  
  bool success = true;

  int TMode;
  if (! ReadXML(Reader, "RayTracing/Parameters/TracingMode", TMode, false))
    TMode = 1;
  Job.RunMode = (TRACING_MODE) TMode;

  ReadXML(Reader, "common/Parameters/ProjectName", Job.ProjectName, false);


  char LoggingDir[199];
  if (!ReadXML(Reader, "common/Parameters/LoggingDirectory", LoggingDir, false))
      sprintf( LoggingDir, "");

  if (!ReadXML(Reader, "RayTracing/Parameters/jobname_R", Job.JobName, false))
      sprintf(Job.JobName, "");

  if (strlen(LoggingDir) > 0)
  {
      if (strlen(Job.JobName) > 0)
	  sprintf(Job.LogFile, "%s/%s.log",  LoggingDir, Job.JobName);
      else
	  sprintf(Job.LogFile, "%s/SeisRay.log",  LoggingDir);
  }
  else
      sprintf(Job.LogFile, "");

  if (!ReadXML(Reader, "RayTracing/Parameters/LoggingLevel", Job.LogLevel, false))
      Job.LogLevel = 1;

  char restart[10];
  Job.g_restart = false;
  if (ReadXML(Reader, "RayTracing/Parameters/Restart", restart, false) )
  {
      Job.g_restart = (strcmp(restart,"yes") == 0);
  }




  if (!ReadXML(Reader, "RayTracing/TTTables/TTDirectoryName", Job.TTDirName))
      sprintf(Job.TTDirName, ".");
  if (!ReadXML(Reader, "RayTracing/TTTables/TTFilePrefix", Job.TTFilePrefix, false))
      sprintf(Job.TTFilePrefix, "%s", Job.JobName);


  int NSrcz;
  success = success && ReadXML(Reader, "RayTracing/TTTables/Nz", NSrcz);
  Job.NSrc = point3D<int>(0, 0, NSrcz);

  float dzSrc;
  success = success && ReadXML(Reader, "RayTracing/TTTables/dz", dzSrc);
  Job.dxSrc = point3D<float>(0, 0, dzSrc);

  float X1Srcz (0.0f);
  success = success && ReadXML(Reader, "RayTracing/TTTables/zMinDepth", X1Srcz);
  Job.X0Src = point3D<float>(0, 0, -(X1Srcz + (Job.NSrc[2]-1)*Job.dxSrc[2])); 



  int SrcElev;
  if ( !ReadXML(Reader, "RayTracing/Parameters/SrcElev", SrcElev, false))
      SrcElev = 0;

  if (SrcElev == 1)
    {
      Job.SrcElev = true;
      success = success && ReadXML(Reader, "RayTracing/Parameters/ElevFileName", Job.SrcElevFileName);
      
    }
  else 
    Job.SrcElev = false;


  success = success && ReadXML(Reader, "RayTracing/Parameters/ApertureAngle", Job.RayAperture_deg.v);


  if ( Job.RunMode == WAVEFRONTTRACER)
  {
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_first_inline_num", Job.first_inline_num_Rcv);
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_first_xline_num", Job.first_xline_num_Rcv);

      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_nx_along_const_inline", Job.n_xlines_Rcv);
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_ny_along_const_xline", Job.n_inlines_Rcv);

      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dx_along_const_inline", Job.d_between_xlines_Rcv);
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dy_along_const_xline", Job.d_between_inlines_Rcv);

      float Z0Rcv, dzRcv;
      int NzRcv;
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_zMinDepth", Z0Rcv);
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_Nz", NzRcv);
      success = success && ReadXML(Reader, "RayTracing/TTTables/SubSurfaceGrid_dz", dzRcv);
      if (success)
      {
	  Job.dxRcv[2] = dzRcv;
	  Job.NRcv[2] = NzRcv;
	  Job.X0Rcv[2] = -(Z0Rcv + (NzRcv - 1) * dzRcv);
      }
  }

  int NVelz;
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/Nz", NVelz);
  Job.NVel = point3D<int>(0, 0, NVelz);

  float dzVel;
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/dz", dzVel);
  Job.dxVel = point3D<float>(0, 0, dzVel);

  float X1Velz (0.0f);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/zMinDepth", X1Velz);
  Job.X0Vel = point3D<float>(0, 0, -(X1Velz + (Job.NVel[2]-1) * Job.dxVel[2])); 



  success = success && ReadXML(Reader, "RayTracing/VelocityModel/VelFileName", Job.VelFileName);

  int VFileMode = 0;
  if (!ReadXML(Reader, "RayTracing/VelocityModel/VelFileMode", VFileMode, false))
  {
      const char* VelFileNameExtension = get_extension( Job.VelFileName );
      if ( VelFileNameExtension != NULL )
      {
	  if ( strcmp(VelFileNameExtension, "su") == 0)
	      VFileMode = 2;
      }
  }
  switch (VFileMode)
    {
    case static_cast<int>(SEGY_BIGENDIAN):
      Job.PropFileMode = SEGY_BIGENDIAN;
    break;
    case static_cast<int>(SU_LITENDIAN):
      Job.PropFileMode = SU_LITENDIAN;
    break;
    default:
      Job.PropFileMode = UNDEFINED_FILE_MODE;
      
      success = false;
    }

  // set Anisotropy mode and corresponding files
  Job.IsoMode = ISOTROPIC;
  if (ReadXML(Reader, "RayTracing/VelocityModel/EpsilonFileName", Job.VTI_EpsilonFileName, false)
      && ReadXML(Reader, "RayTracing/VelocityModel/DeltaFileName", Job.VTI_DeltaFileName, false))
  {
      Job.IsoMode = VTI;
      if (ReadXML(Reader, "RayTracing/VelocityModel/AlphaFileName", Job.TTI_AlphaFileName, false)
	  && ReadXML(Reader, "RayTracing/VelocityModel/BetaFileName", Job.TTI_BetaFileName, false))
      {
	  Job.IsoMode = TTI;
      }
  }

  if (!ReadXML(Reader, "RayTracing/VelocityModel/smoothOnly", Job.ModelPreprocessingOnly, false))
      Job.ModelPreprocessingOnly = false;

  // Need for preprocessing the velocity file
  Job.PreprocessingVel = false;
  Job.PreprocessingAni = false;
  if (ReadXML(Reader, "RayTracing/VelocityModel/smooth_widht05_velocity", Job.SmoothingLengthVel, false))
  {
      if (Job.SmoothingLengthVel > 0.0f)
      {
	  Job.PreprocessingVel = true;
      }
      else
	  Job.SmoothingLengthVel = 0.0f;
  }
  if (ReadXML(Reader, "RayTracing/VelocityModel/smooth_widht05_ani", Job.SmoothingLengthAni, false))
  {
      if (Job.SmoothingLengthAni > 0.0f)
      {
	  Job.PreprocessingAni = true;
      }
      else
	  Job.SmoothingLengthAni = 0.0f;
  }
  if (ReadXML(Reader, "RayTracing/VelocityModel/coarsening_factor_along_inline", Job.CoarseningFactoralongInline, false))
  {
      if (Job.CoarseningFactoralongInline > 1)
      {
	  Job.PreprocessingVel = true;
	  Job.PreprocessingAni = true;
      }
      else
	  Job.CoarseningFactoralongInline = 1;
  }
  if (ReadXML(Reader, "RayTracing/VelocityModel/coarsening_factor_along_xline", Job.CoarseningFactoralongXline, false))
  {
      if (Job.CoarseningFactoralongXline > 1)
      {
	  Job.PreprocessingVel = true;
	  Job.PreprocessingAni = true;
      }
      else
	  Job.CoarseningFactoralongXline = 1;
  }
  if (ReadXML(Reader, "RayTracing/VelocityModel/coarsening_factor_along_depth", Job.CoarseningFactoralongDepth, false))
  {
      if (Job.CoarseningFactoralongDepth > 1)
      {
	  Job.PreprocessingVel = true;
	  Job.PreprocessingAni = true;
      }
      else
	  Job.CoarseningFactoralongDepth = 1;
  }

  const char* VelFileNameExtension = get_extension( Job.VelFileName );
  if ( VelFileNameExtension != NULL )
  {
      sprintf(Job.SmoothedVelFileName, "%s/%s_smoothedVel.%s", Job.TTDirName, Job.JobName, VelFileNameExtension);
  }
  else
      sprintf(Job.SmoothedVelFileName, "%s/%s_smoothedVel", Job.TTDirName, Job.JobName);


  const char* EpsilonFileNameExtension = get_extension( Job.VTI_EpsilonFileName );
  if ( EpsilonFileNameExtension != NULL )
  {
      sprintf(Job.SmoothedVTI_EpsilonFileName, "%s/%s_smoothedEpsilonVel.%s", Job.TTDirName, Job.JobName, EpsilonFileNameExtension);
  }
  else
      sprintf(Job.SmoothedVTI_EpsilonFileName, "%s/%s_smoothedEpsilonVel", Job.TTDirName, Job.JobName);

  const char* DeltaFileNameExtension = get_extension( Job.VTI_DeltaFileName );
  if ( DeltaFileNameExtension != NULL )
  {
      sprintf(Job.SmoothedVTI_DeltaFileName, "%s/%s_smoothedDeltaVel.%s", Job.TTDirName, Job.JobName, DeltaFileNameExtension);
  }
  else
      sprintf(Job.SmoothedVTI_DeltaFileName, "%s/%s_smoothedDeltaVel", Job.TTDirName, Job.JobName);
  const char* AlphaFileNameExtension = get_extension( Job.TTI_AlphaFileName );
  if ( AlphaFileNameExtension != NULL )
  {
      sprintf(Job.SmoothedTTI_AlphaFileName, "%s/%s_smoothedAlphaVel.%s", Job.TTDirName, Job.JobName, AlphaFileNameExtension);
  }
  else
      sprintf(Job.SmoothedTTI_AlphaFileName, "%s/%s_smoothedAlphaVel", Job.TTDirName, Job.JobName);
  const char* BetaFileNameExtension = get_extension( Job.TTI_BetaFileName );
  if ( BetaFileNameExtension != NULL )
  {
      sprintf(Job.SmoothedTTI_BetaFileName, "%s/%s_smoothedBetaVel.%s", Job.TTDirName, Job.JobName, BetaFileNameExtension);
  }
  else
      sprintf(Job.SmoothedTTI_BetaFileName, "%s/%s_smoothedBetaVel", Job.TTDirName, Job.JobName);

  float MaxTracingTime (0.0f);
  success = success && ReadXML(Reader, "RayTracing/Parameters/MaxTracingTime", MaxTracingTime);
  success = success && ReadXML(Reader, "RayTracing/Parameters/TracingStepSize", Job.g_TSTEPSIZE);
  Job.g_MAXTSTEP = (int)(MaxTracingTime/Job.g_TSTEPSIZE + 0.5);

  if (Job.RunMode == RAYTRACER) {
      OptionalPrm<float> InitAngle;
      InitAngle.given = ReadXML(Reader, "RayTracing/Parameters/InitialAngleResolution", InitAngle.value, false);
      success = success && ReadXML(Reader, "RayTracing/TTTables/AngleResolution", Job.g_DANGLE.v);

      if (InitAngle.given)
      {
	  Job.g_InitAngle = InitAngle.value;
	  if (fabs(Job.g_InitAngle.v-Job.g_DANGLE.v)>1.e-6) 
	  {  
	      
	      success = false;    
	  }
      }
      else
      {
	  Job.g_InitAngle = Job.g_DANGLE;
      }
  }
  else {
      success = success && ReadXML(Reader, "RayTracing/Parameters/InitialAngleResolution", Job.g_InitAngle.v);
      if (! ReadXML(Reader, "RayTracing/Parameters/REF_LEN", Job.g_REF_LEN, false)) Job.g_REF_LEN = 10000;
  }

  if (!ReadXML(Reader, "RayTracing/Parameters/RecordingDepth", Job.RecordingDepth, false))
      Job.RecordingDepth = X1Velz;

// read additional geometry definitions
  success = success && ReadXML(Reader, "RayTracing/TTTables/ny_along_const_xline", Job.n_inlines_Src);
  success = success && ReadXML(Reader, "RayTracing/TTTables/nx_along_const_inline", Job.n_xlines_Src);
  success = success && ReadXML(Reader, "RayTracing/TTTables/first_inline_num", Job.first_inline_num_Src);
  success = success && ReadXML(Reader, "RayTracing/TTTables/first_xline_num", Job.first_xline_num_Src);
  success = success && ReadXML(Reader, "RayTracing/TTTables/dy_along_const_xline", Job.d_between_inlines_Src);
  success = success && ReadXML(Reader, "RayTracing/TTTables/dx_along_const_inline", Job.d_between_xlines_Src);

  success = success && ReadXML(Reader, "RayTracing/VelocityModel/ny_along_const_xline", Job.n_inlines_Vel);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/nx_along_const_inline", Job.n_xlines_Vel);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/first_inline_num", Job.first_inline_num_Vel);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/first_xline_num", Job.first_xline_num_Vel);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/dy_along_const_xline", Job.d_between_inlines_Vel);
  success = success && ReadXML(Reader, "RayTracing/VelocityModel/dx_along_const_inline", Job.d_between_xlines_Vel);

  if (!ReadXML(Reader, "common/Geometry/one_inline_num", Job.geom.one_inline_num))
      Job.geom.one_inline_num = 0;
  if (!ReadXML(Reader, "common/Geometry/one_xline_num", Job.geom.one_xline_num))
      Job.geom.one_xline_num = 0;
  if (!ReadXML(Reader, "common/Geometry/one_x", Job.geom.x_one))
      Job.geom.x_one = 0;
  if (!ReadXML(Reader, "common/Geometry/one_y", Job.geom.y_one))
      Job.geom.y_one = 0;
  success = success && ReadXML(Reader, "common/Geometry/dy_between_inlines", Job.geom.d_between_inlines);
  success = success && ReadXML(Reader, "common/Geometry/dx_between_xlines", Job.geom.d_between_xlines);
  success = success && ReadXML(Reader, "common/Geometry/angle_rot_deg", Job.geom.angle_rot_deg.v);
  success = success && ReadXML(Reader, "common/Geometry/inlinedir", Job.geom.inlinedir);
// end read additional geometry definitions


  Scale(Job);

  if (!success)
      return -1;
  else
      return 0;
}

int CheckReadTracingJob::CheckExistence()
{
  FILE * pFile = fopen(CfgFileName, "r");
  if ( pFile == NULL )
    return -1;
  else
    {
      fclose(pFile);
      return 0;
    }
}

bool CheckReadTracingJob::ReadXML(XMLReader& Reader, const char* VarName, int& Var, bool message)
{
    if (!Reader.getInt(VarName, Var))
    {
      if (message)
	
      return false;
    }
  return true;
}

bool CheckReadTracingJob::ReadXML(XMLReader& Reader, const char* VarName, float& Var, bool message)
{
  if (!Reader.getFloat(VarName, Var))
    {
      if (message)
	
      return false;
    }
  return true;
}

bool CheckReadTracingJob::ReadXML(XMLReader& Reader, const char* VarName, char* Var, bool message)
{
  if (!Reader.getChar(VarName, Var))
    {
      if (message)
	
      return false;
    }
  return true;
}

bool CheckReadTracingJob::ReadXML(XMLReader& Reader, const char* VarName, bool& Var, bool message)
{
    char VarString[10];
    if (!Reader.getChar(VarName, VarString))
    {
	if (message)
	    
	return false;
    }

    Var = (strcmp(VarString,"yes") == 0);
     
    return true;
}

int CheckReadTracingJob::WriteConfigFileXML(char* ConfigFileName, const TracingJob& Job)
{
  if ( ConfigFileName == NULL )
    {
      
      return(-1);
    }

  chmod(ConfigFileName, S_IWUSR | S_IWGRP | S_IROTH );
  std::ofstream ConfigFile(ConfigFileName, std::ios::out);
  if ( ConfigFile.fail())
    {
      
      return -1;
    }

  ConfigFile << "<?xml version = '1.0'?>\n";
  ConfigFile << "<GRT3D_version_" << SEISRAY_VERSION << ">\n";
  ConfigFile << "  <common>\n";
  ConfigFile << "     <Parameters>\n";
  ConfigFile << "        <ProjectName>" << Job.ProjectName << "</ProjectName>\n";
  char LogFile[199];
  strcpy(LogFile, Job.LogFile);
  ConfigFile << "        <LoggingDirectory>" << dirname(LogFile) << "</LoggingDirectory>\n";
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
  ConfigFile << "  <RayTracing>\n";
  ConfigFile << "     <Parameters>\n";
  ConfigFile << "       <jobname_R>" << Job.JobName << "</jobname_R>\n";
  ConfigFile << "       <LoggingLevel values=\"0,1,2,3\">" << Job.LogLevel << "</LoggingLevel>\n";
  ConfigFile << "       <Restart values=\"no,yes\">";
  if (Job.g_restart)
      ConfigFile << "yes";
  else
      ConfigFile << "no";
  ConfigFile << "</Restart>\n";
  ConfigFile << "       <ApertureAngle unit=\"[deg]\">" << Job.RayAperture_deg.v << "</ApertureAngle>\n";
  ConfigFile << "       <MaxTracingTime unit=\"[s]\">" << Job.g_MAXTSTEP * Job.g_TSTEPSIZE  << "</MaxTracingTime>\n";
  ConfigFile << "       <TracingStepSize unit=\"[s]\">" << Job.g_TSTEPSIZE << "</TracingStepSize>\n";
  ConfigFile << "       <RecordingDepth unit=\"[m]\">" << Job.RecordingDepth << "</RecordingDepth>\n";
  ConfigFile << "       <InitialAngleResolution unit=\"[deg]\">" << Job.g_InitAngle.v << "</InitialAngleResolution>\n";
  if (Job.RunMode != RAYTRACER)
      {
	  ConfigFile << "       <TracingMode>"<< 0 << "</TracingMode>\n";
	  ConfigFile << "       <REF_LEN>" << Job.g_REF_LEN << "</REF_LEN>\n";  
	  ConfigFile << "       <SrcElev>" << Job.SrcElev << "</SrcElev>\n";
	  ConfigFile << "       <ElevFileName>" << Job.SrcElevFileName << "</ElevFileName>\n";
      }
  ConfigFile << "     </Parameters>\n";
  ConfigFile << "     <VelocityModel>\n";
  ConfigFile << "       <VelFileName>" << Job.VelFileName << "</VelFileName>\n";
  if (Job.IsoMode == VTI)
  {
      ConfigFile << "       <EpsilonFileName>" << Job.VTI_EpsilonFileName << "</EpsilonFileName>\n";
      ConfigFile << "       <DeltaFileName>" << Job.VTI_DeltaFileName << "</DeltaFileName>\n";
  }
  if (Job.IsoMode == TTI)
  {
      ConfigFile << "       <EpsilonFileName>" << Job.VTI_EpsilonFileName << "</EpsilonFileName>\n";
      ConfigFile << "       <DeltaFileName>" << Job.VTI_DeltaFileName << "</DeltaFileName>\n";
      ConfigFile << "       <AlphaFileName>" << Job.TTI_AlphaFileName << "</AlphaFileName>\n";
      ConfigFile << "       <BetaFileName>" << Job.TTI_BetaFileName << "</BetaFileName>\n";
  }

  ConfigFile << "       <VelFileMode>" << Job.PropFileMode << "</VelFileMode>\n";
  ConfigFile << "       <zMinDepth unit=\"[m]\">" << -(Job.X0Vel[2] + (Job.NVel[2]-1)*Job.dxVel[2]) << "</zMinDepth>\n";
  ConfigFile << "       <Nz>" << Job.NVel[2] << "</Nz>\n";
  ConfigFile << "       <dz unit=\"[m]\">" << Job.dxVel[2] << "</dz>\n";
  ConfigFile << "       <first_inline_num>" << Job.first_inline_num_Vel << "</first_inline_num>\n";
  ConfigFile << "       <first_xline_num>" << Job.first_xline_num_Vel << "</first_xline_num>\n";
  ConfigFile << "       <nx_along_const_inline>" << Job.n_xlines_Vel << "</nx_along_const_inline>\n";
  ConfigFile << "       <dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_Vel << "</dx_along_const_inline>\n";
  ConfigFile << "       <ny_along_const_xline>" << Job.n_inlines_Vel << "</ny_along_const_xline>\n";
  ConfigFile << "       <dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_Vel << "</dy_along_const_xline>\n";
  if (Job.ModelPreprocessingOnly)
      ConfigFile << "       <smoothOnly values=\"no,yes\">yes</smoothOnly>\n";
  else
      ConfigFile << "       <smoothOnly values=\"no,yes\">no</smoothOnly>\n";

  if (Job.PreprocessingVel)
  {
      ConfigFile << "       <smooth_widht05_velocity unit=\"[m]\">" << Job.SmoothingLengthVel << "</smooth_widht05_velocity>\n";
  }
  if (Job.PreprocessingAni)
  {
      ConfigFile << "       <smooth_widht05_ani unit=\"[m]\">" << Job.SmoothingLengthAni << "</smooth_widht05_ani>\n";
  }
  if ( (Job.PreprocessingVel) || (Job.PreprocessingAni) )
  {
      ConfigFile << "       <coarsening_factor_along_inline>" << Job.CoarseningFactoralongInline << "</coarsening_factor_along_inline>\n";
      ConfigFile << "       <coarsening_factor_along_xline>" << Job.CoarseningFactoralongXline << "</coarsening_factor_along_xline>\n";
      ConfigFile << "       <coarsening_factor_along_depth>" << Job.CoarseningFactoralongDepth << "</coarsening_factor_along_depth>\n";
  }

  ConfigFile << "     </VelocityModel>\n";

  // TT Table file and geometry definition
  ConfigFile << "     <TTTables>\n";
  ConfigFile << "        <TTDirectoryName>" << Job.TTDirName << "</TTDirectoryName>\n";
  ConfigFile << "        <TTFilePrefix>" << Job.TTFilePrefix << "</TTFilePrefix>\n";
  if (Job.RunMode == RAYTRACER)
  {
      ConfigFile << "        <zMinDepth unit=\"[m]\">" << -(Job.X0Src[2] + (Job.NSrc[2]-1)*Job.dxSrc[2]) << "</zMinDepth>\n";
      ConfigFile << "        <Nz>" << Job.NSrc[2] << "</Nz>\n";
      ConfigFile << "        <dz unit=\"[m]\">" << Job.dxSrc[2] << "</dz>\n";
      ConfigFile << "        <first_inline_num>" << Job.first_inline_num_Src << "</first_inline_num>\n";
      ConfigFile << "        <first_xline_num>" << Job.first_xline_num_Src << "</first_xline_num>\n";
      ConfigFile << "        <nx_along_const_inline>" << Job.n_xlines_Src << "</nx_along_const_inline>\n";
      ConfigFile << "        <dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_Src << "</dx_along_const_inline>\n";
      ConfigFile << "        <ny_along_const_xline>" << Job.n_inlines_Src << "</ny_along_const_xline>\n";
      ConfigFile << "        <dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_Src << "</dy_along_const_xline>\n";
      ConfigFile << "        <AngleResolution unit=\"[deg]\">" << Job.g_DANGLE.v << "</AngleResolution>\n";
  }

  if (Job.RunMode == WAVEFRONTTRACER)
  {
      ConfigFile << "        <zMinDepth unit=\"[m]\">" << -(Job.X0Src[2] + (Job.NSrc[2]-1)*Job.dxSrc[2]) << "</zMinDepth>\n";
      ConfigFile << "        <Nz>" << Job.NSrc[2] << "</Nz>\n";
      ConfigFile << "        <dz unit=\"[m]\">" << Job.dxSrc[2] << "</dz>\n";
      ConfigFile << "        <first_inline_num>" << Job.first_inline_num_Src << "</first_inline_num>\n";
      ConfigFile << "        <first_xline_num>" << Job.first_xline_num_Src << "</first_xline_num>\n";
      ConfigFile << "        <nx_along_const_inline>" << Job.n_xlines_Src << "</nx_along_const_inline>\n";
      ConfigFile << "        <dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_Src << "</dx_along_const_inline>\n";
      ConfigFile << "        <ny_along_const_xline>" << Job.n_inlines_Src << "</ny_along_const_xline>\n";
      ConfigFile << "        <dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_Src << "</dy_along_const_xline>\n";
      ConfigFile << "\n";
      ConfigFile << "        <SubSurfaceGrid_first_inline_num>" << Job.first_inline_num_Rcv << "</SubSurfaceGrid_first_inline_num>\n";
      ConfigFile << "        <SubSurfaceGrid_first_xline_num>" << Job.first_xline_num_Rcv << "</SubSurfaceGrid_first_xline_num>\n";
      ConfigFile << "        <SubSurfaceGrid_nx_along_const_inline>" << Job.n_xlines_Rcv << "</SubSurfaceGrid_nx_along_const_inline>\n";
      ConfigFile << "        <SubSurfaceGrid_ny_along_const_xline>" << Job.n_inlines_Rcv << "</SubSurfaceGrid_ny_along_const_xline>\n";
      ConfigFile << "        <SubSurfaceGrid_dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_Rcv << "</SubSurfaceGrid_dx_along_const_inline>\n";
      ConfigFile << "        <SubSurfaceGrid_dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_Rcv << "</SubSurfaceGrid_dy_along_const_xline>\n";
      ConfigFile << "        <SubSurfaceGrid_zMinDepth unit=\"[m]\">" << -Job.X0Rcv[2] - (Job.NRcv[2] - 1) *  Job.dxRcv[2] << "</SubSurfaceGrid_zMinDepth>\n";
      ConfigFile << "        <SubSurfaceGrid_Nz>" << Job.NRcv[2] << "</SubSurfaceGrid_Nz>\n";
      ConfigFile << "        <SubSurfaceGrid_dz unit=\"[m]\">" << Job.dxRcv[2] << "</SubSurfaceGrid_dz>\n";
      ConfigFile << "\n";
      ConfigFile << "        <X0Rcvx>" << Job.X0Rcv[0] << "</X0Rcvx>\n";
      ConfigFile << "        <X0Rcvy>" << Job.X0Rcv[1] << "</X0Rcvy>\n";
      ConfigFile << "        <X0Rcvz>" << Job.X0Rcv[2] << "</X0Rcvz>\n";
      ConfigFile << "        <NxRcv>" << Job.NRcv[0] << "</NxRcv>\n";
      ConfigFile << "        <NyRcv>" << Job.NRcv[1] << "</NyRcv>\n";
      ConfigFile << "        <NzRcv>" << Job.NRcv[2] << "</NzRcv>\n";
      ConfigFile << "        <dxRcv>" << Job.dxRcv[0] << "</dxRcv>\n";
      ConfigFile << "        <dyRcv>" << Job.dxRcv[1] << "</dyRcv>\n";
      ConfigFile << "        <dzRcv>" << Job.dxRcv[2] << "</dzRcv>\n";
      ConfigFile << "\n";
  }
  ConfigFile << "    </TTTables>\n";
  // end TT Table file and geometry definition

  ConfigFile << "  </RayTracing>\n";
  ConfigFile << "</GRT3D_version_" << SEISRAY_VERSION << ">\n";

  ConfigFile.close();
  chmod(ConfigFileName,S_IRUSR | S_IRGRP | S_IROTH );
  return 0;
}

int CheckReadTracingJob::WriteTTConfigFileXML(char* ConfigFileName, const TracingJob& Job)
{
  if ( ConfigFileName == NULL )
    {
      
      return(-1);
    }

  std::ofstream ConfigFile(ConfigFileName, std::ios::out);

  if (ConfigFile.fail())
  {
      
      return -1;
  }
  else
  {
      ConfigFile << "<?xml version = '1.0'?>\n";
      ConfigFile << "<GRT3D_version_" << SEISRAY_VERSION << ">\n";
      ConfigFile << "   <TTTables>\n";
      ConfigFile << "      <TTDirectoryName>" << Job.TTDirName << "</TTDirectoryName>\n";
      ConfigFile << "      <TTFilePrefix>" << Job.TTFilePrefix << "</TTFilePrefix>\n";
      ConfigFile << "      <zMinDepth unit=\"[m]\">" << -(Job.X0Src[2] + (Job.NSrc[2]-1)*Job.dxSrc[2]) << "</zMinDepth>\n";
      ConfigFile << "      <Nz>" << Job.NSrc[2] << "</Nz>\n";
      ConfigFile << "      <dz>" << Job.dxSrc[2] << "</dz>\n";
      ConfigFile << "      <first_inline_num>" << Job.first_inline_num_Src << "</first_inline_num>\n";
      ConfigFile << "      <first_xline_num>" << Job.first_xline_num_Src << "</first_xline_num>\n";
      ConfigFile << "      <nx_along_const_inline>" << Job.n_xlines_Src << "</nx_along_const_inline>\n";
      ConfigFile << "      <dx_along_const_inline unit=\"[m]\">" << Job.d_between_xlines_Src << "</dx_along_const_inline>\n";
      ConfigFile << "      <ny_along_const_xline>" << Job.n_inlines_Src << "</ny_along_const_xline>\n";
      ConfigFile << "      <dy_along_const_xline unit=\"[m]\">" << Job.d_between_inlines_Src << "</dy_along_const_xline>\n";
      if (Job.RunMode == RAYTRACER)
	ConfigFile << "      <AngleResolution unit=\"[deg]\">" << Job.g_DANGLE.v << "</AngleResolution>\n";
      ConfigFile << "  </TTTables>\n";
      ConfigFile << "</GRT3D_version_" << SEISRAY_VERSION << ">\n";
  }

  if (ConfigFile.fail())
  {
      
      return -1;
  }

  ConfigFile.close();
  chmod(ConfigFileName, S_IRUSR | S_IRGRP | S_IROTH );
  return 0;
}

void CheckReadTracingJob::Scale(TracingJob& Job)
{
    Job.geom.first_inline_num = Job.first_inline_num_Vel;
    Job.geom.first_xline_num = Job.first_xline_num_Vel;
    Job.geom.n_inlines = Job.n_inlines_Vel;
    Job.geom.n_xlines = Job.n_xlines_Vel;

    Acq_geometry<float> Geom(Job.geom);

    Job.X0Vel[0] = 0.0f;
    Job.X0Vel[1] = 0.0f;
//     Geom.inlxl_to_WORLDxy(Job.first_inline_num_Vel, Job.first_xline_num_Vel,
// 			  &Job.X0Vel[0], &Job.X0Vel[1]);
//     Geom.WORLDxy_to_MODxy(Job.X0Vel[0], Job.X0Vel[1],
// 			  &Job.X0Vel[0], &Job.X0Vel[1]);


    Job.X0Src[0] = (Job.first_xline_num_Src - Job.first_xline_num_Vel) * Job.geom.d_between_xlines;
    Job.X0Src[1] = (Job.first_inline_num_Src - Job.first_inline_num_Vel) * Job.geom.d_between_inlines;
//     Geom.inlxl_to_WORLDxy(Job.first_inline_num_Src, Job.first_xline_num_Src,
// 			  &Job.X0Src[0], &Job.X0Src[1]);
//     Geom.WORLDxy_to_MODxy(Job.X0Src[0], Job.X0Src[1],
// 			  &Job.X0Src[0], &Job.X0Src[1]);

    Job.X0Rcv[0] = (Job.first_xline_num_Rcv - Job.first_xline_num_Vel) * Job.geom.d_between_xlines;
    Job.X0Rcv[1] = (Job.first_inline_num_Rcv - Job.first_inline_num_Vel) * Job.geom.d_between_inlines;
//     Geom.inlxl_to_WORLDxy(Job.first_inline_num_Rcv, Job.first_xline_num_Rcv,
// 			  &Job.X0Rcv[0], &Job.X0Rcv[1]);
//     Geom.WORLDxy_to_MODxy(Job.X0Rcv[0], Job.X0Rcv[1],
// 			  &Job.X0Rcv[0], &Job.X0Rcv[1]);

    Job.NVel[0] = Job.n_xlines_Vel;
    Job.NVel[1] = Job.n_inlines_Vel;
    Job.dxVel[0] = Job.d_between_xlines_Vel;
    Job.dxVel[1] = Job.d_between_inlines_Vel;

    Job.NSrc[0] = Job.n_xlines_Src;
    Job.NSrc[1] = Job.n_inlines_Src;
    Job.dxSrc[0] = Job.d_between_xlines_Src;
    Job.dxSrc[1] = Job.d_between_inlines_Src;

    Job.NRcv[0] = Job.n_xlines_Rcv;
    Job.NRcv[1] = Job.n_inlines_Rcv;
    Job.dxRcv[0] = Job.d_between_xlines_Rcv;
    Job.dxRcv[1] = Job.d_between_inlines_Rcv;

    Job.X0Vol[0] = Job.X0Vel[0];
    Job.X0Vol[1] = Job.X0Vel[1];
    Job.X1Vol[0] = Job.X0Vel[0] + (Job.NVel[0] - 1) * Job.dxVel[0];
    Job.X1Vol[1] = Job.X0Vel[1] + (Job.NVel[1] - 1) * Job.dxVel[1];
    Job.X0Vol[2] = Job.X0Vel[2];
    Job.X1Vol[2] = -Job.RecordingDepth;
//     Job.X1Vol[2] = Job.X0Vel[2] + (Job.NVel[2] - 1) * Job.dxVel[2];;


    Job.N0Src = point3D<int>(0, 0, 0);
    Job.NtotSrc = Job.NSrc;
//     point3D<float> X0Ref(Job.x_one, Job.y_one, 0);
//     for (int i = 0; i < 2; i++)
//     {
// 	Job.X0Vol[i] = Job.X0Vol[i] - X0Ref[i];
// 	Job.X1Vol[i] = Job.X1Vol[i] - X0Ref[i];
// 	//Job.X0Vel[i] = Job.X0Vel[i] - Job.X0Ref[i];
// 	Job.X0Src[i] = Job.X0Src[i] - X0Ref[i];
// 	Job.X0Rcv[i] = Job.X0Rcv[i] - X0Ref[i];
//     }
}


///// old Variables :
//   ConfigFile << "       <X0Velx>" << Job.X0Vel[0] << "</X0Velx>\n";
//   ConfigFile << "       <X0Vely>" << Job.X0Vel[1] << "</X0Vely>\n";
//   ConfigFile << "       <NxVel>" << Job.NVel[0] << "</NxVel>\n";
//   ConfigFile << "       <NyVel>" << Job.NVel[1] << "</NyVel>\n";
//   ConfigFile << "       <dxVel>" << Job.dxVel[0] << "</dxVel>\n";
//   ConfigFile << "       <dyVel>" << Job.dxVel[1] << "</dyVel>\n";
//   ConfigFile << "     <X0Volx>" << Job.X0Vol[0] << "</X0Volx>\n";
//   ConfigFile << "     <X0Voly>" << Job.X0Vol[1] << "</X0Voly>\n";
//   ConfigFile << "     <X0Volz>" << Job.X0Vol[2] << "</X0Volz>\n";
//   ConfigFile << "     <X1Volx>" << Job.X1Vol[0] << "</X1Volx>\n";
//   ConfigFile << "     <X1Voly>" << Job.X1Vol[1] << "</X1Voly>\n";
//   ConfigFile << "     <X1Volz>" << Job.X1Vol[2] << "</X1Volz>\n";
//   ConfigFile << "     <X0Srcx>" << Job.X0Src[0] << "</X0Srcx>\n";
//   ConfigFile << "     <X0Srcy>" << Job.X0Src[1] << "</X0Srcy>\n";
//   ConfigFile << "     <NxSrc>" << Job.NSrc[0] << "</NxSrc>\n";
//   ConfigFile << "     <NySrc>" << Job.NSrc[1] << "</NySrc>\n";
//   ConfigFile << "     <NtotxSrc>" << Job.NtotSrc[0] << "</NtotxSrc>\n";
//   ConfigFile << "     <NtotySrc>" << Job.NtotSrc[1] << "</NtotySrc>\n";
//   ConfigFile << "     <NtotzSrc>" << Job.NtotSrc[2] << "</NtotzSrc>\n";
//   ConfigFile << "     <N0xSrc>" << Job.N0Src[0] << "</N0xSrc>\n";
//   ConfigFile << "     <N0ySrc>" << Job.N0Src[1] << "</N0ySrc>\n";
//   ConfigFile << "     <N0zSrc>" << Job.N0Src[2] << "</N0zSrc>\n";
//   ConfigFile << "     <dxSrc>" << Job.dxSrc[0] << "</dxSrc>\n";
//   ConfigFile << "     <dySrc>" << Job.dxSrc[1] << "</dySrc>\n";

int CheckReadTracingJob::AddWorkingDir(char* CurrentDirectory, TracingJob& Job)
{
    char TmpFileName[199];
    if ( (strlen(Job.LogFile) > 0) && (strncmp("/", Job.LogFile, 1) != 0))
    {
	sprintf(TmpFileName, "%s/%s", CurrentDirectory, Job.LogFile);
	sprintf(Job.LogFile, "%s", TmpFileName);
    }

    if ( (strlen(Job.VelFileName) > 0) && (strncmp("/", Job.VelFileName, 1) != 0))
    {
	sprintf(TmpFileName, "%s/%s", CurrentDirectory, Job.VelFileName);
	sprintf(Job.VelFileName, "%s", TmpFileName);
    }

    if ( (strlen(Job.TTDirName) > 0) && (strncmp("/", Job.TTDirName, 1) != 0))
    {
	sprintf(TmpFileName, "%s/%s", CurrentDirectory, Job.TTDirName);
	sprintf(Job.TTDirName, "%s", TmpFileName);
    }

    if ( (strlen(Job.SrcElevFileName) > 0) && (strncmp("/", Job.SrcElevFileName, 1) != 0))
    {
	sprintf(TmpFileName, "%s/%s", CurrentDirectory, Job.SrcElevFileName);
	sprintf(Job.SrcElevFileName, "%s", TmpFileName);
    }
    return 0;
}
