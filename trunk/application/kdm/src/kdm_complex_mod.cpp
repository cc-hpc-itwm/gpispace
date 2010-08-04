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

using we::loader::get;

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

/* ************************************************************************* */

static void initialize ( void *
                       , const we::loader::input_t & input
                       , we::loader::output_t & output
                       )
{
  const std::string & filename (get<std::string> (input, "config_file"));

  const int NThreads (4);
  const unsigned long STORES_PER_NODE (2);

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

  if (Job.NSubVols != fvmGetNodeCount())
    {
      throw std::runtime_error ("Job.NSubVols != fvmGetNodeCount()");
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

  const unsigned long SizeVolWithBuffer
    (Job.SubVolMemSize + 2 * sizeofBunchBuffer (Job));

  Job.ReqVMMemSize = 2 * sizeofJob()
                   + Job.globTTbufsizelocal
                   + (STORES_PER_NODE + 1) * sizeofBunchBuffer (Job)
                   + 2 * SizeVolWithBuffer
    ;
  Job.BunchMemSize = getSizeofTD(Job);

  Job.shift_for_Vol = sizeofJob();
  Job.shift_for_TT = Job.shift_for_TT + SizeVolWithBuffer;

  LOG (INFO, "ReqVMMemSize =  " << Job.ReqVMMemSize
      << " (" <<  (Job.ReqVMMemSize>>20) << " MiB)"
      );

  // Check whether the travel time table covers the entire output volume

  // Touch the offset gather
  MigrationFileHandler MFHandler;
  MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                             Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);


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

  const fvmAllocHandle_t handle_Store
    (fvmGlobalAlloc (STORES_PER_NODE * sizeofBunchBuffer (Job)));
  if (handle_Store == 0)
    throw std::runtime_error ("KDM::initialize handle_Store == 0");

  const fvmAllocHandle_t scratch_Store
    (fvmGlobalAlloc (sizeofBunchBuffer (Job)));
  if (handle_Store == 0)
    throw std::runtime_error ("KDM::initialize scratch_Store == 0");

  const fvmAllocHandle_t handle_Volume (fvmGlobalAlloc (SizeVolWithBuffer));
  if (handle_Volume == 0)
    throw std::runtime_error ("KDM::initialize handle_Volume == 0");

  const fvmAllocHandle_t scratch_Volume (fvmGlobalAlloc (SizeVolWithBuffer));
  if (scratch_Volume == 0)
    throw std::runtime_error ("KDM::initialize scratch_Volume == 0");

  value::structured_t config;

  config["handle_TT"] = static_cast<long>(handle_TT);
  config["handle_Job"] = static_cast<long>(handle_Job);
  config["scratch_Job"] = static_cast<long>(scratch_Job);
  config["handle_Store"] = static_cast<long>(handle_Store);
  config["scratch_Store"] = static_cast<long>(scratch_Store);
  config["handle_Volume"] = static_cast<long>(handle_Volume);
  config["scratch_Volume"] = static_cast<long>(scratch_Volume);
  config["NThreads"] = static_cast<long>(NThreads);

  config["OFFSETS"] = static_cast<long>(Job.n_offset);
  config["SUBVOLUMES_PER_OFFSET"] = static_cast<long>(Job.NSubVols);
  config["BUNCHES_PER_OFFSET"] = static_cast<long>(Nbid_in_pid (1, 1, Job));
  config["PARALLEL_LOADTT"] = static_cast<long>(fvmGetNodeCount());
  config["STORES"] = static_cast<long>(STORES_PER_NODE * fvmGetNodeCount());

  const long & wait (get<long> (config, "SUBVOLUMES_PER_OFFSET"));

  we::loader::put_output (output, "config", config);
  we::loader::put_output (output, "wait", literal::type(wait));
  we::loader::put_output (output, "trigger", control());

  bitsetofint::type bs; bs.ins (0);

  we::loader::put_output (output, "wanted", bs);

  const long & parallel_loadTT (get<long> (config, "PARALLEL_LOADTT"));

  we::loader::put_output (output, "parallel_loadTT", parallel_loadTT);
}

/* ************************************************************************* */

static void loadTT ( void *
                   , const we::loader::input_t &  input
                   , we::loader::output_t & output
                   )
{
  const value::type & config (get<value::type> (input, "config"));
  const long & TT (get<long> (input, "TT"));

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

  we::loader::put_output (output, "TT", TT);
}

/* ************************************************************************* */

