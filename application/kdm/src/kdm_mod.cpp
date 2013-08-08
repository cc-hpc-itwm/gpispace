#include <we/loader/macros.hpp>
#include <putget.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

// KDM include files
#include "structures/migrationjob.h"
#include "filehandler/checkreadmigrationjob.h"
#include "filehandler/migrationfilehandler.h"
#include "TraceBunch.hpp"
#include "MigSubVol.hpp"
#include "sdpa_migrate.hpp"

#include "ttvmmemhandler.h"

#include "sinc_mod.hpp"

#include <we2/type/compat.hpp>

using we::loader::get;

// ************************************************************************* //

static unsigned long sizeofBunchBuffer (const MigrationJob & Job)
{
//   return ( NTrace_in_bid (1, 1, 1, Job)
//          * ( 5 * Job.traceNt * sizeof(float)
//            + sizeof(int) + 7 * sizeof (float)
//            )
//          );
  return getSizeofTD (Job);
}

static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "config_file"));
  const int NThreads (4);

  MigrationJob Job;
  CheckReadMigrationJob JobReader;
  std::string CfgFileName (filename);

  LOG (INFO, "filename = " << filename);

  if (JobReader.ReadConfigFileXML((char*)(CfgFileName.c_str()), Job) != 0)
  {
    throw std::runtime_error("KDM::initialize JobReader.ReadConfigFileXML(CfgFileName, Job) != 0");
  }

  char JobFile[2*199 + 16];
  sprintf(JobFile, "%s/%s_mig.xml", Job.MigDirName, Job.JobName);

  if (JobReader.WriteConfigFileXML(JobFile, Job) == -1)
  {
    throw std::runtime_error("KDM::initialize JobReader.WriteConfigFileXML(JobFile, Job) == -1");
  }

  // Check whether the output volume is totally covered by the
  // travel time tables
  // if not, leave the program !
  {
    int ierr;
    BlockVolume BoxVolume(Job.MigVol,Job.TTVol,ierr);
    if(ierr==-1)
    {
      throw std::runtime_error ("KDM::initialize ierr==-1");
    }
  }

  Job.locbufsize=getSizeofTD(Job); // local buffer size in bytes

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
    const int NodeCount (fvmGetNodeCount());

    Job.globTTbufsizelocal
      = TTVMMem.GetMem (Job.RTFileName,GSrc,GVol,NodeCount,NThreads)
      / NodeCount;
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

    LOG (INFO, "SubVolMemSize = " << Job.SubVolMemSize);
  }

  LOG (DEBUG, "sizeofBunchBuffer =  " << sizeofBunchBuffer(Job));
  LOG (DEBUG, "sizeofJob =  " << sizeof(Job));

  //WORK HERE: add sizeof scratch space here
  Job.ReqVMMemSize = Job.globTTbufsizelocal + 2 * sizeofJob();

  Job.BunchMemSize = getSizeofTD(Job);

  // Check whether the travel time table covers the entire output volume

  // Touch the offset gather
  MigrationFileHandler MFHandler;
  MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                             Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);


  Job.shift_for_TT = sizeofJob() + sizeofBunchBuffer(Job) + Job.SubVolMemSize;
  Job.shift_for_Vol = sizeofJob() + sizeofBunchBuffer(Job);

  LOG (DEBUG, "shift_for_TT = " << Job.shift_for_TT);
  LOG (DEBUG, "shift_for_Vol = " << Job.shift_for_Vol);

  const fvmAllocHandle_t handle_Job (fvmGlobalAlloc (sizeofJob()));
  if (handle_Job == 0)
    throw std::runtime_error ("KDM::initialize handle_Job == 0");

  const fvmAllocHandle_t scratch_Job (fvmGlobalAlloc (sizeofJob()));
  if (scratch_Job == 0)
    throw std::runtime_error ("KDM::initialize scratch_Job == 0");

  memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

  for (int p (0); p < fvmGetNodeCount(); ++p)
    waitComm (fvmPutGlobalData (handle_Job, p * sizeofJob(), sizeofJob(), 0, scratch_Job));

  const fvmAllocHandle_t handle_TT (fvmGlobalAlloc (Job.globTTbufsizelocal));
  if (handle_TT == 0)
    throw std::runtime_error ("KDM::initialize handle_TT == 0");

  LOG(INFO, "Job.globTTbufsizelocal = " << Job.globTTbufsizelocal);

  LOG (DEBUG, "handle_TT " << handle_TT);

  output.bind ("config.handle_Job", static_cast<long>(handle_Job));
  output.bind ("config.scratch_Job", static_cast<long>(scratch_Job));
  output.bind ("config.handle_TT", static_cast<long>(handle_TT));
  output.bind ("config.NThreads", static_cast<long>(NThreads));

  output.bind ("config.OFFSETS", static_cast<long>(Job.n_offset));
  output.bind ("config.SUBVOLUMES_PER_OFFSET", static_cast<long>(Job.NSubVols));
  output.bind ("config.BUNCHES_PER_OFFSET", static_cast<long>(Nbid_in_pid (1, 1, Job)));
  output.bind ("config.PARALLEL_LOADTT", static_cast<long>(fvmGetNodeCount()));

  output.bind ("config.VOLUME_CREDITS", 4 * static_cast<long>(fvmGetNodeCount()));

  output.bind ("config.filter.clip", static_cast<double>(Job.clip));
  output.bind ("config.filter.trap", static_cast<double>(Job.trap));
  output.bind ("config.filter.tpow", static_cast<double>(Job.tpow));

  LOG (DEBUG, "initialize: config = " << get<value::type>(output, "config"));
}

