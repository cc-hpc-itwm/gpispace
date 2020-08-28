#!/bin/bash

# function to wait until a specified file exists
function wait_file() {
  local file="$1"; shift
  local cycles="${1:-10}"; shift
  until test $((cycles--)) -eq 0 -o -e "$file"
  do
    sleep 1
  done
  ((++cycles))
}

mkdir -p ${HOME}/.ssh
socket="${HOME}/.ssh/socket"
sudo socat UNIX-LISTEN:${socket},fork                                         \
           UNIX-CONNECT:/.sockets/socket &
# wait to call chown until the file is created
# by socat
wait_file "${HOME}/.ssh/socket" || {
  echo "Socket timeout reached after $? cycles."
  exit 1
}

sudo chown $(id -u):$(id -g) ${HOME}/.ssh/socket
/bin/bash