static void load ( void *
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));
  const long & empty_store (get<long> (input, "empty_store"));

  MigrationJob Job;

  get_Job (config, Job);

  const long oid (1 + get<long>(bunch, "offset"));
  const long bid (1 + get<long> (bunch, "id"));

  char * pBunchData (((char *)fvmGetShmemPtr()) + sizeofJob());

  TraceBunch Bunch(pBunchData,oid,1,bid,Job);

  Bunch.LoadFromDisk_CO_MT(Job);

  LOG (INFO, "load: loaded bunch " << bunch);

  const fvmAllocHandle_t & handle_Store (get<long> (config, "handle_Store"));
  const fvmAllocHandle_t & scratch_Store (get<long> (config, "scratch_Store"));

  waitComm ( fvmPutGlobalData
             ( handle_Store
             , empty_store * sizeofBunchBuffer (Job)
             , sizeofBunchBuffer (Job)
             , sizeofJob()
             , scratch_Store
             )
           );

  value::structured_t loaded_bunch;

  loaded_bunch["bunch"] = bunch;
  loaded_bunch["store"] = empty_store;
  loaded_bunch["seen"] = bitsetofint::type();
  loaded_bunch["wait"] = get<long> (config, "SUBVOLUMES_PER_OFFSET");

  LOG (INFO, "load: stored bunch " << loaded_bunch);

  we::loader::put_output (output, "loaded_bunch", loaded_bunch);
}

/* ************************************************************************* */

static void process ( void *
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  LOG (INFO, "process: got config " << config);
  LOG (INFO, "process: got volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

  if (!Job.sinc_initialized)
  {
    LOG(INFO, "Init SincInterpolator on node " << fvmGetRank());

    const long & NThreads (get<long> (config, "NThreads"));

    initSincIntArray(NThreads, Job.tracedt);

    Job.sinc_initialized = true;

    memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

    // rewrite Job
    const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
    const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));

    waitComm (fvmPutGlobalData ( handle_Job
                               , fvmGetRank() * sizeofJob()
                               , sizeofJob()
                               , 0
                               , scratch_Job
                               )
             );
  }

  // the migrate part of migrate_and_prefetch
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
  const long vid (1 + get<long> (volume, "volume.id"));

  MigSubVol3D MigSubVol(MigVol,vid,Job.NSubVols);

  const fvmAllocHandle_t & handle_Volume (get<long> (config, "handle_Volume"));
  const fvmAllocHandle_t & scratch_Volume (get<long> (config, "scratch_Volume"));

  const unsigned long SizeVolWithBuffer
    (Job.SubVolMemSize + 2 * sizeofBunchBuffer (Job));

  waitComm ( fvmGetGlobalData
             ( handle_Volume
             , vid * SizeVolWithBuffer
             , SizeVolWithBuffer
             , Job.shift_for_Vol
             , scratch_Volume
             )
           );

  // Attach the mem ptr to the subvolume
  char * pVMMemSubVol (((char *)fvmGetShmemPtr()) + Job.shift_for_Vol);

  MigSubVol.setMemPtr((float *)pVMMemSubVol, Job.SubVolMemSize);

  value::type volume_processed (volume);

  const bool & assigned0 (get<bool>(volume, "buffer0.assigned"));
  const bool & filled0 (get<bool>(volume, "buffer0.filled"));
  const bool & assigned1 (get<bool>(volume, "buffer1.assigned"));
  const bool & filled1 (get<bool>(volume, "buffer1.filled"));

  const int buf_to_prefetch
    ((assigned0 && !filled0) ? 0 : ((assigned1 && !filled1) ? 1 : (-1)));
  const int buf_to_migrate (filled0 ? 0 : (filled1 ? 1 : (-1)));

  fvmCommHandle_t comm_handle;

  if (buf_to_prefetch >= 0)
    {
      // start prefetch
      const std::string buf ((buf_to_prefetch == 0) ? "buffer0" : "buffer1");

      const fvmAllocHandle_t & handle_Store (get<long> (config, "handle_Store"));
      const fvmAllocHandle_t & scratch_Store (get<long> (config, "scratch_Store"));
      const long & store (get<long> (volume, buf + ".store"));

      comm_handle = fvmGetGlobalData
        ( handle_Store
        , store * sizeofBunchBuffer (Job)
        , sizeofBunchBuffer (Job)
        , Job.shift_for_Vol
        + Job.SubVolMemSize
        + buf_to_prefetch * sizeofBunchBuffer (Job)
        , scratch_Store
        );
    }

  if (buf_to_migrate >= 0)
    {
      const long & wait (get<long>(volume, "wait"));
      const long & bunches (get<long>(config, "BUNCHES_PER_OFFSET"));

      if (wait == bunches)
        {
          const value::type vol (get<value::type> (volume, "volume"));

          LOG (INFO, "clear vol " << vol);

          MigSubVol.clear();
        }

      const std::string buf ((buf_to_migrate == 0) ? "buffer0" : "buffer1");

      const value::type vol (get<value::type> (volume, "volume"));
      const value::type bunch (get<value::type> (volume, buf + ".bunch"));

      LOG (INFO, "migrate vol " << vol << " with bunch " << bunch);

      // Reconstruct the tracebunch out of memory
      const long oid (1 + get<long> (volume, "volume.offset"));
      const long bid (1 + get<long> (volume, buf + ".bunch.id"));

      char * migbuf ( ((char *)fvmGetShmemPtr())
                    + Job.shift_for_Vol
                    + Job.SubVolMemSize
                    + buf_to_migrate * sizeofBunchBuffer (Job)
                    );

      TraceBunch Bunch(migbuf,oid,1,bid,Job);

      // migrate the bunch to the subvolume
      const int & NThreads (get<long> (config, "NThreads"));

      char * _VMem  (((char *)fvmGetShmemPtr()) + Job.shift_for_TT);

      const fvmAllocHandle_t & handle_TT (get<long> (config, "handle_TT"));

      MigBunch2SubVol(Job,Bunch,MigSubVol,SincIntArray(),NThreads, _VMem, handle_TT);

      value::field("assigned", value::field(buf, volume_processed)) = false;
      value::field("filled", value::field(buf, volume_processed)) = false;
      value::field("free", value::field(buf, volume_processed)) = false;

      value::field("wait", volume_processed) = wait - 1;
    }

  if (buf_to_prefetch >= 0)
    {
      // finish prefetch
      waitComm (comm_handle);

      const std::string buf ((buf_to_prefetch == 0) ? "buffer0" : "buffer1");

      value::field("free", value::field(buf, volume_processed)) = true;
      value::field("filled", value::field(buf, volume_processed)) = true;
    }

  waitComm ( fvmPutGlobalData
             ( handle_Volume
             , vid * SizeVolWithBuffer
             , SizeVolWithBuffer
             , Job.shift_for_Vol
             , scratch_Volume
             )
           );

  we::loader::put_output (output, "volume_processed", volume_processed);

  LOG (INFO, "process: volume_processed " << volume_processed);
}

