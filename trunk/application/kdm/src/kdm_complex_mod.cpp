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
using we::loader::put;

// ************************************************************************* //

static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

static void get_Job (const value::type & config, MigrationJob & Job)
{
  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle.job"));
  const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch.job"));

  waitComm (fvmGetGlobalData ( handle_Job
                             , fvmGetRank() * sizeofJob()
                             , sizeofJob()
                             , 0
                             , scratch_Job
                             )
           );

  memcpy (&Job, fvmGetShmemPtr(), sizeofJob());
}

static fvmAllocHandle_t alloc ( const long & size
                              , const std::string & descr
                              , long & memsizeGPI
                              )
{
  const fvmAllocHandle_t h (fvmGlobalAlloc (size));

  if (h == 0)
    {
      throw std::runtime_error ("KDM::initialize " + descr + " == 0");
    }

  memsizeGPI -= size;

  return h;
}

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "file_config"));
  long memsizeGPI (get<long> (input, "memsizeGPI"));

  MLOG (INFO, "initialize: filename " << filename);

  const int NThreads (4);

  MigrationJob Job;
  CheckReadMigrationJob JobReader;
  std::string CfgFileName (filename);

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
  }

  //WORK HERE: add sizeof scratch space here
  //  Job.ReqVMMemSize = Job.globTTbufsizelocal + 2 * sizeofJob();

  Job.BunchMemSize = getSizeofTD(Job);
  Job.shift_for_TT = Job.SubVolMemSize + Job.BunchMemSize;

  // Check whether the travel time table covers the entire output volume

  // Touch the offset gather
  MigrationFileHandler MFHandler;
  MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                             Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);

  const fvmAllocHandle_t handle_Job (alloc (sizeofJob(), "handle_Job", memsizeGPI));
  const fvmAllocHandle_t scratch_Job (alloc (sizeofJob(), "scratch_Job", memsizeGPI));

  memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

  for (int p (0); p < fvmGetNodeCount(); ++p)
    waitComm (fvmPutGlobalData (handle_Job, p * sizeofJob(), sizeofJob(), 0, scratch_Job));

  const fvmAllocHandle_t handle_TT (alloc (Job.globTTbufsizelocal, "handle_TT", memsizeGPI));

  long volumes_per_node (1);

  while (volumes_per_node * fvmGetNodeCount() < Job.NSubVols)
    {
      volumes_per_node += 1;
    }

  const fvmAllocHandle_t handle_volume
    (alloc (volumes_per_node * Job.SubVolMemSize, "handle_volume", memsizeGPI));

  const fvmAllocHandle_t scratch_volume (alloc (Job.SubVolMemSize, "scratch_volume", memsizeGPI));

  // needed scratch spaces
  // scratch_TT
  // segYHeader
  // TTLengthpart

  const long per_offset_bunches (static_cast<long>(Nbid_in_pid (1, 1, Job)));

  long size_store_bunch (per_offset_bunches);

  while (size_store_bunch * Job.BunchMemSize > memsizeGPI / 2)
    {
      size_store_bunch -= 1;
    }

  long bunch_store_per_node (1);

  while (bunch_store_per_node * fvmGetNodeCount() < size_store_bunch)
    {
      bunch_store_per_node += 1;
    }

  size_store_bunch = bunch_store_per_node * fvmGetNodeCount();

  const fvmAllocHandle_t handle_bunch
    (alloc (bunch_store_per_node * Job.BunchMemSize, "handle_bunch", memsizeGPI));

  const fvmAllocHandle_t scratch_bunch
    (alloc (Job.BunchMemSize, "scratch_bunch", memsizeGPI));

  put (output, "config", "handle.job", static_cast<long>(handle_Job));
  put (output, "config", "scratch.job", static_cast<long>(scratch_Job));
  put (output, "config", "handle.TT", static_cast<long>(handle_TT));
  put (output, "config", "threads.N", static_cast<long>(NThreads));
  put (output, "config", "handle.bunch", static_cast<long>(handle_bunch));
  put (output, "config", "scratch.bunch", static_cast<long>(scratch_bunch));
  put (output, "config", "handle.volume", static_cast<long>(handle_volume));
  put (output, "config", "scratch.volume", static_cast<long>(scratch_volume));

  put (output, "config", "offsets", static_cast<long> (Job.n_offset));
  put (output, "config", "per_offset.volumes", static_cast<long>(Job.NSubVols));
  put (output, "config", "per_offset.bunches", per_offset_bunches);
  put (output, "config", "loadTT.parallel", static_cast<long>(fvmGetNodeCount()));

  put (output, "config", "size.store.bunch", size_store_bunch);

  const long size_store_volume (volumes_per_node * fvmGetNodeCount());

  put (output, "config", "size.store.volume", size_store_volume);

  put (output, "config", "assign.most", size_store_bunch / 2);

  if ( get<long> (output, "config", "size.store.volume")
     < get<long> (output, "config", "per_offset.volumes")
     )
    {
      throw std::runtime_error
        ("need at least as many volume stores as volumes per offset");
    }

  MLOG (INFO, "initialize: config " << get<value::type>(output, "config"));
}

