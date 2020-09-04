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