// ************************************************************************* //

static void get_Job (const value::type & config, MigrationJob & Job)
{
  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
  const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));

  waitComm (fvmGetGlobalData ( handle_Job
                             , fvmGetRank() * sizeofJob()
                             , sizeofJob()
                             , 0
                             , scratch_Job
                             )
           );

  memcpy (&Job, fvmGetShmemPtr(), sizeofJob());
}

// ************************************************************************* //

static void kdm_loadTT (const value::type & config, const long & TT)
{
  LOG (INFO, "loadTT: got config " << config);

  const long & Parallel_loadTT (get<long> (config, "PARALLEL_LOADTT"));

  LOG (INFO, "loadTT: got TT " << TT << " out of " << Parallel_loadTT);

  MigrationJob Job;

  get_Job (config, Job);

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
  const int & NThreads (get<long> (config, "NThreads"));
  const fvmAllocHandle_t & handle_TT (get<long> (config, "handle_TT"));

  TTVMMemHandler TTVMMem;

  TTVMMem.InitVol(Job,Job.RTFileName,GSrc,GVol,NThreads,0, TT, Parallel_loadTT, handle_TT);
}

// ************************************************************************* //

static void kdm_finalize (const value::type & config)
{
  LOG (INFO, "finalize: got config " << config);

  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
  const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));
  const fvmAllocHandle_t & handle_TT  (get<long> (config, "handle_TT"));

  fvmGlobalFree (handle_Job);
  fvmGlobalFree (scratch_Job);
  fvmGlobalFree (handle_TT);
}

// ************************************************************************* //

