Topology description:
=====================

## What is a "worker"?

GPI-Space executes application codes with the help of a little program, the "worker" that runs on all machines and can be dedicated to the execution of certain types of tasks/activities requiring certain types of "capabilities". The topology description is used by the bootstrapping mechanism to start workers with such "capabilities" before executing the application. The execution environment manages the workers and their capabilities and the scheduler takes the capabilities into account when tasks are assigned to resources (to their representative worker).

## What is a "topology"?

The "topology" describes what kind of workers and how many workers are managed and how they integrate themselves into the execution environment.

EXAMPLE: Look at an application that decomposes into "compute" and "reduce" tasks and wants to "load" and to "store" data. The computations come in two flavors, one for "CPU" and one for "GPU". Further assume that the reduce and the I/O tasks are not using (much) compute resources and can share resources with the computation tasks. Then the topology might specify 5 types of workers, namely "[compute, CPU]", "[compute, GPU]", "[reduce]", "[IO, load]", "[IO, store]". The topology typically starts one worker per available resource.

For every kind of worker the parameters are:

- A set of capabilities that is represented by the workers of that type. The transitions in application Petri-nets may "require" capabilities which implies that topology descriptions are, in general, not independent from the Petri-nets.
- The number of worker instances that is started on each machine.
- A maximum number of machines where workers of that type are started. The value 0 is used to start workers of that type on all machines. It is unspecified which machines are used to start the workers.
- The amount of working memory in bytes that is attached to each instance of workers of that type. The "working memory" is the memory that is used to cache data that is stored in the independent memory layer (IML). Workers without working memory can neither read data from the IML nor store data in the IML.
- Optional: The number of the NUMA socket to pin the worker. HINT: Among others `lstopo` can be used to determine hardware topology.
- Optional: The base network port that is used by the workers to communicate with the execution environment. The port is incremented for each instance, e.g.  with "base_port = 9876" and the number of workers being equal to 4, the used ports are "{9876, 9877, 9878, 9879}".

## How to specify a topology?

Worker descriptions can be specified using a textual representation with the syntax:

```
WorkerDescription -> Capabilities Socket? Instances?
Capabilities      -> Capability | Capability '+' Capabilities
Capability        -> C-identifier
Socket            -> '#' Number
Instances         -> ':' PerNode MaxNodes? Shm? Port?
PerNode           -> Num
MaxNodes          -> 'x' Num
Shm               -> ',' Num
Port              -> '/' Num
Num               -> Digit | Digit Num
Digit             -> '0' .. '9'

with the shortcut for "optional":

T? -> '' | T
```

EXAMPLE continued: The string

```
compute+CPU:24,1073741824 compute+GPU:2,1073741824 reduce:4,2147483648 IO+load:2,16777216 IO+store:2,16777216 init:1x1
```

describes a topology with 24 and 2 workers of type "[compute, CPU]" and "[compute, GPU]" and "1 GiB" working memory per instance, 4 workers of type "reduce" and "2 GiB" working memory and 2 workers of type "[IO, load]" and "[IO, store]" each and "16 MiB" working memory per instance. Additionally there is a single worker of type "init" with no working memory.
