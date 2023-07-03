###############################################################################
# Parameters
###############################################################################

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME="ci-user"

###############################################################################
# Base Image
###############################################################################

FROM dockerreg:5000/top/docker-images/base/centos-7:latest as base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

ENV BOOST_ROOT="/opt/boost"

RUN yum -y install centos-release-scl                                         \
                   epel-release                                               \
 && yum -y install ccache                                                     \
                   chrpath                                                    \
                   cmake3                                                     \
                   devtoolset-8-gcc-c++                                       \
                   devtoolset-8-make                                          \
                   qt5-qtbase-devel                                           \
 && yum -y clean all --enablerepo='*'

RUN ln -s /usr/bin/cmake3 /usr/bin/cmake                                      \
 && ln -s /usr/bin/ctest3 /usr/bin/ctest

###############################################################################
# Boost Builder
###############################################################################

FROM base as boost-builder

COPY parts/dep-boost /build/boost

RUN yum -y install                                                            \
      bzip2-devel                                                             \
      wget                                                                    \
      which                                                                   \
 && yum -y clean all --enablerepo='*'                                         \
 && source scl_source enable devtoolset-8                                     \
 && chmod +x /build/boost                                                     \
 && /build/boost 1.63.0

###############################################################################
# Final Image
###############################################################################

FROM base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

COPY --from=boost-builder /opt/boost /opt/boost/

RUN :                                                                         \
# ccache symlinks are not generated automatically                             \
 && ln -s $(which ccache) /usr/lib64/ccache/gcc                               \
 && ln -s $(which ccache) /usr/lib64/ccache/g++                               \
 && ln -s $(which ccache) /usr/lib64/ccache/clang                             \
 && ln -s $(which ccache) /usr/lib64/ccache/clang++                           \
                                                                              \
 && groupadd --gid ${GROUP_ID} ${USER_NAME}                                   \
 && useradd -r -m --uid ${USER_ID} --gid ${GROUP_ID} ${USER_NAME}             \
                                                                              \
# prolog for ci job scripts                                                   \
 && echo '#!/bin/bash'                                      > /entrypoint.sh  \
 && echo 'source scl_source enable devtoolset-8'           >> /entrypoint.sh  \
# don't set before sourcing scl, those require +e                             \
 && echo 'set -euo pipefail'                               >> /entrypoint.sh  \
 && echo 'exec "${@}"'                                     >> /entrypoint.sh  \
 && chmod +x /entrypoint.sh

USER ${USER_NAME}
ENV USER ${USER_NAME}
ENTRYPOINT ["/entrypoint.sh"]