static void kdm_load (const value::type & config, const value::type & bunch)
{
  LOG (INFO, "load: got bunch " << bunch);

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + get<long> (bunch, "volume.offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * pBunchData (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(pBunchData,oid,1,bid,Job);

  Bunch.LoadFromDisk_CO_MT(Job);

  LOG (INFO, "load: bunch loaded");
}

// ************************************************************************* //

static void kdm_write (const value::type & config, const value::type & volume)
{
  LOG (INFO, "write: got volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

  // write offset class for a  given subvolume to disk
  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  // Reconstruct the subvolume out of memory
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

  grid3D G(X0,Nx,dx);

  // create the subvolume
  const long vid (1 + get<long> (volume, "id"));
  const long oid (1 + get<long> (volume, "offset"));

  MigSubVol3D MigSubVol(MigVol,vid,Job.NSubVols);

  MigrationFileHandler MFHandler;

  MFHandler.WriteOffsetClassForSubVol
    ( Job.MigFileName
    , G
    , (float *)pVMMemSubVol
    , MigSubVol.getix0(), MigSubVol.getNx()
    , MigSubVol.getiy0(), MigSubVol.getNy()
    , oid-1
    , 1
    , Job.NtotOffVol
    , Job.MigFileMode
    );

  LOG (INFO, "write: volume written " << volume);
}

// ************************************************************************* //

static void kdm_init_volume ( const value::type & config
                            , const value::type & volume
                            )
{
  LOG (INFO, "init_volume: got volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

//   if (!Job.sinc_initialized)
//   {
//     LOG(INFO, "Init SincInterpolator on node " << fvmGetRank());

//     const long & NThreads (get<long> (config, "NThreads"));

//     initSincIntArray(NThreads, Job.tracedt);

//     Job.sinc_initialized = true;

//     memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

//     // rewrite Job
//     const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
//     const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));

//     waitComm (fvmPutGlobalData ( handle_Job
//                                , fvmGetRank() * sizeofJob()
//                                , sizeofJob()
//                                , 0
//                                , scratch_Job
//                                )
//              );
//   }

  // init subvolume

  LOG(INFO, "Init Subvolume");

  // initialize subvol to 0.

  // Reconstruct the subvolume out of memory
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

  // create the subvolume
  const long vid (1 + get<long> (volume, "id"));

  MigSubVol3D MigSubVol(MigVol, vid, Job.NSubVols);

  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // now, initialize the subvol
  MigSubVol.clear();
}

// ************************************************************************* //

static void kdm_process ( const value::type & config
                        , const value::type & bunch
                        )
{
  LOG (INFO, "process: got bunch " << bunch);

  MigrationJob Job;

  get_Job (config, Job);

  // the migrate part of migrate_and_prefetch only

  // Reconstruct the subvolume out of memory
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

  // create the subvolume
  const long vid (1 + get<long> (bunch, "volume.id"));

  MigSubVol3D MigSubVol(MigVol,vid,Job.NSubVols);

  // Attach the mem ptr to the subvolume
  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // Reconstruct the tracebunch out of memory
  const long oid (1 + get<long> (bunch, "volume.offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * migbuf (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(migbuf,oid,1,bid,Job);

  // migrate the bunch to the subvolume
  const long & NThreads (get<long> (config, "NThreads"));

  char * _VMem  (((char *)fvmGetShmemPtr()) + Job.shift_for_TT);

  const fvmAllocHandle_t & handle_TT (get<long> (config, "handle_TT"));

  MigBunch2SubVol(Job,Bunch,MigSubVol,NThreads, _VMem, handle_TT);

  LOG (INFO, "process: bunch done " << bunch);
}

// ************************************************************************* //
// wrapper functions

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const long & TT (get<long> (input, "id"));

  kdm_loadTT (config, TT);

  output.bind ("done", we::type::literal::control());
}

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));
  kdm_load (config, bunch);
  output.bind ("bunch", pnet::type::compat::COMPAT (bunch));
}

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  kdm_process (config, bunch);

  output.bind ("bunch", pnet::type::compat::COMPAT (bunch));
}

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  kdm_write (config, volume);

  output.bind ("volume", pnet::type::compat::COMPAT (volume));
}

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  kdm_finalize (config);

  output.bind ("trigger", we::type::literal::control());
}

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  kdm_init_volume (config, volume);

  output.bind ("volume", pnet::type::compat::COMPAT (volume));
}

static void debug (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  LOG (INFO, "DEBUG: volume " << get<value::type>(input, "volume"));

  output.bind ("volume", pnet::type::compat::COMPAT (get<value::type>(input, "volume")));
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdm);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdm)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (init_volume);
  WE_REGISTER_FUN (finalize);
  WE_REGISTER_FUN (debug);
}
WE_MOD_INITIALIZE_END (kdm);

WE_MOD_FINALIZE_START (kdm);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdm)");
}
WE_MOD_FINALIZE_END (kdm);
