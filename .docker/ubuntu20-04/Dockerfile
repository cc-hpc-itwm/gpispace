FROM ubuntu:20.04

# required to prevent tzdata from prompting a timezone input
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update -y                                                             \
 && apt upgrade -y                                                            \
 && apt install -y cmake                                                      \
                   libbz2-dev                                                 \
                   chrpath                                                    \
                   ccache                                                     \
                   g++                                                        \
                   libhwloc-dev                                               \
                   net-tools                                                  \
                   # includes openssl (aka libssl)                            \
                   openssh-server                                             \
                   libssl-dev                                                 \
                   qt5-default                                                \
                   zlib1g-dev                                                 \
                   git                                                        \
                   sudo                                                       \
                   gawk                                                       \
                   pkg-config                                                 \
                   # missing hwloc dependency                                 \
                   libudev-dev                                                \
                   # included in one of the above                             \
                   # libnuma-dev                                              \
 && apt clean

# workaround: libssh2 package on debian based systems is built with gcrypt
#             instead of openssl.
#             older libssh2 can't read the new openssh key format, though
#             that can also be solved by generating ssh-keys explicitly with
#             the old key format
RUN libssh2_version=1.9.0                                                     \
 && git clone                                                                 \
        --jobs $(nproc)                                                       \
        --depth 1                                                             \
        --shallow-submodules                                                  \
        --recursive                                                           \
        --branch libssh2-${libssh2_version}                                   \
        https://github.com/libssh2/libssh2.git                                \
        libssh2                                                               \
 && cd libssh2                                                                \
 && cmake -D CRYPTO_BACKEND=OpenSSL                                           \
          -D CMAKE_BUILD_TYPE=Release                                         \
          -D ENABLE_ZLIB_COMPRESSION=ON                                       \
          -B build                                                            \
          -S .                                                                \
 && cmake --build build --target install                                      \
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

ENV PKG_CONFIG_PATH="/usr/lib64/pkgconfig"
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
 && echo 'set -euo pipefail'                               >> /entrypoint.sh  \
# start sshd in the background automatically                                  \
# max startups increased to avoid sshd refusing connections with many workers \
# the /run/sshd directory is required for launching sshd manually under       \
# ubuntu. If it is not present the command exits with:                        \
#   Missing privilege separation directory: /run/sshd                         \
 && echo 'sudo mkdir -p /run/sshd'                         >> /entrypoint.sh  \
 && echo 'sudo /usr/sbin/sshd -o MaxStartups=10000'        >> /entrypoint.sh  \
 && echo 'exec "${@}"'                                     >> /entrypoint.sh  \
 && chmod +x /entrypoint.sh
USER ci-user
# not set by default, but rif ssh strategy wants it                           \
ENV USER ci-user
ENTRYPOINT ["/entrypoint.sh"]
