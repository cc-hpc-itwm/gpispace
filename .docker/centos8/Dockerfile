FROM centos:8

RUN yum -y update                                                             \
 && yum -y install epel-release                                               \
# activate the PowerTools repo for some devel packages                        \
 && sed -i /etc/yum.repos.d/CentOS-PowerTools.repo                            \
        -e 's/enabled=0/enabled=1/g'                                          \
 && yum -y install bzip2-devel                                                \
                   ccache                                                     \
                   chrpath                                                    \
# gcc-c++ 9.2.1 is an option with the gcc-toolset-9-gcc-c++ package           \
# the default package installs 8.3.1                                          \
                   gcc-c++                                                    \
                   make                                                       \
                   glibc-static                                               \
                   hwloc-devel                                                \
# missing hwloc dependency                                                    \
                   libssh2-devel                                              \
                   libtool-ltdl-devel                                         \
# netstat needed for fixed-port test                                          \
                   net-tools                                                  \
                   numactl-devel                                              \
                   openssh-server                                             \
                   qt5-qtbase-devel                                           \
                   git                                                        \
                   sudo                                                       \
# required for building gpi and boost                                         \
                   which                                                      \
 && yum -y clean all --enablerepo='*'

RUN cmake_version=3.16.9                                                      \
 && git clone                                                                 \
        --jobs $(nproc)                                                       \
        --depth 1                                                             \
        --shallow-submodules                                                  \
        --recursive                                                           \
        --branch tags/v${cmake_version}                                       \
        https://github.com/Kitware/CMake.git                                  \
        cmake                                                                 \
 && cd cmake                                                                  \
 && ./bootstrap --prefix=/usr                                                 \
 && make -j $(nproc) install                                                  \
 && rm -rf "${PWD}"

ENV BOOST_ROOT="/opt/boost"
RUN boost_version=1.63.0                                                      \
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
# don't set before sourcing scl, those require +e                             \
 && echo 'set -euo pipefail'                               >> /entrypoint.sh  \
# start sshd in the background automatically                                  \
# max startups increased to avoid sshd refusing connections with many workers \
 && echo 'sudo /usr/sbin/sshd -o MaxStartups=10000'        >> /entrypoint.sh  \
 && echo 'exec "${@}"'                                     >> /entrypoint.sh  \
 && chmod +x /entrypoint.sh
USER ci-user
# not set by default, but rif ssh strategy wants it                           \
ENV USER ci-user
ENTRYPOINT ["/entrypoint.sh"]
