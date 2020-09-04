| [Home](../../../../../index.md) | [Previous](../devstack.md) | [Up](../devstack.md) | [Next](2_getting-started.md) |
| :-: | :-: | :-: | :-: |

---

<br/>

# TLDR Quickstart

## Table of Contents

1. [Resources](#resources)
   1. [Dockerfiles](#dockerfiles)
   2. [Shell Scripts](#shell-scripts)
2. [Instructions](#instructions)
   1. [Initial Setup](#initial-setup)
   2. [Startup](#startup)
   3. [Initial GPISpace Setup](#initial-gpispace-setup)
   4. [Shutdown](#shutdown)

---

<br/>

## Resources

### Dockerfiles

[_gspc-centos7.dockerfile_](../resources/gspc-centos7.dockerfile):

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

[_custom-gspc-centos7.dockerfile_](../resources/custom-gspc-centos7.dockerfile):

```docker
FROM gspc-centos7-img:latest

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME=user

USER root
WORKDIR /

RUN yum -y update                                                             \
 && yum -y install vim                                                        \
                   socat                                                      \
 && yum -y clean all --enablerepo='*'

RUN groupadd --gid $GROUP_ID $USER_NAME                                       \
 && useradd -r -m --uid $USER_ID --gid $GROUP_ID $USER_NAME                   \
 && mkdir -p /home/$USER_NAME/.ssh                                            \
 && ssh-keygen -t rsa -N '' -f /home/$USER_NAME/.ssh/id_rsa                   \
 && chown -R $USER_NAME:$USER_NAME /home/$USER_NAME/.ssh/                     \
 && echo "%$USER_NAME ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/99-gpispace

# create mount points and assign our user to them
RUN mkdir -p /workspace                                                       \
 && chown -R $USER_NAME:$USER_NAME /workspace                                 \
 && mkdir -p /build                                                           \
 && chown -R $USER_NAME:$USER_NAME /build

ENV SSH_AUTH_SOCK=/home/$USER_NAME/.ssh/socket
COPY custom-gspc-startup.sh /startup.sh
RUN chmod +x /startup.sh

USER $USER_NAME
ENV USER=$USER_NAME
```

[_ssh-agent-service.dockerfile_](../resources/ssh-agent-service.dockerfile):

```docker
FROM alpine:3.12

USER root
WORKDIR /

ARG SOCKET_DIR=/.sockets
ENV SSH_SOCKS=${SOCKET_DIR}                                                   \
    SSH_AUTH_SOCK=${SOCKET_DIR}/socket                                        \
    SSH_AUTH_PROXY_SOCK=${SOCKET_DIR}/proxy-socket

RUN apk add --update --no-cache                                               \
      openssh                                                                 \
      socat                                                                   \
    && rm -rf /var/cache/apk/*

COPY ssh-agent-service-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["start"]
```

### Shell Scripts

[_custom-gspc-startup.sh_](../resources/custom-gspc-startup.sh):

```bash
#!/bin/bash

# function to wait until a specified file exists
function wait_file() {
  local file="$1"; shift
  local cycles="${1:-10}"; shift
  until test $((cycles--)) -eq 0 -o -e "$file"
  do
    sleep 1
  done
  ((++cycles))
}

mkdir -p ${HOME}/.ssh
socket="${HOME}/.ssh/socket"
sudo socat UNIX-LISTEN:${socket},fork                                         \
           UNIX-CONNECT:/.sockets/socket &
# wait to call chown until the file is created
# by socat
wait_file "${HOME}/.ssh/socket" || {
  echo "Socket timeout reached after $? cycles."
  exit 1
}

sudo chown $(id -u):$(id -g) ${HOME}/.ssh/socket
/bin/bash
```

[_ssh-agent-service-entrypoint.sh_](../resources/ssh-agent-service-entrypoint.sh):

```bash
#!/bin/sh

case ${1} in

  start)
    # launch ssh-agent
    mkdir -p ${SSH_SOCKS}
    # clean old sockets
    echo "creating socket ..."
    rm ${SSH_AUTH_SOCK} ${SSH_AUTH_PROXY_SOCK} > /dev/null 2>&1
    socat UNIX-LISTEN:${SSH_AUTH_PROXY_SOCK},perm=0666,fork                   \
          UNIX-CONNECT:${SSH_AUTH_SOCK} &
    echo
    echo "launching ssh-agent ..."
    exec ssh-agent -a ${SSH_AUTH_SOCK} -d
    ;;

  login)
    # init ssh key if none exists
    if [ ! -f "/.key/id_rsa" ] && [ ! -f "/.key/id_rsa.pub" ]
    then
      ssh-keygen                                                              \
        -t rsa                                                                \
        -b 4096                                                               \
        -C "SSH Auth"                                                         \
        -f /.key/id_rsa
    fi

    # output public SSH key
    echo
    echo "---- Public SSH-Key ----"
    cat /.key/id_rsa.pub

    # register key with ssh-agent
    echo
    echo "adding SSH-Key ..."
    ssh-add /.key/id_rsa
    ;;

    *)
    echo "Unrecognized command."
    exit 1
esac

exit 0
```

<br/>

## Instructions

### Initial Setup

1. Build the Docker images:

   ```bash
   docker build                                                               \
     -f gspc-centos7.dockerfile                                               \
     -t gspc-centos7-img                                                      \
     .
   ```

   On Linux systems prefer to use `USER_ID=$(id -u)`, `GROUP_ID=$(id -g)`, and `USER_NAME=$(id -un)` in the following command.

   ```bash
   docker build                                                               \
     --build-arg USER_ID=4096                                                 \
     --build-arg GROUP_ID=4096                                                \
     --build-arg USER_NAME=dev                                                \
     -f custom-gspc-centos7.dockerfile                                        \
     -t custom-gspc-centos7-img                                               \
     .
   ```

   ```bash
   docker build                                                               \
     -f ssh-agent-service.dockerfile                                          \
     -t ssh-agent-service-img                                                 \
     .
   ```

2. Create volumes:

   ```bash
   docker volume create ssh-key
   docker volume create ssh-sockets

   docker volume create gspc-workspace
   docker volume create gspc-build
   ```

3. Create the GPISpace DevStack container:

   ```bash
   docker create                                                              \
     -i                                                                       \
     --name custom-gspc-centos7-ctr                                           \
     --shm-size 8G                                                            \
     --ulimit nofile=1024                                                     \
     --ulimit nproc=65536                                                     \
     --mount source=ssh-sockets,target=/.sockets                              \
     --mount source=gspc-workspace,target=/workspace                          \
     --mount source=gspc-build,target=/build                                  \
     custom-gspc-centos7-img                                                  \
     /startup.sh
   ```

### Startup

Steps 1 and 2 are not required if already executed for another DevStack startup.
The `ssh-agent-service` is shared between all DevStacks.

1. Start `ssh-agent-service`:

   ```bash
   docker run --rm                                                            \
              --detach                                                        \
              --name ssh-agent-service                                        \
              --mount source=ssh-key,target=/.key                             \
              --mount source=ssh-sockets,target=/.sockets                     \
              ssh-agent-service-img
   ```

2. Login to `ssh-agent-service`:

   ```bash
   docker run --rm                                                            \
              -it                                                             \
              --volumes-from ssh-agent-service                                \
              ssh-agent-service-img                                           \
              login
   ```

   Follow the on-screen instructions.
   The public SSH key will be displayed after each login.

   > ---
   > **WARNING:**
   >
   > The SSH key needs to be registered with the GitLab user in order to work.
   > ([Adding an SSH key to your GitLab account](https://docs.gitlab.com/ee/ssh/README.html#adding-an-ssh-key-to-your-gitlab-account))
   >
   > ---

3. Start the DevStack:

   ```bash
   docker start custom-gspc-centos7-ctr
   ```

4. Access the DevStack:

   ```bash
   docker exec -it custom-gspc-centos7-ctr /bin/bash
   ```

   > ---
   > **Tip:**
   >
   > This command can be repeated multiple times, each opening an independent terminal session.
   >
   > ---

   > ---
   > **Note:**
   >
   > Exiting the current terminal session will not shutdown the container.
   >
   > ---

5. Load environment:

   ```bash
   source scl_source enable devtoolset-8 rh-git218
   ```

### Initial GPISpace Setup

1. Clone the GPISpace repository:

   ```bash
   git clone                                                                   \
       --jobs $(nproc)                                                         \
       --recurse-submodules                                                    \
       --branch develop                                                        \
       git@gitlab.hpc.devnet.itwm.fhg.de:top/gpispace.git                      \
       /workspace/gpispace
   ```

2. Configure the CMake build:

   ```bash
   cmake -C /workspace/gpispace/.ci/env/centos7-devtoolset8.cmake             \
         -B /build/gpispace                                                   \
         -S /workspace/gpispace
   ```

3. Compile and install:

   ```bash
   cmake --build /build/gpispace                                              \
         -j $(nproc)

   cmake --build /build/gpispace                                              \
         --target install >/dev/null
   ```

### Shutdown

1. Shutdown DevStack:

    ```bash
    docker stop custom-gspc-centos7-ctr
    ```

2. Shutdown the `ssh-agent-service`:

   ```bash
   docker stop ssh-agent-service
   ```

   > ---
   > **WARNING:**
   >
   > The `ssh-agent-service` is a shared resource and can be used by multiple DevStacks.
   > Before shutting it down, make sure that no other container is actively using it.
   >
   > ---

<br/>

---

| [Home](../../../../../index.md) | [Previous](../devstack.md) | [Up](../devstack.md) | [Next](2_getting-started.md) |
| :-: | :-: | :-: | :-: |
