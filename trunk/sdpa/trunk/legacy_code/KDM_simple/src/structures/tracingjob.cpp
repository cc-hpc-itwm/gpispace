/***************************************************************************
                          tracingjob.cpp  -  description
                             -------------------
    begin                : Thu Apr 6 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "tracingjob.h"

TracingJob::TracingJob()
{
    sprintf(ProjectName, "");
    sprintf(JobName, "");
    sprintf(LogFile, "");

    LogLevel = 1;

    sprintf(VelFileName, "");
    sprintf(VTI_EpsilonFileName, "");
    sprintf(VTI_DeltaFileName, "");
    sprintf(TTI_AlphaFileName, "");
    sprintf(TTI_BetaFileName, "");

    sprintf(SmoothedVelFileName, "");
    sprintf(SmoothedVTI_EpsilonFileName, "");
    sprintf(SmoothedVTI_DeltaFileName, "");
    sprintf(SmoothedTTI_AlphaFileName, "");
    sprintf(SmoothedTTI_BetaFileName, "");

    sprintf(TTDirName, "");
    sprintf(TTFilePrefix, "");

    sprintf(SrcElevFileName, "");

    RunMode = RAYTRACER;
    IsoMode = ISOTROPIC;

    RayAperture_deg = 180.f;

    // Initialize the flags and conditional parameters
    g_restart = false;
    ModelPreprocessingOnly = false;
    PreprocessingVel = false;
    PreprocessingAni = false;
}
TracingJob::~TracingJob(){
}

// TracingJob::TracingJob(const TracingJob& Job)
// {
//     memcpy(ProjectName,Job.ProjectName,199);
//     memcpy(JobName,Job.JobName,199);
// 
//     LogLevel = Job.LogLevel;
//     memcpy(LogFile,Job.LogFile,199);
// 
//     geom = Job.geom;
// 
//     RunMode = Job.RunMode;
//     IsoMode = Job.IsoMode;
// 
//     X0Vol = Job.X0Vol;
//     X1Vol = Job.X1Vol;
//     RecordingDepth = Job.RecordingDepth;
// 
//     memcpy(VelFileName,Job.VelFileName,199);
//     memcpy(VTI_EpsilonFileName,Job.VTI_EpsilonFileName,199);
//     memcpy(VTI_DeltaFileName,Job.VTI_DeltaFileName,199);
//     memcpy(TTI_AlphaFileName,Job.TTI_AlphaFileName,199);
//     memcpy(TTI_BetaFileName,Job.TTI_BetaFileName,199);
// 
//     PropFileMode = Job.PropFileMode;
// 
//     n_inlines_Vel = Job.n_inlines_Vel;
//     n_xlines_Vel = Job.n_xlines_Vel;
//     d_between_inlines_Vel = Job.d_between_inlines_Vel;
//     d_between_xlines_Vel = Job.d_between_xlines_Vel;
//     first_inline_num_Vel = Job.first_inline_num_Vel;
//     first_xline_num_Vel = Job.first_xline_num_Vel;
// 
//     X0Vel = Job.X0Vel;
//     dxVel = Job.dxVel;
//     NVel = Job.NVel;
// 
//     memcpy(TTDirName,Job.TTDirName,199);
//     memcpy(TTFilePrefix,Job.TTFilePrefix,199);
// 
//     n_inlines_Src = Job.n_inlines_Src;
//     n_xlines_Src = Job.n_xlines_Src;
//     d_between_inlines_Src = Job.d_between_inlines_Src;
//     d_between_xlines_Src = Job.d_between_xlines_Src;
//     first_inline_num_Src = Job.first_inline_num_Src;
//     first_xline_num_Src = Job.first_xline_num_Src;
// 
//     X0Src = Job.X0Src;
//     dxSrc = Job.dxSrc;
//     NtotSrc = Job.NtotSrc;
//     N0Src = Job.N0Src;
//     NSrc = Job.NSrc;
// 
//     SrcElev = Job.SrcElev;
//     memcpy(SrcElevFileName,Job.SrcElevFileName,199);
// 
//     n_inlines_Rcv = Job.n_inlines_Rcv;
//     n_xlines_Rcv = Job.n_xlines_Rcv;
//     d_between_inlines_Rcv = Job.d_between_inlines_Rcv;
//     d_between_xlines_Rcv = Job.d_between_xlines_Rcv;
//     first_inline_num_Rcv = Job.first_inline_num_Rcv;
//     first_xline_num_Rcv = Job.first_xline_num_Rcv;
// 
//     X0Rcv = Job.X0Rcv;
//     dxRcv = Job.dxRcv;
//     NRcv = Job.NRcv;
// 
//     RayAperture_deg = Job.RayAperture_deg;
// 
//     Offs = Job.Offs;
// 
//     g_InitAngle = Job.g_InitAngle;
// 
//     g_REF_LEN = Job.g_REF_LEN;
//     g_DANGLE = Job.g_DANGLE;
// 
//     g_restart = Job.g_restart;
// 
//     g_MAXTSTEP = Job.g_MAXTSTEP;
//     g_TSTEPSIZE = Job.g_TSTEPSIZE;
// 
//	 ModelPreprocessingOnly = Job.ModelPreprocessingOnly;
//	 PreprocessingVel = Job.PreprocessingVel;
//	 PreprocessingAni = Job.PreprocessingAni;
//
//     SmoothingLengthVel = Job.SmoothingLengthVel;
//     SmoothingLengthAni = Job.SmoothingLengthAni;
// 
//	 CoarseningFactoralongInline  = Job.CoarseningFactoralongInline;
//	 CoarseningFactoralongXline  = Job.CoarseningFactoralongXline;
//	 CoarseningFactoralongDepth  = Job.CoarseningFactoralongDepth;
// 
//     memcpy(SmoothedVelFileName,Job.SmoothedVelFileName,199);
//     memcpy(SmoothedVTI_EpsilonFileName,Job.SmoothedVTI_EpsilonFileName,199);
//     memcpy(SmoothedVTI_DeltaFileName,Job.SmoothedVTI_DeltaFileName,199);
//     memcpy(SmoothedTTI_AlphaFileName,Job.SmoothedTTI_AlphaFileName,199);
//     memcpy(SmoothedTTI_BetaFileName,Job.SmoothedTTI_BetaFileName,199);
// 
// }
// 
// TracingJob& TracingJob::operator=(const TracingJob& Job)
// {
//     memcpy(ProjectName,Job.ProjectName,199);
//     memcpy(JobName,Job.JobName,199);
// 
//     LogLevel = Job.LogLevel;
//     memcpy(LogFile,Job.LogFile,199);
// 
//     geom = Job.geom;
// 
//     RunMode = Job.RunMode;
//     IsoMode = Job.IsoMode;
// 
//     X0Vol = Job.X0Vol;
//     X1Vol = Job.X1Vol;
//     RecordingDepth = Job.RecordingDepth;
// 
//     memcpy(VelFileName,Job.VelFileName,199);
//     memcpy(VTI_EpsilonFileName,Job.VTI_EpsilonFileName,199);
//     memcpy(VTI_DeltaFileName,Job.VTI_DeltaFileName,199);
//     memcpy(TTI_AlphaFileName,Job.TTI_AlphaFileName,199);
//     memcpy(TTI_BetaFileName,Job.TTI_BetaFileName,199);
// 
//     PropFileMode = Job.PropFileMode;
// 
//     n_inlines_Vel = Job.n_inlines_Vel;
//     n_xlines_Vel = Job.n_xlines_Vel;
//     d_between_inlines_Vel = Job.d_between_inlines_Vel;
//     d_between_xlines_Vel = Job.d_between_xlines_Vel;
//     first_inline_num_Vel = Job.first_inline_num_Vel;
//     first_xline_num_Vel = Job.first_xline_num_Vel;
// 
//     X0Vel = Job.X0Vel;
//     dxVel = Job.dxVel;
//     NVel = Job.NVel;
// 
//     memcpy(TTDirName,Job.TTDirName,199);
//     memcpy(TTFilePrefix,Job.TTFilePrefix,199);
// 
//     n_inlines_Src = Job.n_inlines_Src;
//     n_xlines_Src = Job.n_xlines_Src;
//     d_between_inlines_Src = Job.d_between_inlines_Src;
//     d_between_xlines_Src = Job.d_between_xlines_Src;
//     first_inline_num_Src = Job.first_inline_num_Src;
//     first_xline_num_Src = Job.first_xline_num_Src;
// 
//     X0Src = Job.X0Src;
//     dxSrc = Job.dxSrc;
//     NtotSrc = Job.NtotSrc;
//     N0Src = Job.N0Src;
//     NSrc = Job.NSrc;
// 
//     SrcElev = Job.SrcElev;
//     memcpy(SrcElevFileName,Job.SrcElevFileName,199);
// 
//     n_inlines_Rcv = Job.n_inlines_Rcv;
//     n_xlines_Rcv = Job.n_xlines_Rcv;
//     d_between_inlines_Rcv = Job.d_between_inlines_Rcv;
//     d_between_xlines_Rcv = Job.d_between_xlines_Rcv;
//     first_inline_num_Rcv = Job.first_inline_num_Rcv;
//     first_xline_num_Rcv = Job.first_xline_num_Rcv;
// 
//     X0Rcv = Job.X0Rcv;
//     dxRcv = Job.dxRcv;
//     NRcv = Job.NRcv;
// 
//     RayAperture_deg = Job.RayAperture_deg;
// 
//     Offs = Job.Offs;
// 
//     g_InitAngle = Job.g_InitAngle;
// 
//     g_REF_LEN = Job.g_REF_LEN;
//     g_DANGLE = Job.g_DANGLE;
// 
//     g_restart = Job.g_restart;
// 
//     g_MAXTSTEP = Job.g_MAXTSTEP;
//     g_TSTEPSIZE = Job.g_TSTEPSIZE;
//
//	 ModelPreprocessingOnly = Job.ModelPreprocessingOnly;
//	 PreprocessingVel = Job.PreprocessingVel;
//	 PreprocessingAni = Job.PreprocessingAni;
//
//     SmoothingLengthVel = Job.SmoothingLengthVel;
//     SmoothingLengthAni = Job.SmoothingLengthAni;
// 
//	 CoarseningFactoralongInline  = Job.CoarseningFactoralongInline;
//	 CoarseningFactoralongXline  = Job.CoarseningFactoralongXline;
//	 CoarseningFactoralongDepth  = Job.CoarseningFactoralongDepth;
// 
//     memcpy(SmoothedVelFileName,Job.SmoothedVelFileName,199);
//     memcpy(SmoothedVTI_EpsilonFileName,Job.SmoothedVTI_EpsilonFileName,199);
//     memcpy(SmoothedVTI_DeltaFileName,Job.SmoothedVTI_DeltaFileName,199);
//     memcpy(SmoothedTTI_AlphaFileName,Job.SmoothedTTI_AlphaFileName,199);
//     memcpy(SmoothedTTI_BetaFileName,Job.SmoothedTTI_BetaFileName,199);
// 
// 
//     return *this;
// }

