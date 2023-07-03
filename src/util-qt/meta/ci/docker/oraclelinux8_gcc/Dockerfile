###############################################################################
# Parameters
###############################################################################

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME="ci-user"

###############################################################################
# Base Image
###############################################################################

FROM dockerreg:5000/top/docker-images/base/oraclelinux-8:latest as base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

ENV BOOST_ROOT="/opt/boost"

RUN yum-config-manager --add-repo http://yum.oracle.com/repo/OracleLinux/OL8/developer/EPEL/x86_64 \
 && yum-config-manager --add-repo http://yum.oracle.com/repo/OracleLinux/OL8/codeready/builder/x86_64 \
 && yum -y install ccache                                                     \
                   chrpath                                                    \
                   cmake                                                      \
                   gcc-c++                                                    \
                   make                                                       \
                   qt5-qtbase-devel                                           \
                   tar                                                        \
 && yum -y clean all --enablerepo='*'

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

RUN groupadd --gid ${GROUP_ID} ${USER_NAME}                                   \
 && useradd -r -m --uid ${USER_ID} --gid ${GROUP_ID} ${USER_NAME}

USER ${USER_NAME}
ENV USER ${USER_NAME}

