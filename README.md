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

* [Boost](https://boost.org) (>= 1.61, <= 1.63). Note the remarks below.
* [GPI-2](https://github.com/cc-hpc-itwm/GPI-2) (version 1.3.1)
* [GCC](https://gcc.gnu.org/) (>= 4.9.4), or compatible compiler
* [OpenSSL](https://www.openssl.org/)
* [CMake](https://cmake.org/) (>= 3.13)
* [hwloc](https://www.open-mpi.org/projects/hwloc/) (>= 1.10)
* [Qt5](https://www.qt.io/)
* [libssh2](https://www.libssh2.org/) (>= 1.7, built with OpenSSL as crypto backend)
* [chrpath](https://tracker.debian.org/pkg/chrpath)

> ---
> **WARNING: OpenSSL >= 1.1**
>
> * Boost(>= 1.62, <= 1.63)
> * libssh2(>= 1.8)
>
> Due to API changes in OpenSSL 1.1, Boost versions below 1.62 and libssh2 versions
> below 1.8 no longer compile with it.
>
> ---

> ---
> **WARNING: OpenSSH >= 7.8**
>
> * libssh2(>= 1.9) (see [Notes on building libssh2](#notes-on-building-libssh2))
>
> Starting with OpenSSH 7.8, the default SSH-key format has changed.
> Libssh2 is only capable of reading this key format in version 1.9 or above.
>
> ---

> ---
> **WARNING: Ubuntu 20.04**
>
> The packaged libssh2 is built with libgcrypt as backend.
> Additionally, Ubuntu 20.04 ships with OpenSSH 8.2.
>
> (see [Notes on building libssh2](#notes-on-building-libssh2))
>
> ---

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
  cflags="-fPIC -fno-gnu-unique"                                   \
  cxxflags="-fPIC -fno-gnu-unique"                                \
  link=static                                                     \
  variant=release                                                 \
  install
```

#### Notes on building libssh2

libssh2 is not built with the OpenSSL backend on all systems.
Additionally, some versions available via package manager might not be compatible with OpenSSH's
default settings.
Building libssh2 from scratch is however straightforward thanks to
CMake.
As additional dependencies OpenSSL and Zlib are required (e.g. from a package manager).
If the default install prefix for libssh2 is chosen (`/usr/local`), there is
no need to define `LIBSSH2_ROOT`.

```bash
libssh2_version=1.9.0
export LIBSSH2_ROOT=<install-prefix>

git clone --jobs $(nproc)                                         \
          --depth 1                                               \
          --shallow-submodules                                    \
          --recursive                                             \
          --branch libssh2-${libssh2_version}                     \
          https://github.com/libssh2/libssh2.git                  \
          libssh2

cmake -D CRYPTO_BACKEND=OpenSSL                                   \
      -D CMAKE_BUILD_TYPE=Release                                 \
      -D CMAKE_INSTALL_PREFIX="${LIBSSH2_ROOT}"                   \
      -D ENABLE_ZLIB_COMPRESSION=ON                               \
      -B libssh2/build                                            \
      -S libssh2

cmake --build libssh2/build                                       \
      --target install                                            \
      -j $(nproc)

export PKG_CONFIG_PATH="${LIBSSH2_ROOT}/lib/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
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

> ---
> **NOTE:**
>
> GPI-Space requires a working SSH environment with a password-less
> SSH-key when using the SSH RIF strategy.
>
> ---

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
