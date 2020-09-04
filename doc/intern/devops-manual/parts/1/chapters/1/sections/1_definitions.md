| [Home](../../../../../index.md) | [Previous](../containerization.md) | [Up](../containerization.md) | [Next](2_docker.md) |
| :-: | :-: | :-: | :-: |

---

<br/>

# Definitions

| Term | Definition |
| :-: | :-- |
| Definition File | Text-based file containing the instructions to build a container image. |
| Image | An image is built from a definition file and is a read-only template used to construct containers. Images are used to store and deploy applications and their environment settings. They are comparable to read-only storage media (e.g. floppy disk, CD-ROM, DVD-ROM, ...). |
| Container | A container is a sandboxed environment built from an image that runs applications. It can be used both as an executable or a lightweight virtual machine. Containers are managed by a container engine. |
| Volume |  A volume is a way for having container independent, shareable, persistent storage. Volumes can be created and populated independently of the containers using them (through other containers though). Rebuilding images and/or containers doesn't affect volumes. |
| Container Engine | Container engines are operating-system-level virtualization systems. They allow the existence of multiple isolated instances (i.e. containers). Container engines, also referred to as operating-system-level virtualization, is an operating system in which allows the existence of multiple isolated instances (i.e. containers). They can take definition files and build them into images, which in turn can be used to create containers (e.g. Docker, Singularity, ...). |
| Registry | A registry is a repository for storing images. Container engines can connect to registries to download (pull) and upload (push) images (e.g. Docker Hub, Singularity Hub). A registry is not required to be publicly accessible. |

<br/>

---

| [Home](../../../../../index.md) | [Previous](../containerization.md) | [Up](../containerization.md) | [Next](2_docker.md) |
| :-: | :-: | :-: | :-: |