/* ************************************************************************* */

static void write ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  LOG (INFO, "write: got config " << config);
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

  const fvmAllocHandle_t & handle_Volume (get<long> (config, "handle_Volume"));
  const fvmAllocHandle_t & scratch_Volume (get<long> (config, "scratch_Volume"));

  const unsigned long SizeVolWithBuffer
    (Job.SubVolMemSize + 2 * sizeofBunchBuffer (Job));

  waitComm ( fvmGetGlobalData
             ( handle_Volume
             , vid * SizeVolWithBuffer
             , SizeVolWithBuffer
             , Job.shift_for_Vol
             , scratch_Volume
             )
           );

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

  we::loader::put_output (output, "volume", volume);
}

/* ************************************************************************* */

static void finalize ( void *
                     , const we::loader::input_t &  input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (get<value::type> (input, "config"));

  LOG (INFO, "finalize: got config " << config);

  const fvmAllocHandle_t & handle_TT (get<long> (config, "handle_TT"));
  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle_Job"));
  const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch_Job"));
  const fvmAllocHandle_t & handle_Store (get<long> (config, "handle_Store"));
  const fvmAllocHandle_t & scratch_Store (get<long> (config, "scratch_Store"));
  const fvmAllocHandle_t & handle_Volume (get<long> (config, "handle_Volume"));
  const fvmAllocHandle_t & scratch_Volume (get<long> (config, "scratch_Volume"));

  fvmGlobalFree (handle_TT);
  fvmGlobalFree (handle_Job);
  fvmGlobalFree (scratch_Job);
  fvmGlobalFree (handle_Store);
  fvmGlobalFree (scratch_Store);
  fvmGlobalFree (handle_Volume);
  fvmGlobalFree (scratch_Volume);

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
