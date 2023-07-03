###############################################################################
# Parameters
###############################################################################

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME="ci-user"

###############################################################################
# Base Image
###############################################################################

FROM dockerreg:5000/top/docker-images/base/ubuntu-18.04:latest as base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

ENV BOOST_ROOT="/opt/boost" \
    PATH="/opt/cmake/bin:${PATH}"

# required to prevent tzdata from prompting a timezone input
RUN export DEBIAN_FRONTEND=noninteractive                                     \
 && apt install -y ccache                                                     \
                   chrpath                                                    \
                   g++                                                        \
                   make                                                       \
                   qt5-default                                                \
 && apt clean

###############################################################################
# CMake Builder
###############################################################################

FROM base as cmake-builder

COPY parts/dep-cmake /build/cmake

RUN apt install -y                                                            \
      libssl-dev                                                              \
      wget                                                                    \
 && apt clean                                                                 \
 && chmod +x /build/cmake                                                     \
 && /build/cmake 3.16.9

###############################################################################
# Boost Builder
###############################################################################

FROM base as boost-builder

COPY parts/dep-boost /build/boost

RUN apt install -y                                                            \
      libbz2-dev                                                              \
      wget                                                                    \
 && apt clean                                                                 \
 && chmod +x /build/boost                                                     \
 && /build/boost 1.63.0

 ###############################################################################
# Final Image
###############################################################################

FROM base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

COPY --from=cmake-builder /opt/cmake /opt/cmake/
COPY --from=boost-builder /opt/boost /opt/boost/

RUN groupadd --gid ${GROUP_ID} ${USER_NAME}                                   \
 && useradd -r -m --uid ${USER_ID} --gid ${GROUP_ID} ${USER_NAME}

USER ${USER_NAME}
ENV USER ${USER_NAME}

