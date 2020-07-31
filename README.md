# GPI-Space

GPI-Space is a task-based workflow management system for parallel
applications. It allows the developers to build domain-specific
workflows using their own parallelization patterns, data management
policies and I/O routines, while relying on the runtime system for the
workflow management. The GPI-Space ecosystem "auto-manages" the
application runs with dynamic scheduling, in-built distributed memory
transfer and distributed task execution.

## Installation
GPI-Space targets and has been successfully used on x86-64 Linux
systems. Other architectures are not supported.

The virtual memory layer can be backed with either Ethernet,
Infiniband or BeeOND.

### Dependencies
GPI-Space depends on the following packages:

* [Boost](https://boost.org) (>= 1.61, <= 1.65). Note the remarks below.
* [GPI-2](https://github.com/cc-hpc-itwm/GPI-2) (version 1.3.1)
* [GCC](https://gcc.gnu.org/) (>= 4.9.4), or compatible compiler
* [OpenSSL](https://www.openssl.org/)
* [CMake](https://cmake.org/) (>= 3.13)
* [hwloc](https://www.open-mpi.org/projects/hwloc/) (>= 1.10)
* [Qt5](https://www.qt.io/)
* [libssh2](https://www.libssh2.org/) (>= 1.7)
* [chrpath](https://tracker.debian.org/pkg/chrpath)

RHEL based system provide most of the packages out of the box. Only
`Boost` and `GPI-2` require manual installation on RHEL 7 and on older
RHEL 6 based systems `CMake` and `libssh2` are missing as well.

GPI-Space is built and tested daily on Centos 5, 6 and 7 machines.

#### Notes on building Boost
Boost needs to be installed to a directory containing only the Boost
installation, making it required to build Boost from source for
GPI-Space. It is strongly suggested to configure Boost as follows:

```bash
./bootstrap.sh --prefix="${BOOST_ROOT}"
./b2                                                              \
  clags="-fPIC -fno-gnu-unique"                                   \
  cxxflags="-fPIC -fno-gnu-unique"                                \
  link=static                                                     \
  variant=release                                                 \
  install
```

### Building the package
GPI-Space can be built as follows. The code listings in this document
assume

- `${GPISPACE_SOURCE_DIR}` to be the directory storing the GPI-Space
  sources.
- `${GPISPACE_BUILD_DIR}` to be an empty directory to be used for
  building GPI-Space.
- `${GPISPACE_INSTALL_DIR}` to be a directory to install GPI-Space
  to. It is suggested to use a previously empty directory on a shared
  filesystem.
- `${GPISPACE_TEST_DIR}` to be an empty directory on a shared
  filesystem, which used when running the system tests.
- `${BOOST_ROOT}` being the directory Boost was installed to.

```bash
cd "${GPISPACE_SOURCE_DIR}"
git submodule update --init --recursive

mkdir -p "${GPISPACE_BUILD_DIR}" && cd "${GPISPACE_BUILD_DIR}"

cmake -DCMAKE_BUILD_TYPE=Release                                  \
      -DCMAKE_INSTALL_PREFIX="${GPISPACE_INSTALL_DIR}"            \
      -DSHARED_DIRECTORY_FOR_TESTS="${GPISPACE_TEST_DIR}"         \
      -DTESTING_RIF_STRATEGY=ssh                                  \
      -DCMAKE_INSTALL_MESSAGE=LAZY                                \
      -DBOOST_ROOT="${BOOST_ROOT}"                                \
      "${GPISPACE_SOURCE_DIR}"

make -j$(nproc) install
```

After installing, you can verify the installation by running a simple
self-test as follows:

```bash
cd "${GPISPACE_BUILD_DIR}"

hostname > nodefile
export GSPC_NODEFILE_FOR_TESTS="${PWD}/nodefile"
# or to test in a cluster allocation:
# Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
# PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"

ctest --output-on-failure                                         \
      --tests-regex share_selftest
```

## Next steps
After a successful GPI-Space installation, the next step is to make
use of it and create the first application. For that, a step-by-step
guide on how to get started and create a first application is
available at `${GPISPACE_INSTALL_DIR}/share/gspc/getting_started.md`.
