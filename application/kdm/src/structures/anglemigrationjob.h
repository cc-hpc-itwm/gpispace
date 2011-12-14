/***************************************************************************
                          anglemigrationjob.h  -  description

    Complete description of a job for angle migration. 

                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2010-01-11
               AperAng_deg renamed to RayAperture_deg               
               RayFan_TaperWidth_deg renamed to RayApertureTaperWidth_deg
               AngleAperture renamed to CDPAperture_deg               
               MinAperture renamed to CDPApertureMin_mtr               
               TaperWidthAperture renamed to CDPApertureTaperWidth_mtry               

      merten | 2010-01-06
               aperture angle changed to float and renamed to AperAng_deg

      merten | 2009-08-03
               Adapted to modified seisgrid3D class internal structure
 ***************************************************************************/


#ifndef ANGLEMIGRATIONJOB_H
#define ANGLEMIGRATIONJOB_H


/**
  *@author Dirk Merten
  */

#include "include/migtypes.h"
#include "structures/optionalprm.h"
#include "structures/geomdefstruct.hpp"
#include "structures/point3d.h"
#include "structures/seisgrid3d.h"
#include "structures/deg.h"

#include <string.h>

class AngleMigrationJob {

// public methods
public: 
    AngleMigrationJob();
    AngleMigrationJob(const AngleMigrationJob& Job);
    ~AngleMigrationJob();
    AngleMigrationJob& operator = (const AngleMigrationJob& Job);

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

    /** Mode for job distribution */
    bool BlockMode;

    /** Number of Block at a time */
    OptionalPrm<int> JobSize;
    
    /** origin, dimension and resolution of the migration volume in model coordinates */
    seisgrid3D MigVol;

    /**  Ray Fan resolutions */
    DEG DANG_RAYFAN_COARSE;
    DEG DANG_RAYFAN_FINE;
    DEG DANG_RAYFAN_FILE;
    DEG DANG_RAYFAN_DIP;
  
    /** Conditioning of angle integration flag */
    bool conditionAngleBins;

    /** Maximum aperture angle for the take-off direction of the rays (in deg).
        Angle is given realitiv to the vertical, range 0..180 */
    DEG RayAperture_deg;
    /**  Width of the taper for RayAperture_deg (in deg). 
         Used for smoothing the RayFan internal edges */
    DEG RayApertureTaperWidth_deg;

    /** Maximum aperture angle (in deg). Traces CMP seen from a subsurface point under an angle larger
        than this are neglected.
	This angle is given realitiv to the vertical, range 0..90 */
    DEG CDPAperture_deg;
    /** Minimum spatial distance between trace CMP and subsurface coordinates (x,y) for which a trace
	outside of the CDPAperture_deg is neglected. */
    float CDPApertureMin_mtr;
    /**  Width of the taper for CDPAperture_deg.
         Taper is given and applied in the spatial distance of the corresponding surface coordinates. */
    float CDPApertureTaperWidth_mtr;

    /** Opening Angle range */
    DEG phi_open_start;
    DEG phi_open_end;
    DEG dphi_open;
    int N_PHI_OPEN;

    /** Azimuth Angle range */
    DEG dazimuth_deg; // = 180 / N_AZIMUTH_CLASSES
    int N_AZIMUTH_CLASSES;

    /** Dip Angle range */
    DEG theta_dip_start;
    DEG theta_dip_end;
    DEG diprange_deg;

    /** Taper width for dip angle */
    DEG g_DipTaperWidth;

    /** Flag and widths for CDP taper in model x and y */
    bool cdp_taper;
    float cdpx_taperwidth;
    float cdpy_taperwidth;

    /** Offset taper definition */
    bool offset_innermute;
    float offset_innermute_offset;
    bool offset_outermute;
    bool offset_outermute_linear;
    float offset_outermutetaperwidth;
    float offset_outermute_offset1;
    float offset_outermute_t1;
    float offset_outermute_offset2;
    float offset_outermute_t2;


    /** Acceptance distance for Sources and Receiver positions */
    float dSrcxmax, dSrcymax, dRcvxmax, dRcvymax;

