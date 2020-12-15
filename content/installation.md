---
layout: page
title: Installation
permalink: /installation
---

GPI-Space targets and has been successfully used on x86-64 Linux
systems.
Other architectures are not guaranteed to work properly at this point.

The virtual memory layer can be backed with either Ethernet,
Infiniband or BeeOND.

GPI-Space supports multiple Linux distributions:
* Centos 6
* Centos 7
* Centos 8
* Ubuntu 18.04 LTS
* Ubuntu 20.04 LTS

It is built and tested daily on those systems.

### Dependencies
GPI-Space comes with some third-party dependencies.
The following are provided out of the box as packages or other binary distributions on most supported operating systems.

* [GCC](https://gcc.gnu.org/) (>= 4.9.4), or compatible compiler
* [OpenSSL](https://www.openssl.org/) (>= 0.9)
* [CMake](https://cmake.org/) (>= 3.13)
* [hwloc](https://www.open-mpi.org/projects/hwloc/) (>= 1.10)
* [Qt5](https://www.qt.io/) (>= 5.9)
* [chrpath](https://tracker.debian.org/pkg/chrpath) (>= 0.13)

`Boost` and `GPI-2` require a manual installation on all systems.
`libssh2` is also required to be built manually on some systems, due to build version compatibility issues with other dependencies.

#### Boost

| Website | Supported Versions |
| :-: | :-: |
| [Boost](https://boost.org) | >= 1.61, <= 1.63 |

Boost needs to be installed to a directory containing only the Boost
installation, making it required to build Boost from source for
GPI-Space. It is strongly suggested to configure and build Boost as follows:

> ---
> **WARNING:**
>
> Boost 1.61 is not compatible with **OpenSSL >= 1.1**.
>
> ---

```bash
boost_version=1.61.0
export BOOST_ROOT=<install-prefix>

git clone                                                         \
    --jobs $(nproc)                                               \
    --depth 1                                                     \
    --shallow-submodules                                          \
    --recursive                                                   \
    --branch boost-${boost_version}                               \
    https://github.com/boostorg/boost.git                         \
    boost

./boost/bootstrap.sh --prefix="${BOOST_ROOT}"
./boost/b2                                                        \
  -j $(nproc)                                                     \
  headers
./boost/b2                                                        \
  cflags="-fPIC -fno-gnu-unique"                                  \
  cxxflags="-fPIC -fno-gnu-unique"                                \
  link=static                                                     \
  variant=release                                                 \
  install
```

#### GPI-2

| Website | Supported Versions |
| :-: | :-: |
| [GPI-2](http://www.gpi-site.com) | 1.3.2 |

If Infiniband support is required, the `--with-ethernet` option can be omitted.

```bash
arch=$(getconf LONG_BIT)
export GASPI_ROOT=<install-prefix>
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}

RUN gpi2_version=1.3.2                                                        \
 && git clone                                                                 \
        --depth 1                                                             \
        --branch v${gpi2_version}                                             \
        https://github.com/cc-hpc-itwm/GPI-2.git                              \
        GPI-2                                                                 \
 && cd GPI-2                                                                  \
 && grep "^CC\s*=\s*gcc$" . -lR                                               \
    | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'                                   \
 && ./install.sh -p "${GASPI_ROOT}"                                           \
                 --with-fortran=false                                         \
                 --with-ethernet
```

#### libssh2

| Website | Supported Versions |
| :-: | :-: |
| [libssh2](https://www.libssh2.org/) | >= 1.7 |

`libssh2` is not built with the OpenSSL backend on all systems.
Additionally, some versions available via package manager might not be compatible with OpenSSH's
default settings.
For those reasons, we highly recommend building `libssh2` 1.9 from scratch.
Doing so is however straightforward thanks to CMake.
As additional dependencies `OpenSSL` and `Zlib` are required (e.g. from a package manager).
Also, unless `Libssh2_ROOT` is set to `/usr`, the `LD_LIBRARY_PATH` needs to be set in order for
applications to find the correct one.

> ---
> **WARNING:**
> * libssh2 1.7 is not compatible with **OpenSSL >= 1.1**.
> * libssh2 <= 1.8 is incompatible with the new default SSH-key format in **OpenSSH >= 7.8**.
>
> ---

```bash
libssh2_version=1.9.0
export Libssh2_ROOT=<install-prefix>
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

git clone --jobs $(nproc)                                         \
          --depth 1                                               \
          --shallow-submodules                                    \
          --recursive                                             \
          --branch libssh2-${libssh2_version}                     \
          https://github.com/libssh2/libssh2.git                  \
          libssh2

cmake -D CRYPTO_BACKEND=OpenSSL                                   \
      -D CMAKE_BUILD_TYPE=Release                                 \
      -D CMAKE_INSTALL_PREFIX="${Libssh2_ROOT}"                   \
      -D ENABLE_ZLIB_COMPRESSION=ON                               \
      -D BUILD_SHARED_LIBS=ON                                     \
      -B libssh2/build                                            \
      -S libssh2

cmake --build libssh2/build                                       \
      --target install                                            \
      -j $(nproc)
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

```bash
cd "${GPISPACE_SOURCE_DIR}"

mkdir -p "${GPISPACE_BUILD_DIR}" && cd "${GPISPACE_BUILD_DIR}"

cmake -C ${GPISPACE_SOURCE_DIR}/config.cmake                      \
      -B ${GPISPACE_BUILD_DIR}                                    \
      -S ${GPISPACE_SOURCE_DIR}

cmake --build ${GPISPACE_BUILD_DIR}                               \
      --target install                                            \
      -j $(nproc)
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
available [here]({{ "/how-to-use" | relative_url }}).
