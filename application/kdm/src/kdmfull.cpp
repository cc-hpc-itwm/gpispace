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

template<typename T>
T divru (const T & a, const T & b)
{
  return (a == 0) ? 0 : (1 + (a-1) / b);
}

static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

static void get_Job (const value::type & config, MigrationJob & Job)
{
  const fvmAllocHandle_t & handle_Job (get<long> (config, "handle.job"));

  waitComm (fvmGetGlobalData ( handle_Job
                             , fvmGetRank() * sizeofJob()
                             , sizeofJob()
                             , 0
                             , 0
                             )
           );

  memcpy (&Job, fvmGetShmemPtr(), sizeofJob());
}

static fvmAllocHandle_t alloc ( const long & size
                              , const std::string & descr
                              , long & memsizeGPI
                              )
{
  DMLOG (TRACE, "alloc: " << descr << ": " << size << " bytes");

  const fvmAllocHandle_t h (fvmGlobalAlloc (size, ("kdm." + descr).c_str ()));

  if (h == 0)
    {
      throw std::runtime_error ("KDM::initialize " + descr + " == 0");
    }

  memsizeGPI -= size;

  DMLOG (TRACE, "alloc: still free " << memsizeGPI << " bytes");

  return h;
}

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "config_file"));
  long memsizeGPI (get<long> (input, "memsizeGPI"));

  MLOG (INFO, "initialize: filename " << filename);
  MLOG (INFO, "initialize: memsizeGPI " << memsizeGPI);

  const int NThreads (1);

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

  Job.BunchMemSize = getSizeofTD(Job);
  Job.shift_for_TT = Job.SubVolMemSize + Job.BunchMemSize;

  MLOG(INFO, "Job.BunchMemSize = " << Job.BunchMemSize);

  // Touch the offset gather
  MigrationFileHandler MFHandler;
  MFHandler.TouchOffsetGather(Job,Job.MigFileName,Job.N0OffVol,
                             Job.NOffVol,Job.NtotOffVol,Job.MigFileMode);

  const fvmAllocHandle_t handle_Job (alloc (sizeofJob(), "handle_Job", memsizeGPI));

  memcpy (fvmGetShmemPtr(), &Job, sizeofJob());

  for (int p (0); p < fvmGetNodeCount(); ++p)
    waitComm (fvmPutGlobalData (handle_Job, p * sizeofJob(), sizeofJob(), 0, 0));

  const fvmAllocHandle_t handle_TT (alloc (Job.globTTbufsizelocal, "handle_TT", memsizeGPI));

  const long per_offset_volumes (Job.NSubVols);
  const long offsets (Job.n_offset);
  const long node_count (fvmGetNodeCount());

  MLOG(INFO, "per_offset_volumes = " << per_offset_volumes);
  MLOG(INFO, "offsets = " << offsets);
  MLOG(INFO, "node_count = " << node_count);

  const int nproc_per_node = 4;

  // WORK HERE: overcome this by using virtual offsetclasses
  const long volumes_per_node (nproc_per_node * divru (per_offset_volumes, node_count));

  MLOG(INFO, "volumes_per_node = " << volumes_per_node);

  const fvmAllocHandle_t handle_volume
    (alloc (volumes_per_node * Job.SubVolMemSize, "handle_volume", memsizeGPI));

  const long work_parts (offsets * per_offset_volumes);

  MLOG (INFO, "work_parts = " << work_parts);

  // TUNING: the 3: at least (k=3) times P workparts
  long copies (divru (3 * node_count, work_parts));

  MLOG (INFO, "copies = " << copies);

  // buffer for readTT, is 10 MiB enough?
  long bunch_store_per_node ((memsizeGPI - (10<<20)) / Job.BunchMemSize);

  MLOG (INFO, "bunch_store_per_node = " << bunch_store_per_node);

  const long per_offset_bunches (static_cast<long>(Nbid_in_pid (1, 1, Job)));

  MLOG (INFO, "per_offset_bunches = " << per_offset_bunches);

  long offsets_at_once (divru ( volumes_per_node * node_count
                              , copies * per_offset_volumes
                              )
                       );

  MLOG (INFO, "offsets_at_once  = " << offsets_at_once);

  bunch_store_per_node = 1 +
    std::min ( bunch_store_per_node
             , divru ( per_offset_bunches * 2 * offsets_at_once
                     , node_count
                     )
             );

  MLOG (INFO, "bunch_store_per_node = " << bunch_store_per_node);

  if (bunch_store_per_node < 2)
    {
      MLOG (INFO, "bunch_store_per_node < 2");

      throw std::runtime_error ("bunch_store_per_node < 2");
    }

  const long size_store_bunch ((bunch_store_per_node - 1) * node_count);

  MLOG (INFO, "size_store_bunch = " << size_store_bunch);

  const fvmAllocHandle_t handle_bunch
    (alloc ((bunch_store_per_node - 1) * Job.BunchMemSize, "handle_bunch", memsizeGPI));

  // machine
  output.bind ("config.threads.N", static_cast<long>(NThreads));

  // system
  output.bind ("config.handle.job", static_cast<long>(handle_Job));

  // problem, derived
  output.bind ("config.offsets", offsets);
  output.bind ("config.per.offset.volumes", per_offset_volumes);
  output.bind ("config.per.offset.bunches", per_offset_bunches);

  output.bind ("config.loadTT.parallel"
              , static_cast<long>(fvmGetNodeCount()) * 2
      //      , std::max (1L, static_cast<long>(fvmGetNodeCount())/2L)
      );
  output.bind ("config.handle.TT", static_cast<long>(handle_TT));

  // tuning
  output.bind ("config.size.store.bunch", size_store_bunch);
  output.bind ("config.per.volume.copies", copies);

  LOG_IF ( WARN
         , copies > 8
         ,  "#copies == " << copies << ", does this make any sense?"
         << " Probably you are running quite a small problem"
         << " on quite a large machine!?"
         );

  // tuning: volumes_per_node could be higher
  const long size_store_volume (volumes_per_node * node_count);

  output.bind ("config.size.store.volume", size_store_volume);

  // tuning, derived?
  output.bind ("config.assign.most"
              , divru (size_store_bunch, offsets_at_once) / 2
              );

  // tuning induced
  output.bind ("config.handle.bunch", static_cast<long>(handle_bunch));
  output.bind ("config.handle.volume", static_cast<long>(handle_volume));

  // WORK HERE: overcome this by using virtual offsetclasses
  if ( get<long> (output, "config", "size.store.volume")
     < ( get<long> (output, "config", "per.offset.volumes")
       * get<long> (output, "config", "per.volume.copies")
       )
     )
    {
      MLOG (INFO, "need at least as many volume stores as volumes per offset");

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

  output.bind ("done", we::type::literal::control());
}

