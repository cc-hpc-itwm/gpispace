#include <we/loader/macros.hpp>
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

// ************************************************************************* //

static unsigned long sizeofBunchBuffer (const MigrationJob & Job)
{
  return getSizeofTD (Job);
}

static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

// ************************************************************************* //

static void get_Job (const value::type & config, MigrationJob & Job)
{
  const fvmAllocHandle_t handle_Job
    (value::get_literal_value<long> (value::get_field ("handle_Job", config)));
  const fvmAllocHandle_t scratch_Job
    (value::get_literal_value<long> (value::get_field ("scratch_Job", config)));

  waitComm (fvmGetGlobalData ( handle_Job
                             , fvmGetRank() * sizeofJob()
                             , sizeofJob()
                             , 0
                             , scratch_Job
                             )
           );

  memcpy (&Job, fvmGetShmemPtr(), sizeofJob());
}

/* ************************************************************************* */

static void initialize ( void *
                       , const we::loader::input_t & input
                       , we::loader::output_t & output
                       )
{
  const std::string & filename
    (we::loader::get_input<std::string> (input, "config_file"));

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

  LOG (INFO, "sizeofBunchBuffer =  " << sizeofBunchBuffer(Job));
  LOG (INFO, "sizeofJob =  " << sizeof(Job));

  // WORK HERE:
  Job.ReqVMMemSize = Job.globTTbufsizelocal + 2 * sizeofJob();
  Job.BunchMemSize = getSizeofTD(Job);

  // Check whether the travel time table covers the entire output volume

  // Touch the offset gather
  MigrationFileHandler MFHandler;
  MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                             Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);


  Job.shift_for_TT = sizeofJob() + sizeofBunchBuffer(Job) + Job.SubVolMemSize;
  Job.shift_for_Vol = sizeofJob() + sizeofBunchBuffer(Job);

  LOG (INFO, "shift_for_TT = " << Job.shift_for_TT);
  LOG (INFO, "shift_for_Vol = " << Job.shift_for_Vol);

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

  LOG (DEBUG, "handle_TT " << Job.globTTbufsizelocal);

  value::structured_t config;

  config["handle_Job"] = static_cast<long>(handle_Job);
  config["handle_TT"] = static_cast<long>(handle_TT);
  config["NThreads"] = static_cast<long>(NThreads);

  config["OFFSETS"] = static_cast<long>(Job.n_offset);
  config["SUBVOLUMES_PER_OFFSET"] = static_cast<long>(Job.NSubVols);
  config["BUNCHES_PER_OFFSET"] = static_cast<long>(Nbid_in_pid (1, 1, Job));
  config["PARALLEL_LOADTT"] = static_cast<long>(fvmGetNodeCount());
  config["STORES"] = static_cast<long>(2 * fvmGetNodeCount());

  const long wait 
    (value::get_literal_value<long> 
     (value::get_field ("SUBVOLUMES_PER_OFFSET", config))
    )
    ;

  we::loader::put_output (output, "config", config);
  we::loader::put_output (output, "wait", literal::type(wait));
  we::loader::put_output (output, "trigger", control());

  bitsetofint::type bs; bs.ins (0);

  we::loader::put_output (output, "wanted", bs);

  const long parallel_loadTT
    (value::get_literal_value<long> 
     (value::get_field ("PARALLEL_LOADTT", config))
    )
    ;

  we::loader::put_output (output, "parallel_loadTT", parallel_loadTT);
}

/* ************************************************************************* */

static void loadTT ( void *
                   , const we::loader::input_t &  input
                   , we::loader::output_t & output
                   )
{
  const value::type & config (input.value("config"));
  const long & TT (we::loader::get_input<long> (input, "TT"));

  LOG (INFO, "loadTT: got config " << config);

  const long & Parallel_loadTT
    (value::get_literal_value<long> 
     (value::get_field ("PARALLEL_LOADTT", config))
    );

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
  const int NThreads
    (value::get_literal_value<long> (value::get_field ("NThreads", config)));
  const fvmAllocHandle_t handle_TT
    (value::get_literal_value<long> (value::get_field ("handle_TT", config)));

  TTVMMemHandler TTVMMem;

  TTVMMem.InitVol(Job,Job.RTFileName,GSrc,GVol,NThreads,0, TT, Parallel_loadTT, handle_TT);

  we::loader::put_output (output, "TT", TT);
}

/* ************************************************************************* */

static void load ( void *
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const value::type & config (input.value("config"));
  const value::type & bunch (input.value("bunch"));
  const long & empty_store (we::loader::get_input<long> (input, "empty_store"));

  std::cout << "load: got config " << config << std::endl;
  std::cout << "load: got bunch " << bunch << std::endl;
  std::cout << "load: got empty store " << empty_store << std::endl;

  value::structured_t loaded_bunch;

  loaded_bunch["bunch"] = bunch;
  loaded_bunch["store"] = empty_store;
  loaded_bunch["seen"] = bitsetofint::type();
  loaded_bunch["wait"] = value::get_literal_value<long> 
    (value::get_field ("SUBVOLUMES_PER_OFFSET", config));

  we::loader::put_output (output, "loaded_bunch", loaded_bunch);

  std::cout << "load: loaded_bunch " << loaded_bunch << std::endl;
}

/* ************************************************************************* */

static void process ( void *
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));

  std::cout << "process: got config " << config << std::endl;
  std::cout << "process: got volume " << volume << std::endl;

  value::type volume_processed (volume);

  const value::type buffer0 (value::get_field ("buffer0", volume));
  const bool assigned0
    (value::get_literal_value<bool>(value::get_field ("assigned", buffer0)));
  const bool filled0
    (value::get_literal_value<bool>(value::get_field ("filled", buffer0)));

  const value::type buffer1 (value::get_field ("buffer1", volume));
  const bool assigned1
    (value::get_literal_value<bool>(value::get_field ("assigned", buffer1)));
  const bool filled1
    (value::get_literal_value<bool>(value::get_field ("filled", buffer1)));

  // die hier implementierte Logik ist noch nicht optimal: es wird
  // einfach nur jeder bufffer geladen, der assigned aber nicht
  // gefüllt ist und die buffer, die geladen sind, werden verarbeitet.

  const long wait
    (value::get_literal_value<long>(value::get_field ("wait", volume)));

  value::field("assigned", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("filled", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("free", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("assigned", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("filled", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("free", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("wait", volume_processed) = wait - ((filled0) ? 1 : 0) - ((filled1) ? 1 : 0);

  we::loader::put_output (output, "volume_processed", volume_processed);

  std::cout << "process: volume_processed " << volume_processed << std::endl;
}

/* ************************************************************************* */

static void write ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));

  std::cout << "write: got config " << config << std::endl;
  std::cout << "write: got volume " << volume << std::endl;

  we::loader::put_output (output, "volume", volume);
}

/* ************************************************************************* */

static void finalize ( void *
                     , const we::loader::input_t &  input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (input.value("config"));

  LOG (INFO, "finalize: got config " << config);

  const fvmAllocHandle_t handle_Job
    (value::get_literal_value<long> (value::get_field ("handle_Job", config)));
  const fvmAllocHandle_t handle_TT
    (value::get_literal_value<long> (value::get_field ("handle_TT", config)));

  fvmGlobalFree (handle_Job);
  fvmGlobalFree (handle_TT);

  we::loader::put_output (output, "done", control());
}

/* ************************************************************************* */

WE_MOD_INITIALIZE_START (kdm_complex);
{
  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdm_complex);

WE_MOD_FINALIZE_START (kdm_complex);
{
}
WE_MOD_FINALIZE_END (kdm_complex);
