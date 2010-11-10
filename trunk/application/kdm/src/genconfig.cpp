#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <fstream>
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

template<typename T>
T divru (const T & a, const T & b)
{
  return (a == 0) ? 0 : (1 + (a-1) / b);
}

static unsigned long sizeofJob (void)
{
  return sizeof(MigrationJob);
}

// ************************************************************************* //

static void genconf (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "file_config"));
  const std::string & outfilename (get<std::string> (input, "file_out"));
  long memsizeGPI (get<long> (input, "memsizeGPI"));

  MLOG (INFO, "genconfig: filename " << filename);
  MLOG (INFO, "genconfig: memsizeGPI " << memsizeGPI);

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

  Job.BunchMemSize = getSizeofTD(Job);
  Job.shift_for_TT = Job.SubVolMemSize + Job.BunchMemSize;

  MLOG(INFO, "Job.BunchMemSize = " << Job.BunchMemSize);

  memsizeGPI -= sizeofJob();            // handle_Job
  memsizeGPI -= sizeofJob();            // scratch_Job
  memsizeGPI -= Job.globTTbufsizelocal; // handle_TT
  
  const long per_offset_volumes (Job.NSubVols);
  const long offsets (Job.n_offset);
  const long node_count (fvmGetNodeCount());

  // WORK HERE: overcome this by using virtual offsetclasses
  const long volumes_per_node (divru (per_offset_volumes, node_count));

  memsizeGPI -= volumes_per_node * Job.SubVolMemSize; // handle_volume
  memsizeGPI -= Job.SubVolMemSize;                    // scratch_volume

  const long work_parts (offsets * per_offset_volumes);

  // TUNING: the 3: at least (k=3) times P workparts
  long copies (divru (3 * node_count, work_parts));

  // buffer for readTT, is 10 MiB enough?
  long bunch_store_per_node ((memsizeGPI - (10<<20)) / Job.BunchMemSize);

  const long per_offset_bunches (static_cast<long>(Nbid_in_pid (1, 1, Job)));

  long offsets_at_once (divru ( volumes_per_node * node_count 
			      , copies * per_offset_volumes
			      )
		       );

  std::ofstream out (outfilename.c_str());

  out << "threads.N " << NThreads << std::endl;

  // problem, derived
  out << "offsets " << offsets << std::endl;
  out << "per.offset.volumes " << per_offset_volumes << std::endl;
  out << "per.offset.bunches " << per_offset_bunches << std::endl;

  out << "loadTT.parallel " 
      << std::max (1L, static_cast<long>(fvmGetNodeCount())/2L)
      << std::endl;

  const long size_store_bunch ((bunch_store_per_node - 1) * node_count);

  // tuning
  out << "size.store.bunch " << size_store_bunch << std::endl;
  out << "per.volume.copies " << copies << std::endl;

  // tuning: volumes_per_node could be higher
  const long size_store_volume (volumes_per_node * node_count);

  out << "size.store.volume " << size_store_volume << std::endl;

  // tuning, derived?
  out << "assign.most "
      << divru (size_store_bunch, offsets_at_once) / 2
      << std::endl;

  put (output, "done", control());
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (genconf);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (genconf)");

  WE_REGISTER_FUN (genconf);
}
WE_MOD_INITIALIZE_END (genconf);

WE_MOD_FINALIZE_START (genconf);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (genconf)");
}
WE_MOD_FINALIZE_END (genconf);