// ************************************************************************* //

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "load: bunch " << bunch);

  MigrationJob Job;

  get_Job (config, Job);

  const long & oid (get<long> (bunch, "bunch.offset.id"));
  const long & bid (get<long> (bunch, "bunch.id"));

  TraceBunch Bunch((char *)fvmGetShmemPtr(),oid+1,1,bid+1,Job);

  Bunch.LoadFromDisk_CO_MT(Job);

  const fvmAllocHandle_t handle_bunch (get<long> (config, "handle.bunch"));

  const long & sid (get<long> (bunch, "store.id"));

  waitComm ( fvmPutGlobalData ( handle_bunch
                              , sid * Job.BunchMemSize
                              , Job.BunchMemSize
                              , 0
                              , 0
                              )
           );

  output.bind ("bunch", pnet::type::compat::COMPAT (bunch));
}

// ************************************************************************* //

static void initialize_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "init_volume: volume " << volume << " config := " << config);

  MigrationJob Job;

  get_Job (config, Job);

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
  const long & vid (get<long> (volume, "volume.id"));

  MigSubVol3D MigSubVol(MigVol, vid+1, Job.NSubVols);

  char * pVMMemSubVol (((char *)fvmGetShmemPtr()));

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  // now, initialize the subvol
  MigSubVol.clear();

  const fvmAllocHandle_t & handle_volume (get<long>(config, "handle.volume"));

  const long & sid (get<long> (volume, "store.id"));

  waitComm (fvmPutGlobalData ( handle_volume
                             , sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , 0
                             )
           );

  output.bind ("volume", pnet::type::compat::COMPAT (volume));
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
  const long & vid (get<long>(volume, "volume.volume.id"));

  MigSubVol3D MigSubVol(MigVol,vid+1,Job.NSubVols);

  MigSubVol.setMemPtr((float *)pVMMemSubVol,Job.SubVolMemSize);

  const long & volume_sid (get<long>(volume, "volume.store.id"));

  waitComm (fvmGetGlobalData ( handle_volume
                             , volume_sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , 0
                             )
           );

  const fvmAllocHandle_t & handle_bunch (get<long> (config, "handle.bunch"));

      const long & bid (get<long> (volume, "assigned.bunch.id"));
      const long & sid (get<long> (volume, "assigned.store.id"));
      const long & oid (get<long> (volume, "volume.volume.offset.id"));

      MLOG ( INFO
           , "process: match volume " << oid << "." << vid
           << " with bunch " << bid << " from store " << sid
           );

      waitComm ( fvmGetGlobalData ( handle_bunch
                                  , sid * Job.BunchMemSize
                                  , Job.BunchMemSize
                                  , Job.SubVolMemSize
                                  , 0
                                  )
               );

      // Reconstruct the tracebunch out of memory
      char * migbuf (((char *)fvmGetShmemPtr()) + Job.SubVolMemSize);

      TraceBunch Bunch(migbuf,oid+1,1,bid+1,Job);

      // migrate the bunch to the subvolume
      const long & NThreads (get<long> (config, "threads.N"));

      char * _VMem  (((char *)fvmGetShmemPtr()) + Job.shift_for_TT);

      const fvmAllocHandle_t & handle_TT (get<long> (config, "handle.TT"));

      MigBunch2SubVol(Job,Bunch,MigSubVol,NThreads, _VMem, handle_TT);

  waitComm (fvmPutGlobalData ( handle_volume
                             , volume_sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , 0
                             )
           );

  output.bind ("volume", pnet::type::compat::COMPAT (volume));
}