// ************************************************************************* //

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const long & id (get<long> (input, "id"));
  const long & parallel (get<long> (config, "loadTT.parallel"));

  MLOG (INFO, "loadTT: id " << id << " out of " << parallel);

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
  const int & NThreads (get<long> (config, "threads.N"));
  const fvmAllocHandle_t & handle_TT (get<long> (config, "handle.TT"));

  TTVMMemHandler TTVMMem;

  TTVMMem.InitVol(Job,Job.RTFileName,GSrc,GVol,NThreads,0, id, parallel, handle_TT);

  put (output, "done", control());
}

// ************************************************************************* //

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "load: bunch " << bunch);

  MigrationJob Job;

  get_Job (config, Job);

  const long & oid (get<long> (bunch, "offset.id"));
  const long & bid (get<long> (bunch, "id"));

  TraceBunch Bunch((char *)fvmGetShmemPtr(),oid+1,1,bid+1,Job);

  Bunch.LoadFromDisk_CO_MT(Job);

  const fvmAllocHandle_t handle_bunch (get<long> (config, "handle.bunch"));
  const fvmAllocHandle_t scratch_bunch (get<long> (config, "scratch.bunch"));

  const long & sid (get<long> (bunch, "store.id"));

  waitComm ( fvmPutGlobalData ( handle_bunch
                              , sid * Job.BunchMemSize
                              , Job.BunchMemSize
                              , 0
                              , scratch_bunch
                              )
           );

  put (output, "bunch", bunch);
}

// ************************************************************************* //

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "process: volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

  // load and reconstruct the volume
  const fvmAllocHandle_t & handle_volume (get<long>(config, "handle.volume"));
  const fvmAllocHandle_t & scratch_volume (get<long>(config, "scratch.volume"));
  const long & vid (get<long>(volume, "id"));

  char * pVMMemSubVol (((char *)fvmGetShmemPtr()));

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
  MigSubVol3D MigSubVol(MigVol,vid+1,Job.NSubVols);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  waitComm (fvmGetGlobalData ( handle_volume
                             , vid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , scratch_volume
                             )
           );

  literal::stack_type stack_bunch_id
    (get<literal::stack_type> (volume, "assigned.bunch.id"));
  literal::stack_type stack_store_id
    (get<literal::stack_type> (volume, "assigned.bunch.store.id"));

  MLOG (INFO, "process: size " << stack_bunch_id.size());

  const fvmAllocHandle_t & handle_bunch (get<long> (config, "handle.bunch"));
  const fvmAllocHandle_t & scratch_bunch (get<long> (config, "scratch.bunch"));

  while (!stack_bunch_id.empty())
    {
      if (stack_store_id.empty())
        {
          throw std::runtime_error ("BUMMER!");
        }

      const long & bid (stack_bunch_id.back());
      const long & sid (stack_store_id.back());
      const long & oid (get<long> (volume, "offset.id"));

      MLOG ( INFO
           , "process: match volume " << oid << "." << vid
           << " with bunch " << bid << " from store " << sid
           );

      waitComm ( fvmGetGlobalData ( handle_bunch
                                  , sid * Job.BunchMemSize
                                  , Job.BunchMemSize
                                  , Job.SubVolMemSize
                                  , scratch_bunch
                                  )
               );

      // Reconstruct the tracebunch out of memory
      char * migbuf (((char *)fvmGetShmemPtr()) + Job.SubVolMemSize);

      TraceBunch Bunch(migbuf,oid+1,1,bid+1,Job);

      // migrate the bunch to the subvolume
      const long & NThreads (get<long> (config, "threads.N"));

      char * _VMem  (((char *)fvmGetShmemPtr())
                    + Job.SubVolMemSize + Job.BunchMemSize
                    );

      const fvmAllocHandle_t & handle_TT (get<long> (config, "handle.TT"));

      MigBunch2SubVol(Job,Bunch,MigSubVol,SincIntArray(),NThreads, _VMem, handle_TT);

      stack_bunch_id.pop_back();
      stack_store_id.pop_back();
    }

  waitComm (fvmPutGlobalData ( handle_volume
                             , vid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , scratch_volume
                             )
           );

  put (output, "volume", volume);
}

