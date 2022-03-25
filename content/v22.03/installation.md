---
layout: versioned_page
title: Installation
version: 22.03
permalink: /v22.03/installation
---

GPI-Space targets x86-64 Linux systems. Other architectures are not
supported at this point. It is built and tested daily on

* Centos 7
* Oracle Linux 8
* Ubuntu 18.04 LTS
* Ubuntu 20.04 LTS

The virtual memory layer can be backed with either Ethernet,
Infiniband or BeeOND.

GPI-Space is usually installed on cluster systems where a shared
filesystem is available on all nodes. It is suggested to use a
previously empty directory on such a shared filesystem for the
GPI-Space installation and all manually installed dependencies.

The source and build directories are not required to be shared
and for best performance it is recommended to place them in a
fast (local) file system.

### Spack Package

The recommended way of installing GPI-Space is with the `Spack` package
manager:

  - [Spack - Getting Started](https://spack.readthedocs.io/en/latest/getting_started.html)
  - [Spack - Basic Usage](https://spack.readthedocs.io/en/latest/basic_usage.html)

The most recent version of GPI-Space can be installed using the following
command:

```bash
spack install gpi-space
```

> ---
> **NOTE:**
>
> There might be a time delay between a GPI-Space release and its availability in the Spack package.
>
> ---

### Manual Installation

Alternatively, GPI-Space can also be installed manually.
GPI-Space requires some third-party dependencies. Most of the
following are provided out of the box as (-devel/-dev) packages or
other binary distributions on most supported operating systems.

`Boost` and `GPI-2` require a manual installation on all systems.
`libssh2` is also required to be built manually on some systems, due
to build version compatibility issues with other dependencies.

Note that some GPI-Space components can be disabled, which may remove
some dependencies needed. Also see the "Building GPI-Space" section
and subsection "Optional Components" below.

* [GCC](https://gcc.gnu.org/) (>= 5.5.0), or compatible compiler
* [CMake](https://cmake.org/) (>= 3.15)
  * Some distributions name the binary `cmake3` while others use
    `cmake`. Snippets below assume `cmake`.
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) (>= 0.23)
  , or an equivalent [pkgconf](http://pkgconf.org/). Both offer the `pkg-config`
  program.
* [OpenSSL](https://www.openssl.org/) (>= 0.9)
  * When using OpenSSL >= 1.1, Boost >= 1.62 and libssh2 (>= 1.8) are
    required for compatibility.
* [hwloc](https://www.open-mpi.org/projects/hwloc/) (>= 1.10)
* [Qt5](https://www.qt.io/) (>= 5.9)
  * Only required if GPI-Space is built with `-DGSPC_WITH_MONITOR_APP=ON`.
* [chrpath](https://tracker.debian.org/pkg/chrpath) (>= 0.13)
* [Boost](https://boost.org) (>= 1.61, <= 1.63)
  * When using OpenSSL >= 1.1, Boost >= 1.62 is required.
* [libssh2](https://www.libssh2.org/) (>= 1.7, built with OpenSSL backend)
  * When SSH keys are generated with OpenSSH >= 7.8, only libssh2 >= 1.9
    is compatible.
  * Among others, Ubuntu 20.04 ships with libgcrypt as backend as well
    as OpenSSH 8.2, thus needs a custom installation of libssh2.
* [GPI-2](http://www.gpi-site.com) (1.3.3)

> ---
> **WARNING:**
>
> It is important to keep the versions of all dependencies used in
> GPI-Space as well as the applications using GPI-Space synchronized
> to avoid hard to debug conflicts at run-time. Using a single
> environment to build the entire stack is strongly encouraged.
>
> ---

#### Boost

> ---
> **WARNING:**
>
> Boost 1.61 is not compatible with **OpenSSL >= 1.1**.
>
> ---

Boost needs to be installed to a directory containing only the Boost
installation, hence GPI-Space requires to build Boost from source.

Due to the size of Boost, extracting the sources and building might
take some time. A local non-shared filesystem is strongly suggested
to be used for building Boost.

As this Boost installation is also used by GPI-Space applications,
additional requirements like specifying a Python installation to use
may be required.

```bash
export BOOST_ROOT=<boost-install-prefix>
boost_version=1.63.0

wget "https://downloads.sourceforge.net/project/boost/boost/${boost_version}/boost_${boost_version//./_}.tar.gz" \
  -O boost_${boost_version//./_}.tar.gz
tar xf "boost_${boost_version//./_}.tar.gz"
cd "boost_${boost_version//./_}"

./bootstrap.sh --prefix="${BOOST_ROOT}"
./b2                               \
  cflags="-fPIC -fno-gnu-unique"   \
  cxxflags="-fPIC -fno-gnu-unique" \
  link=static                      \
  variant=release                  \
  install                          \
  -j $(nproc)
```

#### GPI-2

GPI-2 supports both Infiniband and Ethernet, but the decision has to
be made at compilation time. If the system used does not support
Infiniband, `with_ethernet=--with-ethernet` needs to be specified
below or installing GPI-2 will fail.

```bash
GPI2_ROOT=<gpi2-install-prefix>
#with_ethernet=--with-ethernet ## remove '#' if no Infiniband
gpi2_version=1.3.3

export PKG_CONFIG_PATH="${GPI2_ROOT}/lib64/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}

wget "https://github.com/cc-hpc-itwm/GPI-2/archive/v${gpi2_version}.tar.gz" \
  -O "GPI-2-${gpi2_version}.tar.gz"
tar xf "GPI-2-${gpi2_version}.tar.gz"
cd "GPI-2-${gpi2_version}"

grep "^CC\s*=\s*gcc$" . -lR | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'
./install.sh -p "${GPI2_ROOT}" \
  --with-fortran=false         \
  ${with_ethernet:-}
```

#### libssh2

> ---
> **WARNING:**
> * libssh2 1.7 is not compatible with **OpenSSL >= 1.1**.
> * libssh2 <= 1.8 is incompatible with the new default SSH-key format in **OpenSSH >= 7.8**.
>
> ---

`libssh2` can be built with various backends, but GPI-Space requires
`OpenSSL` to be used, which is not the default on various
distributions. In some distributions the `OpenSSH` default settings
are incompatible with the `libssh2` package. It is thus highly
recommended to build `libssh2` version 1.9 from scratch.

`libssh2` requires the dependencies `OpenSSL` and `zlib` (which are
assumed to be installed via the system package manager below).

```bash
Libssh2_ROOT=<libssh2-install-prefix>
libssh2_version=1.9.0

export PKG_CONFIG_PATH="${Libssh2_ROOT}/lib/pkgconfig:${Libssh2_ROOT}/lib64/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}

wget "https://github.com/libssh2/libssh2/releases/download/libssh2-${libssh2_version}/libssh2-${libssh2_version}.tar.gz" \
  -O "libssh2-${libssh2_version}.tar.gz"
tar xf "libssh2-${libssh2_version}.tar.gz"
mkdir "libssh2-${libssh2_version}/build"
cd "libssh2-${libssh2_version}/build"

cmake -D CRYPTO_BACKEND=OpenSSL                 \
      -D CMAKE_BUILD_TYPE=Release               \
      -D CMAKE_INSTALL_PREFIX="${Libssh2_ROOT}" \
      -D ENABLE_ZLIB_COMPRESSION=ON             \
      -D BUILD_SHARED_LIBS=ON                   \
      -D BUILD_TESTING=OFF                      \
      ..
cmake --build . --target install -- -j$(nproc)
```

#### Building GPI-Space

GPI-Space can be built as described below. The dependencies are found
via various environment variables in addition to being searched in
system directories. Some dependencies may have multiple ways to be
found. The dependency-install snippets above already set the
corresponding variables. It may be required to set additional
environment or CMake variables if packages are not installed
system-wide.

- Boost: `BOOST_ROOT`. Newer versions use CMake's new config format
  and will be automatically preferred if existing. Using config format
  installations can be disabled with the CMake option
  `-DBoost_NO_BOOST_CMAKE=on`.
- GPI-2: `PKG_CONFIG_PATH`
- libssh2: `PKG_CONFIG_PATH`. Note that libssh2 also installs a CMake
  new-config-format file, but GPI-Space currently does not support it.
- OpenSSL: `OPENSSL_ROOT_DIR`
- hwloc: `PKG_CONFIG_PATH`
- Qt5: `PATH` (the `qmake`/`qmake-qt5` binary), `Qt5_ROOT`, `Qt5_DIR`
  or `CMAKE_PREFIX_PATH`
- chrpath: `PATH`

By default, GPI-Space does not build unit- and system tests. To enable
them, the options `-DBUILD_TESTING=ON` and
`-DSHARED_DIRECTORY_FOR_TESTS=<shared-directory>` need to be
given to CMake by setting `build_tests` in the snippet
below. `SHARED_DIRECTORY_FOR_TESTS` shall point to an existing but
empty directory on a shared filesystem. The tests can then be run with
`GSPC_NODEFILE_FOR_TESTS=<path to nodefile> ctest` within the build
directory.

```bash
export GPISpace_ROOT=<gpispace-install-prefix>
# to build tests, remove leading # and choose a shared directory to be
# used to store temporary data.
#build_tests="-DBUILD_TESTING=on -DSHARED_DIRECTORY_FOR_TESTS=<shared-directory>"
gpispace_version=main

# While CMake finds GPI-Space via GPISpace_ROOT, many applications
# and tools still use this variable for legacy reasons.
export GSPC_HOME="${GPISpace_ROOT}"

wget "https://github.com/cc-hpc-itwm/gpispace/archive/${gpispace_version}.tar.gz" \
  -O "gpispace-${gpispace_version}.tar.gz"
tar xf "gpispace-${gpispace_version}.tar.gz"

cmake -D CMAKE_INSTALL_PREFIX="${GPISpace_ROOT}"  \
      -B "gpispace-${gpispace_version}/build" \
      -S "gpispace-${gpispace_version}"       \
      ${build_tests:-}
cmake --build "gpispace-${gpispace_version}/build" \
      --target install                             \
      -j $(nproc)
```

After installing, the installation can be verified by running a simple
self-test example as follows:

> ---
> **NOTE:**
>
> GPI-Space requires a working SSH environment with a password-less
> SSH-key when using the SSH RIF strategy, the default for most
> applications.
>
> By default, `${HOME}/.ssh/id_rsa` is used for authentication. If no
> such key exists,
>
> ```bash
> ssh-keygen -t rsa -b 4096 -N '' -f "${HOME}/.ssh/id_rsa"
> ssh-copy-id -f -i "${HOME}/.ssh/id_rsa" "${HOSTNAME}"
> ```
> can be used to create and register one.
>
> ---

```bash
# to test with multiple nodes, set GSPC_NODEFILE_FOR_TESTS
#   Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
#   PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"
# and SWH_INSTALL_DIR:
#   export SWH_INSTALL_DIR=<a-shared-directory-visible-on-all-nodes>

"${GPISpace_ROOT}/share/GPISpace/doc/example/stochastic_with_heureka/selftest"
```

If GPI-Space has been built with testing enabled, then `ctest` can be
used to execute the unit- and system tests:

```bash
# it is always required to set GSPC_NODEFILE_FOR_TESTS for ctest:
#   Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
#   PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"
#   single-host: export GSPC_NODEFILE_FOR_TESTS=/etc/hostname
#     or hostname > nodefile; export ...="${PWD}/nodefile"
export GSPC_NODEFILE_FOR_TESTS=<path-to-a-nodefile>

cd "gpispace-${gpispace_version}/build"
ctest --output-on-failure \
      -j $(nproc)
```

## Optional Components

GPI-Space supports disabling some components at configuration time to
reduce the dependencies needed as well as build time.

### GSPC Monitor

The `gspc-monitor` (also known as "gantt") application for execution monitoring
requires Qt5. The following options can be used to enable or disable this
feature (enabled by default):

| Spack | CMake |
| - | - |
| `[+|~]monitor` | `-D GSPC_WITH_MONITOR_APP=[ON|OFF]` |

## Next steps

After a successful GPI-Space installation, the next step is to make
use of it and create the first application. For that, a step-by-step
guide on how to get started and create a first application is
available [here]({{ "v" | append: page.version | append: "/how-to-use" | relative_url }}).
