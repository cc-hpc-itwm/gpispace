LOG (INFO, "INIT");

determine_size (file_input, type_input, config.num.trace, config.size.trace);

LOG (TRACE, "num.trace = " << config.num.trace);
LOG (TRACE, "size.trace = " << config.size.trace);

config.per.node.mem.shmem = shmem_per_node;
config.per.node.mem.gpi = gpi_mem_per_node;

config.param.Vnx = Vnx;
config.param.Vny = Vny;
config.param.Vnz = Vnz;

config.param.dx = dx;
config.param.dy = dy;
config.param.dz = dz;

config.param.zmax = zmax;

config.param.lx = lx;
config.param.ly = ly;

config.param.f1 = f1;
config.param.f2 = f2;
config.param.f3 = f3;
config.param.f4 = f4;

config.param.pad = pad;

config.param.latSamplesPerWave = latSamplesPerWave;
config.param.vertSamplesPerWave = vertSamplesPerWave;

config.param.medium = medium;
config.param.propagator = propagator;

config.param.vPFile = vPFile;
config.param.eFile = eFile;
config.param.dFile = dFile;

#define IMIN(a,b) ((a)<(b) ? (a) : (b))
#define NINT(x) ((long)((x)>0.0?(x)+0.5:(x)-0.5))

config.param.nx = NINT (lx / dx) + 1;
config.param.ny = NINT (ly / dy) + 1;

config.param.nz = (zmax > 0.0) ? IMIN (Vnz, NINT (zmax / dz)) : Vnz;

LOG (TRACE, "nx " << config.param.nx
      << ", ny " << config.param.ny
      << ", nz " << config.param.nz
    );

config.param.nxf = npfao (config.param.nx, 2 * config.param.nx);
config.param.nyf = npfao (config.param.ny, 2 * config.param.ny);

const long size_vpcube = ( config.param.nz
                         * config.param.nxf
                         * config.param.nyf
                         * sizeof(float)
                         );

LOG (TRACE, "nxf " << config.param.nxf
       << ", nyf " << config.param.nyf
       << ", size(vPCube) " << size_vpcube
    );

if (config.per.node.mem.shmem < size_vpcube)
  {
    throw std::runtime_error ("not enough shmem for vpcube");
  }

config.per.node.mem.shmem -= size_vpcube;

if (  !boost::filesystem::exists (vPFile)
   || !boost::filesystem::is_regular (vPFile)
   )
  {
    throw std::runtime_error ("missing regular vpFile " + vPFile);
  }

config.size.vpfile = static_cast<long> (boost::filesystem::file_size (vPFile));

LOG (TRACE, "filesize (vpfile) " << config.size.vpfile);

const long size_vpfile_per_node
  ((config.size.vpfile + fvmGetNodeCount() - 1) / fvmGetNodeCount());

config.handle.vpfile.data
  = ::sp_par_vel::alloc (size_vpfile_per_node, "vpfile.data");
config.handle.vpfile.scratch
  = ::sp_par_vel::alloc (config.param.nxf * config.param.Vnz, "vpfile.scratch");

config.per.node.mem.gpi -= size_vpfile_per_node;
config.per.node.mem.gpi -= config.param.nxf * config.param.Vnz;

config.size.output_per_shot
 = config.param.nx * config.param.ny
 * (sizeof (SegYHeader) + config.param.nz * sizeof(float))
 ;

LOG (TRACE, "size.output_per_shot = " << config.size.output_per_shot);

if (config.per.node.mem.shmem < config.size.output_per_shot)
  {
    throw std::runtime_error ("not enough memory for output (shmem)");
  }

LOG(TRACE, "left: shmem " << config.per.node.mem.shmem
   << ", gpi " << config.per.node.mem.gpi
   );

const long gpi_mem_for_traces
  (config.per.node.mem.gpi - 2 * config.size.output_per_shot);
const long shmem_for_traces
  (config.per.node.mem.shmem - config.size.output_per_shot);

const long mem_for_trace_bunch (std::min ( gpi_mem_for_traces / 2
                                         , shmem_for_traces
                                         )
                               );

LOG(TRACE, "mem_for_trace_bunch " << mem_for_trace_bunch);

config.per.bunch.trace = mem_for_trace_bunch / config.size.trace;

LOG (TRACE, "per.bunch.trace = " << config.per.bunch.trace);

if (config.per.bunch.trace < 1)
  {
    throw std::runtime_error ("not enough memory for traces (shmem)");
  }

config.size.bunch = config.per.bunch.trace * config.size.trace;

const long slot_per_node (gpi_mem_for_traces / config.size.bunch);

if (slot_per_node < 2)
  {
    throw std::runtime_error ("not enough memory for traces (gpi)");
  }

config.num.slot = (slot_per_node - 1) * fvmGetNodeCount();

if (config.num.slot * fvmGetNodeCount() < 2)
  {
    throw std::runtime_error ("not enough memory (total)");
  }

LOG (TRACE, "num.slot = " << config.num.slot);

config.handle.input.data =
  ::sp_par_vel::alloc ((slot_per_node - 1) * config.size.bunch, "input.data");
config.handle.input.scratch =
  ::sp_par_vel::alloc (                 1  * config.size.bunch, "input.scratch");
config.handle.output.data =
  ::sp_par_vel::alloc (config.size.output_per_shot, "output.data");
config.handle.output.scratch =
  ::sp_par_vel::alloc (config.size.output_per_shot, "output.scratch");

config.file.input.name = file_input;
config.file.input.type = type_input;
config.file.output.name = file_output;
config.file.output.type = type_output;

config.per.node.mem.shmem = shmem_per_node;
config.per.node.mem.gpi = gpi_mem_per_node;

config.num.nodes = fvmGetNodeCount();

give_back_trace = 0;

size_wanted = config.size.trace * config.per.bunch.trace;
num_trace = config.num.trace;

num_output_slot = fvmGetNodeCount();

shot_num = shot_trace = 0;

std::ofstream out (config.file.output.name.c_str());

if (!out.good())
  {
    throw std::runtime_error
      ("could not open output file " + config.file.output.name);
  }
