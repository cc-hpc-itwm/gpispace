| [Home](../../../../../index.md) | [Previous](1_definitions.md) | [Up](../containerization.md) | Next |
| :-: | :-: | :-: | :-: |

---

<br/>

# Docker

Docker is available for the three major operating systems Windows, Linux, and MacOS.
It is by far one of the most popular container engines and spear headed the standardization of container technology.
Most other container engines include support for Docker's definition files and/or images (e.g. Singularity).

## Table of Contents

1. [Requirements](#requirements)
2. [Query Commands](#query-commands)
3. [Dockerfiles](#dockerfiles)
4. [Images](#images)
5. [Application Containers](#application-containers)
6. [Interactive Containers](#interactive-containers)
7. [Volumes](#volumes)
8. [Docker-out-of-Docker](#docker-out-of-docker)

<br/>

---

## Requirements

> ---
> **Note:**
>
> Only 64-bit operating systems are supported
>
> ---

| | Linux | MS Windows | MacOS |
| :-: | :-- | :-- | :-- |
| Version | CentOS 7<br/>CentOS 8<br/>Debian Stretch 9<br/>Raspbian Stretch<br/>Debian Buster 10 (stable)<br/>Fedora 30<br/>Fedora 31<br/>Ubuntu Xenial 16.04 (LTS)<br/>Ubuntu Bionic 18.04 (LTS)<br/>Ubuntu Eoan 19.10<br/>Ubuntu Focal 20.04 (LTS)<br/>Binary Installation | 10 Professional<br/>10 Enterprise | 10.13 High Sierra<br/>10.14 Mojave<br/>10.15 Catalina |
| Hardware | | CPU with SLAT | 2010 model or newer with an Intel chipset supporting MMU virtualization, EPT, and Unrestricted Mode. |
| Minimum RAM | 4 GB | 4 GB | 4 GB |

## Query Commands

| Command | Description |
| :-: | :-- |
| `docker ps` | Shows running containers. Works the same way as the `ps` command on Unix systems. |
| `docker image` | Manipulate images stored in the local `Docker` engine with subcommands (e.g. `docker image ls`, `docker image rm <image name>`, ...). |
| `docker container` | Manipulate containers stored in the local `Docker` engine with subcommands (e.g. `docker container ls`, `docker container rm <container name>`, ...). |
| `docker volume` | Manipulate volumes stored in the local `Docker` engine with subcommands (e.g. `docker volume ls`, `docker volume rm <volume name>`, ...). |

## Dockerfiles

Dockerfiles are the definition files used by docker.
By default, Docker is looking for a file named `Dockerfile`, but they are not bound to that filename.
Dockerfiles with custom names usually carry the file extension `.dockerfile` which is also recognized by most editors for syntax highlighting.

The following shows an example of a Dockerfile, named `Dockerfile`, to create a very basic C/C++ developer image, which will be referred to as `dev-base` later on:

```docker
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt upgrade -y \
    && apt install -y \
        build-essential \
        cmake \
        git
```
<p style="text-align:center;"><small><i>dev-base Dockerfile</i></small></p>

The `FROM` statement is the basis of each Dockerfile and defines the starting image to use.
This image can be a minimal OS image, a more complex image, and even a user-defined image.

The `RUN` statement can execute any terminal command the previously specified OS image has installed.
By default, they are run as the root user.
`RUN` statements are only used for the purpose of creating an image.
They can't be used as container startup commands or for setting up environment variables.
In fact, a variable exported within a `RUN` statement will not persist in the next command.

Environment variables can be set during image creation with `ENV` and the `ARG` statements, with the latter ones only being available during image creation.

External files and directories can be copied into the Docker image using the `COPY` statement.

A Dockerfile can also define applications and their parameters to be executed at container startup with the `ENTRYPOINT` and `CMD` statements.
`ENTRYPOINT` statements are set in stone and a container is required to execute them (there are workarounds).
However, `CMD` statements can be modified during the `docker` executable call.
There can only be one statement of each, where multiple occurences override each other.
`ENTRYPOINT` has a higher priority than `CMD`.
However, a `CMD` statement following an `ENTRYPOINT` statement is combined with it as input parameters forming a single command.

> ---
> **Note:**
>
> Each statement in a `Dockerfile` creates a new cached intermediate image, which the next statement is based upon.
> If a statement is modified, all subsequent statements need to be re-evaluated, causing long build times.
> In a production `Dockerfile`, the number of statements should be reduced to a minimum for cache size reduction (i.e. poorly written large `Dockerfiles` can incur a cache of several GB).
>
> The `COPY` statement is an exception to this and will always be evaluated.
> Hence, `COPY` statements are best placed as far at the end of a `Dockerfile` as possible for optimal cache utilisation.
>
> ---

The following shows a little example of a Dockerfile named `hello.dockerfile` and some external files to create an executable image to print out a greeting to an optional name.
As a starting point, the `dev-base` image created from the `Dockerfile` above is used.
The two external files `hello.cpp` and `CMakeLists.txt` are copied over into the image.
The source code is then configured, compiled, and installed.
At this point we no longer need the copied files and the build directory and we can delete them again from the final image.
The `ENTRYPOINT` defines the application to execute upon a container created from this image and `CMD` contains its default input parameter.

```c++
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "'hello' requires exactly one argument" << std::endl;
        return 1;
    }

    std::string name(argv[1]);
    std::cout << "Hello " << name << "!!!" << std::endl;

    return 0;
}
```
<p style="text-align:center;"><small><i>hello.cpp</i></small></p>

```cmake
cmake_minimum_required(VERSION 3.0)
project(hello)

add_executable(${PROJECT_NAME} hello.cpp)
install(TARGETS ${PROJECT_NAME})
```
<p style="text-align:center;"><small><i>CMakeLists.txt</i></small></p>

```docker
FROM dev-base

COPY hello.cpp .
COPY CMakeLists.txt .

RUN cmake -DCMAKE_BUILD_TYPE:STRING=Release -B build -S . \
    && cmake --build build -- -j $(nproc) \
    && cmake --install build \
    && rm -rf build CMakeLists.txt hello.cpp

ENTRYPOINT ["hello"]
CMD ["World"]
```
<p style="text-align:center;"><small><i>hello.dockerfile</i></small></p>

> ---
> **Note:**
>
> A container will exit as soon as its `ENTRYPOINT` or `CMD` finish or exit.
> This is also the case for a program detaching from the terminal.
>
> ---

## Images

Creating a Docker image from a Dockerfile and all its dependencies (i.e. local files), a simple command line call is enough in most cases.

The following little example takes the `dev-base` Dockerfile defined above and builds a Docker image from it.

```bash
docker build -t dev-base .
```

The command tells the Docker engine to perform the build command on the `Dockerfile` in the current directory and name the resulting image `dev-base`.
The final argument specifies the working directory for local files, in this case the current directory.
The target name is actually optional for this command.
The Docker engine will automatically assign a randomly generated name to images by default.

This second example shows the `docker build` command used to create the `hello` Docker image from above which doesn't use the default `Dockerfile` name.

```bash
docker build -f hello.dockerfile -t hello .
```

The only change from the previous command is the addition of the `-f hello.dockerfile` option.

> ---
> **Warning:**
>
>The `-f <path/dockerfile>` command line option takes the entire relative or absolute path to the desired Docker definition file.
> The `.` at the end of the command defines the working directory of the `docker build` command.
> However, this working directory doesn't apply to the `Dockerfile` and a full path is still required in `-f <path/dockerfile>`.
>
> Assume the `hello.dockerfile` is located in a subdirectory called `deffiles`, then the command on the left is the correct one to use.
>
> <p style="text-align:center;"><small><b>CORRECT</b></small></p>
>
> ```bash
> docker build -f deffiles/hello.dockerfile -t hello .
> ```
>
> <p style="text-align:center;"><small><b>WRONG</b></small></p>
>
> ```bash
> docker build -f hello.dockerfile -t hello deffiles
> ```
>
> ---

## Application Containers

Creating a container and executing its contained application is done in a single easy command.
The following shows two executions of the hello image.
The first one uses the default parameter and outputs `"Hello World!!!`.
The second one overwrites the default input and outputs `"Hello John!!!`.

```bash
docker run hello
docker run hello John
```

> ---
> **Note:**
>
> Each execution will create and run a new container, which depending on the image can quickly take up a lot of memory.
> In order to prevent this, the run command can be called with the `--rm` option to remove a container immediately after finishing the application.
>
> ```bash
> docker run --rm hello
> ```
>
> ---

## Interactive Containers

Docker containers can also be used interactively similar to a virtual machine.
This mode of operation is extremely useful for testing and developing software in different environments without the bloat of a real virtual machine.
Same as for images, containers will get random names assigned to them if no other name is given using the `--name` parameter.

The following shows how to use the previously created `dev-base` image as an interactive development container.
The easiest way to achieve this is by running the image with the `-i | --interactive` and the `-t | --tty` flags:

```bash
docker run --name dev-base-container -it dev-base
```

This will create an interactive container and automatically connect to its TTY.
From there on, it can mostly be used like a regular machine (most images don't have a working systemd).
However, when exiting the container it is also shutdown and needs to be restarted the next time around.
For starting it up again, the following command needs to be used as `docker run` will also create a container.

```bash
docker start -i dev-base-container
```

The container will launch `/bin/bash` and will shutdown immediately without the `-i | --interactive` flag.

Container names need to be unique, alas to update a container with a new image it needs to be stopped, deleted, created again, and in most cases relaunched.

```bash
docker stop <container-name>
docker container rm <container-name>
docker run <container-options> --name <container-name> <new-image>
```

> ---
> **Note:**
>
> A container can also be created without launching it using the following command.
>
> ```bash
> docker create --name dev-base-container -it dev-base
> ```
>
> It is also possible to pass command-line arguments at the end of the command, which will subsequently be passed during every call to `docker start`.
>
> ```bash
> docker create --name <container-name> -it <image-name> <command> <arguments>
> ```
>
> ---

Having the container shutdown immediately once the user exits the TTY is inconvenient in cases where other services depend on it.
If this behavior is desired, first a container needs to be created and then started.
The only difference to the previous commands, is that the `-t | --tty` flag is not required anymore.

```bash
docker create --name dev-base-container -i dev-base
docker start dev-base-container
```

> ---
> **Note:**
>
> Entrypoints are executed as well, when calling `docker start`.
> However, for the container to remain open the last command needs to be a blocking one.
> Most commonly, a shell like bash is called as the last command in the entrypoint.
>
> ---

Now that the container is running, it is possible to connect to it by calling bash interactively on it.

```bash
docker exec -it dev-base-container bash
```

> ---
> **Note:**
>
> Other commands can also be executed on the container without the container shutting down upon finishing it.
>
> Example:
> ```bash
> docker exec dev-base-container echo "Hello World!!!"
> ```
>
> ---

## Volumes

Interactive containers created like above still have a very inconvenient issue.
Every development done in such a stack is stored within the container.
Meaning, if the `Dockerfile` gets updated and the container requires a rebuilt, all development will be lost.
Volumes are meant to separate data from their containers and also allow for passing data between containers.

Creating a volume is straight forward using a single command.

```bash
docker volume create dev-base-volume
```

A volume needs to be attached to a container at creation time using the `--mount` flag.
The `--mount` flag takes a comma separated parameter list.

| Parameter | Description |
| :-: | :-- |
| `source` | Name of the volume to use or location on the host system to bind. |
| `target` | Location to mount the volume to in the container. |
| `type` | Type of the mount, which can be `bind`, `volume`, `tmpfs`. The default is `volume`. |
| `readonly` | Mounts the source as readonly storage. |

The command below mounts the volume created above into a container using the `dev-base` image into the directory `/app`.
Now the `dev-base` container can be updated without loosing any development progress made in `/app`.

```bash
docker create --name dev-base-container --mount source=dev-base-volume,target=/app -i dev-base
```

The mount target location doesn't need to exist within the container.
It will be created automatically by the root user if it doesn't.
On the other hand, if the mount target already exists in the container one of two scenarios will apply.

* A new volume will be initialized by the mount target, adopting the target locations content and permissions.
* An already initialized volume will obscure the mount target's content and permissions with the volume's.

The container with the mounted volume can be started and interacted with in the same way as the previous containers.

> ---
> **Note:**
>
> Since volumes are independent of their container, they can be populated by a different container than the one using it in the end.
> For example, a container could clone a git repository containing a `Dockerfile`, build an image of it, and finally create a container from that image mounting the created volume.
>
> ---

## Docker-out-of-Docker

Some use cases require the Docker application to build images or launch containers.
Most commonly in Continuouse Integration or Development Stack scenarios, where the original application needs to execute another program in a variety of different environments.
Fortunately, Docker containers have the ability to run Docker commands as long as a Docker installation is available.
There are two possibilities of doing this:

1. Docker in Docker (DinD)
2. Docker out of Docker (DooD)

Both of these approaches bring their own advantages and disadvantages, however the first option is mostly discouraged due to setup difficulties and nasty side-effects (https://jpetazzo.github.io/2015/09/03/do-not-use-docker-in-docker-for-ci/).
That being said, the second option also has its fair share of drawbacks and concerns, but can be set up faster and more reliably.
Therefore it is recommended to use `DooD` over `DinD` in the vast majority of cases.
However, for both it is strongly discouraged to fire up any untrusted containers.

The `DooD` approach shares the host's Docker socket with its child container.
If that child container launches another container, the new container will actually be launched by the host making it a sibling of the calling container.
Sharing the Docker socket also allows bi-directional access of images, containers, and volumes created by the host and containers.
In order to access the Docker daemon, the container needs to use either a Docker CLI installation or directly the Docker API.

To share the Docker socket, the `run` or `create` command needs to be modified like so:

```bash
docker run -it \
    --mount source=/var/run/docker.sock,target=/var/run/docker.sock,type=bind \
    docker:19.03
```

> ---
> **Note:**
>
> The host-side path is the same under Windows.
>
> ---

> ---
> **Warning:**
>
> When using the Docker image, always specify the version.
> Do not use `docker:latest` as it could cause compatibility issues.
>
> ---

<br/>

---

| [Home](../../../../../index.md) | [Previous](1_definitions.md) | [Up](../containerization.md) | Next |
| :-: | :-: | :-: | :-: |