    /** Spatial resolution of Rays at the surface */
    float dRayxmax, dRayymax;

    /** Spatial average distance of traces; used for anti-aliasing */
    float averageTraceDistance;

    /** Minimal traveltime difference for neighboring rays to be accepted */
    float TTMinDiff;

    /** Max number of rays per surface bin */
    int MAXRAY;

    /** Parallel rays with termination distance below this value are treated as plane wave front  */
    float Pointsplit_distance;

    /** Number of trace files */
    int NTraceFiles;
    /** List of trace file names */
    char** TraceFileName;

    /** Directory of the migration output */
    char MigDirName[199];
    /** Name of the migration output file */
    char MigFileName[199];
    /** Name of the migration header file 1 */
    char Header1FileName[199];
    /** Name of the migration header file 2 */
    char Header2FileName[199];
    /** Name of the migration QC file */
    char QCFileName[199];
    /** Name of the migration restart file */
    char RestartFileName[199];
    /** Name of the vector migration M1 output file */
    char VecMigM1FileName[199];
    /** Name of the vector migration M2 output file */
    char VecMigM2FileName[199];
    /** Name of the vector migration M3 output file */
    char VecMigM3FileName[199];
    /** Name of the vector migration quality output file */
    char VecMigQualityFileName[199];

    /** Directory of the travel time tables */
    char TTDirName[199];
    /** Name of the travel time table master file */
    char TTCfgFileName[199];
    /** Name prefix of the travel time tables */
    char TTFilePrefix[199];

    /** origin, dimension and resolution of the TT table volume in model coordinates */
    seisgrid3D TTVol;

    /** Format of trace data files */
    FILE_MODE TraceFileMode;
    /** Format of migration output file */
    FILE_MODE MigFileMode;

    /** Strategy for parallel IO */
    IO_MODE IOMode;

    /** Time shift of Trace data */
    OptionalPrm<float> g_T0;
    /** Minimal time of interest of the trace data */
    OptionalPrm<float> g_Tmin;
    /** Maximal time of interest of the trace data */
    OptionalPrm<float> g_Tmax;

    /** Length of taper zone in seconds against end of trace  */
    float tEndTaperLength;

    /** Band pass Frequencies */
    float frequ1, frequ2, frequ3, frequ4;

    /** Flag for Restart mode */
    bool g_restart;

    /** Switch for true amplitude weighting factor; 1 : scattering coefficient, 2 : reflection coefficient **/ 
    int trueAmplitudeModus;

    /** Flag for kin. Weights, i.e. detQ == 1 (true) or full dynamic weigthing with detQ (false, default) **/ 
    bool kinematicWeightsOnly;

    /** Switch for regularization of the dip integral following Ursin **/ 
    bool UrsinRegularization;

    /** Flag for DipGather mode */
    bool g_Dip_Gather;
    /** Number of Dip Gathers */
    int N_GATHER;

    /** Flag for  Upwards-going-rays-only mode */
    bool g_No_Reflection_and_Headwave;

    /** Clip value */
    float clip;

    /** Flag for derivative of Input Data */
    bool frac;

    /** tpow gaining of input data */
    bool applytpow;
    float tpow;

    /** Flag for Angle Migration */
    bool AngleMigration;
    /** Flag for Vector Migration */
    bool VectorMigration;
    /** Flag for performing ECED smoothing of Vector Migration results*/
    bool VectorMigrationECED;

    /** Flag for applying Dip Cube information */
    bool DipCube;
    /** Flag for applying Dip Cube confidence information */
    bool DipCubeConfidence;
    /** Inline Dip Cube file name */
    char InlineFileName[199];
    /** Xline Dip Cube file name */
    char XlineFileName[199];
    /** Xline Dip Cube file name */
    char ConfidenceFileName[199];
    /** Format of Dip Cube files */
    FILE_MODE PropFileMode;

    /** origin, dimension and resolution of the dip cubes in model coordinates */
    seisgrid3D DCVol;

// private methods
 private:

// private attributes
 private:

};

#endif
