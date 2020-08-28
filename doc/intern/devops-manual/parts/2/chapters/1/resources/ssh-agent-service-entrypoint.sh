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
