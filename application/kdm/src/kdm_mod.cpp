#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <we/type/value/show.hpp>
#include <we/type/value/peek.hpp>

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

namespace
{
  template<typename R>
    R peek (const std::string& key, const pnet::type::value::value_type& x)
  {
    return boost::get<R> (*pnet::type::value::peek (key, x));
  }
}

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

static void initialize (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const std::string& filename
    (boost::get<const std::string&> (input.value ("config_file")));
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

  LOG (DEBUG, "initialize: config = "
           << pnet::type::value::show (output.value ("config"))
      );
}

// ************************************************************************* //

static void get_Job ( const pnet::type::value::value_type& config
                    , MigrationJob & Job
                    )
{
  const fvmAllocHandle_t handle_Job (peek<long> ("handle_Job", config));
  const fvmAllocHandle_t scratch_Job (peek<long> ("scratch_Job", config));

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

static void kdm_loadTT ( const pnet::type::value::value_type& config
                       , const long & TT
                       )
{
  LOG (INFO, "loadTT: got config " << pnet::type::value::show (config));

  const long& Parallel_loadTT (peek<const long&> ("PARALLEL_LOADTT", config));

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
  const int NThreads (peek<long> ("NThreads", config));
  const fvmAllocHandle_t handle_TT (peek<long> ("handle_TT", config));

  TTVMMemHandler TTVMMem;

  TTVMMem.InitVol(Job,Job.RTFileName,GSrc,GVol,NThreads,0, TT, Parallel_loadTT, handle_TT);
}

// ************************************************************************* //

static void kdm_finalize (const pnet::type::value::value_type& config)
{
  LOG (INFO, "finalize: got config " << pnet::type::value::show (config));

  fvmGlobalFree (peek<long> ("handle_Job", config));
  fvmGlobalFree (peek<long> ("scratch_Job", config));
  fvmGlobalFree (peek<long> ("handle_TT", config));
}

// ************************************************************************* //

static void kdm_load ( const pnet::type::value::value_type& config
                     , const pnet::type::value::value_type& bunch
                     )
{
  LOG (INFO, "load: got bunch " << pnet::type::value::show (bunch));

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + peek<long> ("volume.offset", bunch));
  const long bid (1 + peek<long> ("id", bunch));

  char * pBunchData (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(pBunchData,oid,1,bid,Job);

  Bunch.LoadFromDisk_CO_MT(Job);

  LOG (INFO, "load: bunch loaded");
}

// ************************************************************************* //

static void kdm_write ( const pnet::type::value::value_type& config
                      , const pnet::type::value::value_type& volume
                      )
{
  LOG (INFO, "write: got volume " << pnet::type::value::show (volume));

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
  const long vid (1 + peek<long> ("id", volume));
  const long oid (1 + peek<long> ("offset", volume));

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

  LOG (INFO, "write: volume written " << pnet::type::value::show (volume));
}

// ************************************************************************* //

static void kdm_init_volume ( const pnet::type::value::value_type& config
                            , const pnet::type::value::value_type& volume
                            )
{
  LOG (INFO, "init_volume: got volume " << pnet::type::value::show (volume));

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
  const long vid (1 + peek<long> ("id", volume));

  MigSubVol3D MigSubVol(MigVol, vid, Job.NSubVols);

  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // now, initialize the subvol
  MigSubVol.clear();
}

// ************************************************************************* //

static void kdm_process ( const pnet::type::value::value_type & config
                        , const pnet::type::value::value_type & bunch
                        )
{
  LOG (INFO, "process: got bunch " << pnet::type::value::show (bunch));

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
  const long vid (1 + peek<long> ("volume.id", bunch));

  MigSubVol3D MigSubVol(MigVol,vid,Job.NSubVols);

  // Attach the mem ptr to the subvolume
  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // Reconstruct the tracebunch out of memory
  const long oid (1 + peek<long> ("volume.offset", bunch));
  const long bid (1 + peek<long> ("id", bunch));

  char * migbuf (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(migbuf,oid,1,bid,Job);

  // migrate the bunch to the subvolume
  const long & NThreads (peek<const long &> ("NThreads", config));

  char * _VMem  (((char *)fvmGetShmemPtr()) + Job.shift_for_TT);

  const fvmAllocHandle_t handle_TT (peek<long> ("handle_TT", config));

  MigBunch2SubVol(Job,Bunch,MigSubVol,NThreads, _VMem, handle_TT);

  LOG (INFO, "process: bunch done " << pnet::type::value::show (bunch));
}

// ************************************************************************* //
// wrapper functions

static void loadTT (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  kdm_loadTT (input.value ("config"), boost::get<long> (input.value ("id")));

  output.bind ("done", we::type::literal::control());
}

static void load (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& bunch (input.value ("bunch"));

  kdm_load (input.value ("config"), bunch);

  output.bind ("bunch", bunch);
}

static void process (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& bunch (input.value ("bunch"));

  kdm_process (input.value ("config"), bunch);

  output.bind ("bunch", bunch);
}

static void write (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& config (input.value ("config"));
  const pnet::type::value::value_type& volume (input.value ("volume"));

  kdm_write (config, volume);

  output.bind ("volume", volume);
}

static void finalize (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  kdm_finalize (input.value ("config"));

  output.bind ("trigger", we::type::literal::control());
}

static void init_volume (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& volume (input.value ("volume"));

  kdm_init_volume (input.value ("config"), volume);

  output.bind ("volume", volume);
}

static void debug (gspc::drts::context *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& volume (input.value ("volume"));

  LOG (INFO, "DEBUG: volume " << pnet::type::value::show (volume));

  output.bind ("volume", volume);
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
