| [Home](../../../../../index.md) | [Previous](1_tldr-quickstart.md) | [Up](../devstack.md) | [Next](3_advanced-usage.md) |
| :-: | :-: | :-: | :-: |

---

<br/>

# Getting Started

This short guide describes all the necessary steps to get started with a minimal DevStack.
It is structured in two parts.
The first part covers a GPISpace Dockerfile and goes through it one step at a time before creating a Docker image from it.
The second part describes how to create and launch a container from the previous image and how to use it as a minimal DevStack.

More advanced setups are covered in subsequent chapters further down the line.

## Table of Contents

1. [Docker Image](#docker-image)
2. [Running a Minimal DevStack](#running-a-minimal-devstack)

---

<br/>

## Docker Image

The containerization of the GPISpace DevStack is shown on an example building it on CentOS 7 with a GCC 8.3 C++ compiler.
The first step in recreating a DevStack inside a container is to provide a suitable definition file.

```docker
FROM centos:7

RUN yum -y update                                                             \
 && yum -y install centos-release-scl                                         \
                   epel-release                                               \
 && yum -y install asciidoc                                                   \
                   bzip2-devel                                                \
                   ccache                                                     \
                   chrpath                                                    \
                   cmake3                                                     \
                   devtoolset-8-gcc-c++                                       \
                   devtoolset-8-make                                          \
                   glibc-static                                               \
                   hwloc-devel                                                \
# missing hwloc dependency                                                    \
                   libssh2-devel                                              \
                   libtool-ltdl-devel                                         \
# netstat needed for fixed-port test                                          \
                   net-tools                                                  \
                   numactl-devel                                              \
                   openssh-server                                             \
# not automatically pulled in by devel, needed for generating keys            \
                   openssl                                                    \
                   openssl-devel                                              \
                   qt5-qtbase-devel                                           \
                   rh-git218                                                  \
                   source-highlight                                           \
                   sudo                                                       \
# required for building gpi and boost                                         \
                   which                                                      \
                   zlib-devel                                                 \
 && yum -y clean all --enablerepo='*'

ENV BOOST_ROOT="/opt/boost"
RUN boost_version=1.61.0                                                      \
 && source scl_source enable devtoolset-8 rh-git218                           \
 && git clone                                                                 \
        --jobs $(nproc)                                                       \
        --depth 1                                                             \
        --shallow-submodules                                                  \
        --recursive                                                           \
        --branch boost-${boost_version}                                       \
        https://github.com/boostorg/boost.git                                 \
        boost                                                                 \
 && cd boost                                                                  \
 && ./bootstrap.sh --prefix=${BOOST_ROOT}                                     \
 && ./b2 -j $(nproc) headers                                                  \
 && ./b2 cxxflags="-fPIC -fno-gnu-unique" cflags="-fPIC -fno-gnu-unique"      \
         -j $(nproc) --without-mpi --without-python link=static install       \
 && grep -lr '# *pragma *message' "${BOOST_ROOT}/include/boost/type_traits"   \
    | xargs sed -i'' -e '/^# *pragma *message.* is deprecated.*/d'            \
 && rm -rf "${PWD}"

RUN gpi2_version=1.3.1                                                        \
 && source scl_source enable devtoolset-8 rh-git218                           \
 && git clone                                                                 \
        --depth 1                                                             \
        --branch v${gpi2_version}                                             \
        https://github.com/cc-hpc-itwm/GPI-2.git                              \
        GPI-2                                                                 \
 && cd GPI-2                                                                  \
 && grep "^CC\s*=\s*gcc$" . -lR                                               \
    | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'                                   \
 && ./install.sh -p "/usr" --with-fortran=false --with-ethernet               \
 && rm -rf "${PWD}"

RUN :                                                                         \
# cmake comes as cmake3. use cmake for consistency in build scripts           \
 && ln -s /usr/bin/cmake3 /usr/bin/cmake                                      \
 && ln -s /usr/bin/ctest3 /usr/bin/ctest                                      \
                                                                              \
# we want to test in user space, so create a user                             \
 && groupadd ci-user                                                          \
 && useradd -r -m -g ci-user ci-user                                          \
 && mkdir -p /home/ci-user/.ssh                                               \
 && ssh-keygen -t rsa -N '' -f /home/ci-user/.ssh/id_rsa                      \
 && cp /home/ci-user/.ssh/id_rsa.pub /home/ci-user/.ssh/authorized_keys       \
 && chown -R ci-user:ci-user /home/ci-user/.ssh/                              \
                                                                              \
# setup required to start sshd in entrypoint.sh                               \
 && ssh-keygen -A                                                             \
 && echo "%ci-user ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/99-gpispace      \
                                                                              \
# prolog for ci job scripts                                                   \
 && echo '#!/bin/bash'                                      > /entrypoint.sh  \
 && echo 'source scl_source enable devtoolset-8'           >> /entrypoint.sh  \
 && echo 'source scl_source enable rh-git218'              >> /entrypoint.sh  \
# don't set before sourcing scl, those require +e                             \
 && echo 'set -euo pipefail'                               >> /entrypoint.sh  \
# start sshd in the background automatically                                  \
# max startups increased to avoid sshd refusing connections with many workers \
 && echo 'sudo /usr/sbin/sshd -o MaxStartups=10000'        >> /entrypoint.sh  \
 && echo 'exec "${@}"'                                     >> /entrypoint.sh  \
 && chmod +x /entrypoint.sh
USER ci-user
# not set by default, but rif ssh strategy wants it
ENV USER=ci-user
ENTRYPOINT ["/entrypoint.sh"]
```
<p style="text-align:center;"><small><i>GPISpace CentOS 7 CI Dockerfile</i></small></p>

The Docker image setting up the DevStack will be based on a basic CentOS 7 image.

```docker
FROM centos:7

...
```

Next, all the dependencies available as operating system packages with the required versions are installed.

```docker
...

RUN yum -y update                                                             \
 && yum -y install centos-release-scl                                         \
                   epel-release                                               \
 && yum -y install asciidoc                                                   \
                   bzip2-devel                                                \
                   ccache                                                     \
                   chrpath                                                    \
                   cmake3                                                     \
                   devtoolset-8-gcc-c++                                       \
                   devtoolset-8-make                                          \
                   glibc-static                                               \
                   hwloc-devel                                                \
# missing hwloc dependency                                                    \
                   libssh2-devel                                              \
                   libtool-ltdl-devel                                         \
# netstat needed for fixed-port test                                          \
                   net-tools                                                  \
                   numactl-devel                                              \
                   openssh-server                                             \
# not automatically pulled in by devel, needed for generating keys            \
                   openssl                                                    \
                   openssl-devel                                              \
                   qt5-qtbase-devel                                           \
                   rh-git218                                                  \
                   source-highlight                                           \
                   sudo                                                       \
# required for building gpi and boost                                         \
                   which                                                      \
                   zlib-devel                                                 \
 && yum -y clean all --enablerepo='*'

...
```

Next all the dependencies not available as packages are compiled and installed, starting with `Boost`.

```docker
...

ENV BOOST_ROOT="/opt/boost"
RUN boost_version=1.61.0                                                      \
 && source scl_source enable devtoolset-8 rh-git218                           \
 && git clone                                                                 \
        --jobs $(nproc)                                                       \
        --depth 1                                                             \
        --shallow-submodules                                                  \
        --recursive                                                           \
        --branch boost-${boost_version}                                       \
        https://github.com/boostorg/boost.git                                 \
        boost                                                                 \
 && cd boost                                                                  \
 && ./bootstrap.sh --prefix=${BOOST_ROOT}                                     \
 && ./b2 -j $(nproc) headers                                                  \
 && ./b2 cxxflags="-fPIC -fno-gnu-unique" cflags="-fPIC -fno-gnu-unique"      \
         -j $(nproc) --without-mpi --without-python link=static install       \
 && grep -lr '# *pragma *message' "${BOOST_ROOT}/include/boost/type_traits"   \
    | xargs sed -i'' -e '/^# *pragma *message.* is deprecated.*/d'            \
 && rm -rf "${PWD}"

...
```

Next, the `GPI-2` source is cloned, build, and installed.

```docker
...

RUN gpi2_version=1.3.1                                                        \
 && source scl_source enable devtoolset-8 rh-git218                           \
 && git clone                                                                 \
        --depth 1                                                             \
        --branch v${gpi2_version}                                             \
        https://github.com/cc-hpc-itwm/GPI-2.git                              \
        GPI-2                                                                 \
 && cd GPI-2                                                                  \
 && grep "^CC\s*=\s*gcc$" . -lR                                               \
    | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'                                   \
 && ./install.sh -p "/usr" --with-fortran=false --with-ethernet               \
 && rm -rf "${PWD}"

...
```

After installing all of _GPISpace_'s dependencies, the environment is setup.
`cmake` is installed previously from a package, which names the binaries `cmake3` and `ctest3`.
For consistency in build scripts, those binaries are made available as `cmake` and `ctest` respectively.
_GPISpace_ requires some tests to run in user space, so a non-root user is created for that purpose.

```docker
...

RUN :                                                                         \
# cmake comes as cmake3. use cmake for consistency in build scripts           \
 && ln -s /usr/bin/cmake3 /usr/bin/cmake                                      \
 && ln -s /usr/bin/ctest3 /usr/bin/ctest                                      \
                                                                              \
# we want to test in user space, so create a user                             \
 && groupadd ci-user                                                          \
 && useradd -r -m -g ci-user ci-user                                          \
 && mkdir -p /home/ci-user/.ssh                                               \
 && ssh-keygen -t rsa -N '' -f /home/ci-user/.ssh/id_rsa                      \
 && cp /home/ci-user/.ssh/id_rsa.pub /home/ci-user/.ssh/authorized_keys       \
 && chown -R ci-user:ci-user /home/ci-user/.ssh/                              \

...
```

Since the non-root user needs to launch an SSH daemon at container startup, host SSH keys are created and the non-root user is getting password-less sudo permissions.

```docker
...

# setup required to start sshd in entrypoint.sh                               \
 && ssh-keygen -A                                                             \
 && echo "%ci-user ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/99-gpispace      \

...
```

Additionally, an entrypoint is defined to perform more environment initialization work not possible within the _Dockerfile_.

```docker
...

 # prolog for ci job scripts                                                   \
 && echo '#!/bin/bash'                                      > /entrypoint.sh  \
 && echo 'source scl_source enable devtoolset-8'           >> /entrypoint.sh  \
 && echo 'source scl_source enable rh-git218'              >> /entrypoint.sh  \
# not set by default, but rif ssh strategy wants it                           \
 && echo 'export USER="$(id -un)"'                         >> /entrypoint.sh  \
# don't set before sourcing scl, those require +e                             \
 && echo 'set -euo pipefail'                               >> /entrypoint.sh  \
# start sshd in the background automatically                                  \
# max startups increased to avoid sshd refusing connections with many workers \
 && echo 'sudo /usr/sbin/sshd -o MaxStartups=10000'        >> /entrypoint.sh  \
 && echo 'exec "${@}"'                                     >> /entrypoint.sh  \
 && chmod +x /entrypoint.sh

...
```

Finally, the `root` user is switched to the new `ci-user` and the entrypoint script from above is set as entrypoint.

```docker
...

USER ci-user
ENTRYPOINT ["/entrypoint.sh"]
```

After having created the Dockerfile, it can be built into a Docker image with the following command.

```bash
docker build -t gspc-centos7-img .
```

<br/>

## Running a Minimal DevStack

With the Docker image available, it is time to put it to use as a DevStack.
The first step is to create a Docker container.
There are two options to realize this task.
The container can be created directly without launching it using `docker create`:

```bash
docker create -i                      \
              --name gspc-centos7-ctr \
              gspc-centos7-img        \
              /bin/bash
```

Alternatively, it can be created and launched immediately using `docker run`:

```bash
docker run -it                     \
           --name gspc-centos7-ctr \
           gspc-centos7-img        \
           /bin/bash
```

> ---
> **Note:**
>
> Exiting the container after launching it with `docker run` will shut it down.
>
> ---

The container can be launched afterwards using the `docker start` command:

```bash
docker start gspc-centos7-ctr
```

It is possible to verify that it is indeed running using `docker ps` and making sure it is listed in the output table.
If the container is launched properly, it is possible to connect to it using the `docker exec` command like so:

```bash
docker exec -it gspc-centos7-ctr /bin/bash
```

> ---
> **Note:**
>
> Only `vi` is available as an editor by default.
> Other editors need to be installed through the package manager or build from source.
>
> ---

After finishing working with the container, it is necessary to explicitly shut it down:

```bash
docker stop gspc-centos7-ctr
```

<br/>

---

| [Home](../../../../../index.md) | [Previous](1_tldr-quickstart.md) | [Up](../devstack.md) | [Next](3_advanced-usage.md) |
| :-: | :-: | :-: | :-: |
