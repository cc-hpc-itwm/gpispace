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