// ************************************************************************* //

static void reduce (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & pair (get<value::type> (input, "pair"));
  const value::type & volume (get<value::type> (pair, "vol"));
  const value::type & sum (get<value::type> (pair, "sum.volume"));

  MLOG (INFO, "reduce: volume " << volume);
  MLOG (INFO, "reduce: sum " << sum);

  const long & vid (get<long>(volume, "volume.id"));
  const long & sid (get<long>(sum, "volume.id"));
  const long & volume_sid (get<long>(volume, "store.id"));
  const long & sum_sid (get<long>(sum, "store.id"));
  const fvmAllocHandle_t & handle_volume (get<long>(config, "handle.volume"));

  MigrationJob Job;

  get_Job (config, Job);

  float * pVMMemVol ((float *)((char *)fvmGetShmemPtr()));
  float * pVMMemSum ((float *)((char *)fvmGetShmemPtr() + Job.SubVolMemSize));

  point3D<float> X0(Job.MigVol.first_x_coord.v,
                   Job.MigVol.first_y_coord.v,
                   Job.MigVol.first_z_coord);
  point3D<int>   Nx(Job.MigVol.nx_xlines,
                   Job.MigVol.ny_inlines,
                   Job.MigVol.nz);
  point3D<float> dx(Job.MigVol.dx_between_xlines,
                   Job.MigVol.dy_between_inlines,
                   Job.MigVol.dz);
  MigVol3D Full(X0,Nx,dx);


  MigSubVol3D VolSub(Full,vid+1,Job.NSubVols);
  MigSubVol3D SumSub(Full,sid+1,Job.NSubVols);

  if (  VolSub.getNx() != SumSub.getNx()
     || VolSub.getNy() != SumSub.getNy()
     || VolSub.getNz() != SumSub.getNz()
     )
    {
      throw std::runtime_error ("reduce: size conflict!");
    }

  // get the data
  waitComm (fvmGetGlobalData ( handle_volume
                             , volume_sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             , 0
                             )
           );


  waitComm (fvmGetGlobalData ( handle_volume
                             , sum_sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             )
           );

  // calculate SumSub += VolSub
  for ( long i (0)
      ; i < VolSub.getNx() * VolSub.getNy() * VolSub.getNz()
      ; ++i
      )
    {
      pVMMemSum[i] += pVMMemVol[i];
    }

  // store the result
  waitComm (fvmPutGlobalData ( handle_volume
                             , sum_sid * Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , Job.SubVolMemSize
                             , 0
                             )
           );

  output.bind ("pair", pnet::type::compat::COMPAT (pair));
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
  const long & sid (get<long>(volume, "store.id"));

  waitComm ( fvmGetGlobalData ( handle_volume
                              , sid * Job.SubVolMemSize
                              , Job.SubVolMemSize
                              , 0
                              , 0
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
  const long & vid (get<long>(volume, "volume.id"));
  const long & oid (get<long> (volume, "volume.offset.id"));

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

  output.bind ("volume", pnet::type::compat::COMPAT (volume));
}

// ************************************************************************* //

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  fvmGlobalFree (get<long> (config, "handle.job"));
  fvmGlobalFree (get<long> (config, "handle.volume"));
  fvmGlobalFree (get<long> (config, "handle.bunch"));
  fvmGlobalFree (get<long> (config, "handle.TT"));

  output.bind ("trigger", we::type::literal::control());
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdmfull);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdmfull)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (initialize_volume);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (reduce);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdmfull);

WE_MOD_FINALIZE_START (kdmfull);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdmfull)");
}
WE_MOD_FINALIZE_END (kdmfull);