// ************************************************************************* //

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "write: volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

  const fvmAllocHandle_t & handle_volume (get<long>(config, "handle.volume"));
  const fvmAllocHandle_t & scratch_volume (get<long>(config, "scratch.volume"));
  const long & vid (get<long>(volume, "id"));

  waitComm ( fvmGetGlobalData ( handle_volume
			      , vid * Job.SubVolMemSize
			      , Job.SubVolMemSize
			      , 0
			      , scratch_volume
			      )
           );

  // write offset class for a  given subvolume to disk
  char * pVMMemSubVol (((char *)fvmGetShmemPtr()));

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
  const long & oid (get<long> (volume, "offset.id"));

  MigSubVol3D MigSubVol(MigVol,vid+1,Job.NSubVols);

  MigrationFileHandler MFHandler;

  MFHandler.WriteOffsetClassForSubVol
    ( Job.MigFileName
    , G
    , (float *)pVMMemSubVol
    , MigSubVol.getix0(), MigSubVol.getNx()
    , MigSubVol.getiy0(), MigSubVol.getNy()
    , oid
    , 1
    , Job.NtotOffVol
    , Job.MigFileMode
    );

  put (output, "volume", volume);
}

// ************************************************************************* //

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  fvmGlobalFree (get<long> (config, "handle.job"));
  fvmGlobalFree (get<long> (config, "scratch.job"));
  fvmGlobalFree (get<long> (config, "handle.volume"));
  fvmGlobalFree (get<long> (config, "scratch.volume"));
  fvmGlobalFree (get<long> (config, "handle.bunch"));
  fvmGlobalFree (get<long> (config, "scratch.bunch"));
  fvmGlobalFree (get<long> (config, "handle.TT"));

  put (output, "trigger", control());
}

// ************************************************************************* //

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "init_volume: volume " << volume);

  MigrationJob Job;

  get_Job (config, Job);

  if (!Job.sinc_initialized)
  {
    MLOG (INFO, "Init SincInterpolator on node " << fvmGetRank());

    const long & NThreads (get<long> (config, "threads.N"));

    initSincIntArray(NThreads, Job.tracedt);

    Job.sinc_initialized = true;

    memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

    // rewrite Job
    const fvmAllocHandle_t & handle_Job (get<long> (config, "handle.job"));
    const fvmAllocHandle_t & scratch_Job (get<long> (config, "scratch.job"));

    waitComm (fvmPutGlobalData ( handle_Job
                               , fvmGetRank() * sizeofJob()
                               , sizeofJob()
                               , 0
                               , scratch_Job
                               )
             );
  }

  // init subvolume

  MLOG (INFO, "Init Subvolume");

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
  const long & vid (get<long> (volume, "id"));

  MigSubVol3D MigSubVol(MigVol, vid+1, Job.NSubVols);

  char * pVMMemSubVol (((char *)fvmGetShmemPtr()));

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // now, initialize the subvol
  MigSubVol.clear();

  const fvmAllocHandle_t & handle_volume (get<long>(config, "handle.volume"));
  const fvmAllocHandle_t & scratch_volume (get<long>(config, "scratch.volume"));

  waitComm (fvmPutGlobalData ( handle_volume
                             , vid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , scratch_volume
                             )
           );

  put (output, "volume", volume);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdm_complex);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdm_complex)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (init_volume);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdm_complex);

WE_MOD_FINALIZE_START (kdm_complex);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdm_complex)");
}
WE_MOD_FINALIZE_END (kdm_complex);
