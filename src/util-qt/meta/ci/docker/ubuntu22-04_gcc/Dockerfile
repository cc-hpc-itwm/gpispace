###############################################################################
# Parameters
###############################################################################

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME="ci-user"

###############################################################################
# Base Image
###############################################################################

FROM dockerreg:5000/top/docker-images/base/ubuntu-20.04:latest as base

ARG USER_ID \
    GROUP_ID \
    USER_NAME

ENV BOOST_ROOT="/opt/boost"

# required to prevent tzdata from prompting a timezone input
RUN export DEBIAN_FRONTEND=noninteractive                                     \
 && apt install -y ccache                                                     \
                   chrpath                                                    \
                   cmake                                                      \
                   g++                                                        \
                   make                                                       \
                   qtbase5-dev                                                \
                   qtbase5-dev-tools                                          \
                   qtchooser                                                  \
                   qt5-qmake                                                  \
 && apt clean

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

COPY --from=boost-builder /opt/boost /opt/boost/

RUN groupadd --gid ${GROUP_ID} ${USER_NAME}                                   \
 && useradd -r -m --uid ${USER_ID} --gid ${GROUP_ID} ${USER_NAME}

USER ${USER_NAME}
ENV USER ${USER_NAME}

