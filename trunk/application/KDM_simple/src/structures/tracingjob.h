/***************************************************************************
                          tracingjob.h  -  description

    Complete description of a job for Seismic Ray Tracing. 

                             -------------------
    begin                : Thu Apr 6 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-11
                AperAng_deg renamed to RayAperture_deg

       merten | 2010-01-06
                aperture angle changed to float and renamed to AperAng_deg

       merten | 2009-11-05
                new variables for TTI added

       merten | 2009-11-05
                new variables for velicity file pre-processing included

      micheld | 2009-08-07
		parameter controlling rays per source changed from g_RAY_MIN to g_InitAngle

***************************************************************************/


#ifndef TRACINGJOB_H
#define TRACINGJOB_H


/**
 *@author Dirk Merten
 */

#include "include/tracingtypes.h"
#include "include/migtypes.h"
#include "structures/geomdefstruct.hpp"
#include "structures/point3d.h"
#include "structures/deg.h"

class TracingJob {

// public methods
 public: 
    TracingJob();
    ~TracingJob();
//     TracingJob(const TracingJob& Job);
//     TracingJob& operator=(const TracingJob& Job);

// private methods
 private:


// public attributes
 public: 
    /** Name of the project */
    char ProjectName[199];
    /** Name of the job */
    char JobName[199];

    /** Level of logging information */
    int LogLevel;
    /** Name of the log file */
    char LogFile[199];

    /** Definition of the reference geometry frame */
    geomdefstruct geom;

    /** Run Mode of the ray tracer, i.e. single ray or wave front */
    TRACING_MODE RunMode;

    /** Isotropy Mode of the ray tracer and the underlying velocity model */
    ISOTROPY_MODE IsoMode;

    /** corners of the whole volume to be considered */
    point3D<float> X0Vol, X1Vol;
    /** z coordinate of recording Surface */
    float RecordingDepth;

    /**  Name of Velocity File */
    char VelFileName[199];
    /**  Name of VTI Velocity Files */
    char VTI_EpsilonFileName[199];
    char VTI_DeltaFileName[199];
    /**  Name of TTI Velocity Files */
    char TTI_AlphaFileName[199];
    char TTI_BetaFileName[199];

    /**  Type of Velocity Files */
    FILE_MODE PropFileMode;

    /** origin, dimension and resolution of the velocity model in il/xl coordinates */
    int n_inlines_Vel, n_xlines_Vel;
    float d_between_inlines_Vel, d_between_xlines_Vel;
    int first_inline_num_Vel, first_xline_num_Vel;

    /** origin, dimension and resolution of the velocity model in model coordinates */
    point3D<float> X0Vel, dxVel;
    point3D<int> NVel;


    /**  Name of the directory for travel time tables */
    char TTDirName[199];
    /**  Name Prefix for travel time tables */
    char TTFilePrefix[199];

    /** origin, dimension and resolution of the shots in il/xl coordinates*/
    int n_inlines_Src, n_xlines_Src;
    float d_between_inlines_Src, d_between_xlines_Src;
    int first_inline_num_Src, first_xline_num_Src;

    /** origin, dimension and resolution of the shots in model coordinates */
    point3D<float> X0Src, dxSrc;
    point3D<int> NtotSrc;
    /** starting indicees and number of shots for this job */
    point3D<int> N0Src;
    point3D<int> NSrc;
  
    /** flag for given source elevation file */
    bool SrcElev;
    /** Name of source elevation file */
    char SrcElevFileName[199];


    /** origin, dimension and resolution of the receivers in il/xl coordinates*/
    int n_inlines_Rcv, n_xlines_Rcv;
    float d_between_inlines_Rcv, d_between_xlines_Rcv;
    int first_inline_num_Rcv, first_xline_num_Rcv;

    /** origin, dimension and resolution of the receivers */
    point3D<float> X0Rcv, dxRcv;
    point3D<int> NRcv;
    
    /** Maximum aperture angle for the take-off direction of the rays (in deg).
        Angle is given realitiv to a algorith dependent normal, range 0..180 */
    DEG RayAperture_deg;

    /** Offset resp. to the aperture angle */
    point3D<float> Offs;

    /** Initial angular spreading of rays per source */
    DEG g_InitAngle;

    /** critical length for refinement (WAVEFRONTTRACER)*/
    int g_REF_LEN;
    /** Angle resolution (RAYTRACER)*/
    DEG g_DANGLE;

    /** Flag for Restart mode */
    bool g_restart;

    /** Max number of time steps */
    int g_MAXTSTEP;
    /** Time step size */
    float g_TSTEPSIZE;

    /** Flag for doing no ray tracing but velocity file preparation and coarseing only */
    bool ModelPreprocessingOnly;
    /** Flag for velocity file preparation, smoothing and coarsening */
    bool PreprocessingVel;
    /** Flag for anisotropy parameter file preparation, smoothing and coarsening */
    bool PreprocessingAni;

    /** Half width of the smoothing operators in physical units */
    float SmoothingLengthVel;
    float SmoothingLengthAni;

    /** Factor of coarsening for all Vel/Ani files, direction dependent */
    int CoarseningFactoralongInline;
    int CoarseningFactoralongXline;
    int CoarseningFactoralongDepth;

    /**  Name of smoothed Velocity File */
    char SmoothedVelFileName[199];
    /**  Name of smoothed VTI Velocity Files */
    char SmoothedVTI_EpsilonFileName[199];
    char SmoothedVTI_DeltaFileName[199];
    /**  Name of smoothed VTI Velocity Files */
    char SmoothedTTI_AlphaFileName[199];
    char SmoothedTTI_BetaFileName[199];

// private attributes
 private:

};
#endif
