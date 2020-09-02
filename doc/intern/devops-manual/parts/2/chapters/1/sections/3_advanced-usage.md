| [Home](../../../../../index.md) | [Previous](2_getting-started.md) | [Up](../devstack.md) | Next |
| :-: | :-: | :-: | :-: |

---

<br/>

# Advanced Usage

In the previous chapter, only a minimal DevStack setup is described.
Such a DevStack comes however with certain drawbacks and restrictions.

## Table of Contents

1. [Imposing Limits](#imposing-limits)
2. [Container Image Customization](#container-image-customization)
3. [SSH Authentication](#ssh-authentication)
   1. [SSH-Agent Service](#ssh-agent-service)
   2. [GPISpace DevStack Integration](#gpispace-devstack-integration)
4. [Workspace Decoupling](#workspace-decoupling)

---

<br/>

## Imposing Limits

For a lot of Docker applications the default configuration is not a problem.
However, GPISpace tests have some special requirments that can result in very poor performance and errors if not met.
GPISpace tests fork a lot, which can lead to problems if the ulimits for `nofiles` and `nproc` are set to unlimited.
Additionally, virtual memory uses `shm_open()` which on Linux defaults to using `/dev/shm`, which by default Docker only assigns 64MB to.
To make things a little more complicated, setting ulimits in a container requires extra privileges not available in all containers.

The best approach to impose sane values to those ulimits and shm size would be to change or add default values to the Docker daemon configuration file.
The following should be appended at the end of the `daemon.json` file.
Under Linux this file is located under `/etc/docker/daemon.json` and under MacOS and Windows it can be accessed through Docker Desktop's settings menu under Docker Engine.

```json
{
  ...

  "default-shm-size": "64G",
  "default-ulimits": {
    "nofile": {
      "Name": "nofile",
      "Hard": 1024,
      "Soft": 1024
    },
    "nproc": {
      "Name": "nproc",
      "Soft": 65536,
      "Hard": 65536
    }
  }
}
```

The settings can be verified from within the container using `df -kh /dev/shm` for the shm size, `ulimit -n` for `nofile`, and `ulimit -u` for `nproc`.

> ---
> **Note:**
>
> The `default-shm-size` here is the one set for a docker runner (docker06 at the time of writing).
> A user's system most likely does not have 64GB of memory to spare for Docker.
> 8GB are hopefully enough for applications using GPISpace.
>
> ---

If for some reason the user is not able to manipulate the Docker daemon configuration, Docker can also change those problematic values at container creation time.
It suffices to extend the creation command like so (same for `docker run`):

```bash
docker create -i                                                              \
              --name gspc-centos7-ctr                                         \
              --shm-size 64G                                                  \
              --ulimit nofile=1024                                            \
              --ulimit nproc=65536:65536                                      \
              gspc-centos7-img                                                \
              /bin/bash
```

> ---
> **Note:**
>
> If no hard limit is provided the passed value is used for both limits.
> Otherwise the soft and hard limits are colon separated as shown for `nproc`.
>
> ---

<br/>

## Container Image Customization

Customizing the container image brings several advantages to the table.
It allows to install additional software persistently and to exert more control over the original image.
In order to make the GPISpace image better suited as a DevStack, a few things could be customized.

For once, it doesn't provide any kind of text editors (besides `vi`) or other tools except those required for the CI environment.
Developers could install or compile those tools themselves, but they would have to do so after each time the container gets updated.
Installing the tools in a decoupled location will bring some ease, but unless the binaries are linked completely statically, a container update can introduce incompatibilities, in which case a reinstallation or recompilation is required anyway.

Another potential headache is the non-root `ci-user`.
Specifically, a risk of the `UID` and/or `GID` changing after a container update.
Nothing guarantees the same `UID` or `GID` between different container environments, which can lead to permission problems when sharing data between them.
It is of course possible to change file and directory ownerships over and over again, but this becomes tedious really fast.
The best case, would be a user with the same `UID`, `GID`, and `USER` across environments.

> ---
> **Note:**
>
> The same `USER` is not required. The same `UID` and `GID` are sufficient.
>
> ---

In both cases, the best solution without modifying the original Dockerfile or Docker image itself, is to customize the Docker image through extension.
This means to use the GPISpace image as a base for a new Docker image containing the desired changes.
The Dockerfile below demonstrates this for installing the `vim` package and creating a new non-root user.

```docker
FROM gspc-centos7-img:latest

ARG USER_ID=9999
ARG GROUP_ID=9999
ARG USER_NAME=user

# switch to root for the build process
USER root
WORKDIR /

RUN yum -y update                                                             \
 && yum -y install vim                                                        \
 && yum -y clean all --enablerepo='*'

RUN groupadd --gid $GROUP_ID $USER_NAME                                       \
 && useradd -r -m --uid $USER_ID --gid $GROUP_ID $USER_NAME                   \
 && mkdir -p /home/$USER_NAME/.ssh                                            \
 && ssh-keygen -t rsa -N '' -f /home/$USER_NAME/.ssh/id_rsa                   \
 && chown -R $USER_NAME:$USER_NAME /home/user/.ssh/                           \
 && echo "%$USER_NAME ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/99-gpispace

USER $USER_NAME
ENV USER=$USER_NAME
```

The custom GPISpace Dockerfile build command needs to be modified as follows if not using the default values for `UID`, `GID`, and `USER`:

```bash
docker build --build-arg USER_ID=4096                                         \
             --build-arg GROUP_ID=4096                                        \
             --build-arg USER_NAME=dev                                        \
             -f custom-gspc-centos7.dockerfile                                \
             -t custom-gspc-centos7-img                                       \
             .
```

Now, all containers built from the image above will contain `vim` as well as login as the given username with the `UID` and `GID` given at image build time.

> ---
> **Note:**
>
> On Linux systems the best practice is to use the same user as on the host system.
> Simply use the following command instead:
>
> ```bash
> docker build --build-arg USER_ID=$(id -u)                                   \
>              --build-arg GROUP_ID=$(id -g)                                  \
>              --build-arg USER_NAME=$(id -un)                                \
>              -f custom-gspc-centos7.dockerfile                              \
>              -t custom-gspc-centos7-img                                     \
>              .
> ```
>
> ---

<br/>

## SSH Authentication

Another inconvenience with the minimal setup is SSH authentication.
Docker images rarely come packing SSH keys for their available users.
`ci-user` in the GPISpace image does have one, but that's the exception.
However, that SSH key is not registered with any Git host by default.
Even worse, the provided key will be different each time the image is created from the GPISpace Dockerfile.
So even if the SSH key is registered with Git hosts, the keys need to be registered again each time.
But the ugliest is yet to come.
Suppose the Docker image is a super stable long time support image uploaded to a registry, so no need to rebuild the image anytime soon.
As mentioned before, the SSH key is created at image build time.
This means that everyone pulling that image will have the same SSH key with exactly the same password.
The whole situation becomes even more complicated when multiple DevStacks come into play.

### SSH-Agent Service

The easiest way to address this issue would be to bind mount the host's SSH key into the container.
This approach might be the easiest but isn't the most secure either.
Generally, containers should be viewed as untrusted systems, especially those where third parties might have access to (e.g. opensource projects).

Another more secure way, is to run an SSH agent service container, with its own SSH key, sharing the agent's socket with other containers.

First, the service requires a Dockerfile to create the image from.

```docker
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

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["start"]
```

As a base image Alpine Linux is used.
Alpine is a super lightweight Linux distribution available as an official Docker image, making it ideal for running services on top.
The only package required to run a ssh-agent service, besides `openssh`, is `socat` for controlling the socket creation.
The socket directory can be configured at build time with `/.sockets` as default value.

The only thing missing now is the entrypoint script given below:

```bash
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
```

The entrypoint script takes exactly one of two arguments, which decides what part of the script is executed.
`start` will create a socket and launch the `ssh-agent` with it in debug mode preventing it from detaching.
`login` will create a SSH key if non exists and registers it with the running agent.
The public key is always printed on the screen.

The only thing left now is to create the image and create a container from it.
However, before creating the service container, two Docker volumes are required.
One for holding the generated SSH key pair and one for storing the SSH sockets.

```bash
docker volume create ssh-key
docker volume create ssh-sockets
```

Finally, launching the container is a two step process.
First the container is started with the following command:

```bash
docker run --rm                                                               \
           --detach                                                           \
           --name ssh-agent-service                                           \
           --mount source=ssh-key,target=/.key                                \
           --mount source=ssh-sockets,target=/.sockets                        \
           ssh-agent-img
```
> ---
> **Note:**
>
> The command-line output can be verified with the following command:
>
> ```bash
> docker logs ssh-agent-service
> ```
>
> ---

Second the entrypoints login command needs to be executed.
This is done by created a temporary second container with the login command invoked upon.
This is possible as the login command only populates the `ssh-key` Docker volume also in use by the actual service container.

```bash
docker run --rm                                                               \
           -it                                                                \
           --volumes-from ssh-agent-service                                   \
           ssh-agent-img                                                      \
           login
```

### GPISpace DevStack Integration

Now that the `ssh-agent-service` is running, it can be used by other containers.
The only thing necessary to do so are two small modifications to the container creation command.
The `ssh-agent-service`'s socket volume needs to be mounted and the `SSH_AUTH_SOCK` environment variables needs to be set to the correct socket location as shown below.

```bash
docker create -i                                                              \
              --mount source=ssh-sockets,target=/.sockets                     \
              --env SSH_AUTH_SOCK=/.sockets/socket                            \
              --name custom-gspc-centos7-ctr                                         \
              custom-gspc-centos7-img                                         \
              /startup.sh
```

However, when calling `git clone` on the GPISpace repository from within the container, a permission denied error will appear.
What happened is, that the GPISpace container is not running as `root` but as `ci-user` who doesn't have the necessary permissions to access the `ssh-agent` socket.
For the `ssh-agent-service` to work with arbitrary non-root users, the container's entrypoint needs to redirect the SSH socket to one controlled by the user.

The most convenient solution for this is to add this to the GPISpace image through customization, as seen before.
This customization will add an additional script called `startup` to the image, which will be passed to the entrypoint script instead of `/bin/bash`.
The new customized image then looks like this:

```docker
FROM gspc-img:latest

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
 && chown -R $USER_NAME:$USER_NAME /home/user/.ssh/                           \
 && echo "%$USER_NAME ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/99-gpispace

ENV SSH_AUTH_SOCK=/home/user/.ssh/socket
COPY custom-gspc-startup.sh /startup.sh
RUN chmod +x /startup.sh

USER $USER_NAME
ENV USER=$USER_NAME
```

With the new `startup.sh` script defined below:

```bash
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

sudo chown user:user ${HOME}/.ssh/socket
/bin/bash
```

> ---
> **Note:**
>
> The socket redirection commands could also be entered manually on every startup.
>
> ---

After the changes made to the custom GPISpace image, the container creation command from before needs to be modified slightly:

```bash
docker create -i                                                              \
              --mount source=ssh-sockets,target=/.sockets                     \
              --name gspc-centos7-ctr                                         \
              custom-gspc-centos7-img                                         \
              /startup.sh
```

Now when attaching to the running DevStack container, `git clone` can be called without difficulties and without having to enter a password.

<br/>

## Workspace Decoupling

A major drawback of the minimal setup is that the developer's workspace is part of the container.
Some common operations on containers are to update their image or just refresh them.
Both involve the destruction and recreation of the container.
This means that these operations will also destroy any changes previously made to that container.
Those unfortunately also include any data and files created or placed within the container.

The solution is to decouple the workspace from the container.
That way, it exists apart and the container can be updated or refreshed without losing the workspace.
In Docker decoupling is achieved through volumes.
After creating the Docker image and before creating the Docker container, a volume needs to be created:

```bash
docker volume create gspc-volume
```

To initialize the volume the first time around, the following lines have to be added to the custom GPISpace Dockerfile right after creating the user.

```docker
...

# create mount points and assign our user to them
RUN mkdir -p /workspace                                                       \
 && chown -R $USER_NAME:$USER_NAME /workspace

...
```

In order to use the volume, the container creation command needs to be modified like so:

```bash
docker create -i                                                              \
              --mount source=gspc-workspace,target=/workspace                    \
              --mount source=ssh-sockets,target=/.sockets                     \
              --name gspc-centos7-ctr                                         \
              custom-gspc-centos7-img                                         \
              /startup.sh
```

A common use case is to test the same codebase with multiple environments.
In that case, it is advisable to keep separate build directories inside the workspace volume.
Alternatively, each build environment could have its own build volume.
The latter approach allows to have reusable scripts between build environments.

**Example:**
```bash
docker volume create gspc-volume
docker volume create gspc-centos7-build
docker volume create gspc-ubuntu20.04-build

docker create -i                                                              \
              --mount source=gspc-volume,target=/workspace                    \
              --mount source=gspc-centos7-build,target=/build                 \
              --mount source=ssh-sockets,target=/.sockets                     \
              --name gspc-centos7-ctr                                         \
              custom-gspc-centos7-img                                         \
              /startup.sh
docker create -i                                                              \
              --mount source=gspc-volume,target=/workspace                    \
              --mount source=gspc-ubuntu20.04-build,target=/build             \
              --mount source=ssh-sockets,target=/.sockets                     \
              --name custom-gspc-ubuntu20.04-ctr                              \
              custom-gspc-ubuntu20.04-img                                     \
              /startup.sh
```

In the example above, build and test scripts could be the same for both containers as there is no need to distinguish between build directories.

> ---
> **Note:**
>
> If the container is recreated all volumes need to be mounted again.
>
> ---

<br/>

---

| [Home](../../../../../index.md) | [Previous](2_getting-started.md) | [Up](../devstack.md) | Next |
| :-: | :-: | :-: | :-: |
