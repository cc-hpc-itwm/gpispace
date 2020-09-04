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
