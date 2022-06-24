# Preferences and Multiple Module Implementations

## Leveraging the Use of Heterogeneous Architectures

GPI-Space leverages the use of heterogeneous architectures, being able to combine
multiple task implementations for different target architectures in workflows and
to execute them efficiently on a given set of heterogeneous resources such that
the overall performance of an application is maximized.

The users are offered the possibility to design highly-optimized workflows for
current and emerging hardware accelerators such as `FPGAs` and `GPUs`. To leverage
the different compute capabilities, the users are required to provide
implementations optimized for specific devices and architectures. GPI-Space
provides support for scheduling and running specific task implementations,
allowing the users to efficiently exploit heterogeneous architectures and
increase the performance of applications.

## Heterogeneous Setup

For running GPI-Space in a heterogeneous environment consisting of
different computing devices, it should be started with a worker topology
assuming different classes of workers, typically one class per target
and one worker per device. The categorization into classes is done
by assigning capabilities to the workers, a capability being for example the
target architecture for which a worker is responsible with running task
implementations.
Before starting the runtime system the user should first create a so called
worker topology description that is used for remotely starting workers on a set
of hosts. Such a topology description can be for example `CPU:20 GPU:4 FPGA:2`,
which specifies that `20` workers with the capability `CPU`, `4` workers with
the capability `GPU` and `2` workers with the capability `FPGA` should be started
on each host.

In order to take advantage of a heterogeneous architecture, the users should also
provide different specific implementations for the tasks, typically one for
each target architecture, if possible. Apart this, the users may express
preferences about the selection order of task implementations to be considered
(e.g. according to their relative performances). This information is used as
a hint by the scheduler, which will automatically select an implementation of a
task to run using some cost-based criteria, but also using these hints.


## Preferences

The preferences are a way to tell to the GPI-Space scheduler that a task has
multiple implementations, one for each target, and that the order in which they
should be considered for selection is the one specified by the user.
The preferences must be defined in the Petri net XML description as a list
of items in the section `preferences` of the section `modules` of a transition
definition. Each preference should be defined within an XML element of type
`<target> ... </target>`. For example, for a heterogeneous worker topology as
defined above, the user might express a list of preferences for a transition
containing module implementations for the `FPGA`, `GPU` and `CPU` targets as in
the code snippet below:

```xml
    ...
    <transition name="transition_with_preferences">
      <defun>
        ...
        <modules>
          <preferences>
            <target>FPGA</target>
            <target>GPU</target>
            <target>CPU</target>
          </preferences>
          ...
        </modules>
      </defun>
      ...
    </transition>
    ...
```
In this example there are three preferences described, one for each target
architecture (i.e. `FPGA`, `GPU` and `CPU`). The preferences should exactly
match the worker capabilities defined in the topology description passed to the
runtime system, as in the section above.

For each preference in this list, the user is required to provide in the
subsequent `modules` section exactly one implementation for the corresponding
target. This implementation is intended to be run on a worker with a capability
identified by that target.

The order in the list of preferences is important. This tells to the scheduler
that this is the preferred order to respect when selecting which task
implementation a worker should run. For instance, assuming that the user disposes
of an `FPGA` implementation of a task that is highly optimized as compared to the
`GPU` and `CPU` versions, and that the `GPU` implementation is more performant
than the `CPU` version, one can specify the order as in the example above.

This provides to the scheduler important hints required to associate compute
costs with each heterogeneous worker type. The ultimate decision is taken by the
scheduler based on a cost-aware strategy, selecting the implementation to
execute in a way that guarantees an efficient utilization of resources that
minimizes the time to solution of the entire workflow.


## Multiple Module Implementations

GPI-Space models the application workflow as a Petri Net and defines the
individual tasks as its transitions. These transitions encapsulate the task
executions into modules that are scheduled on workers across the distributed
system. In order to support heterogeneity, it is vital the users to provide
multiple module implementations for any type of task required by the
application, each of them targeted at a particular type of architecture (assuming
that the runtime system is started on a heterogeneous set of resources).
The usage is not restricted to architectures that are heterogeneous from a
physical point of view only (i.e. using different hardware devices). One can
also handle software heterogeneities. For example, one may have tasks relying on
functionality provided by third-party software. It cannot be always ensured that
on different machines the same software versions or libraries are installed. In
this case the user may provide different task implementations that are either
using different software versions, possibly with API differences (e.g. Python 3
or Python 2), or different libraries (e.g. libssh or libssh2), expressing in the
same time his preferences for one version/type or another.

For each target specified in the list of preferences, the user is required to
provide an implementation that is specific to that target.
Typically, the user should provide within the section corresponding to the
element `modules` a `module` subsection, following the subsection `preferences`,
for each preference, as in the example below:


```xml
    ...
    <transition name="heterogeneous_compute_task">
      <defun>
        ...
        <modules>
          <preferences>
            <target>FPGA</target>
            <target>GPU</target>
            <target>CPU</target>
          </preferences>

          <module name="compute_task"
                  function="result func_compute_task()"
                  target="FPGA">
                  <cinclude href="..."/>
            <cxx flags="..."/>
            <ld flags="..."/>
            <code>
                <![CDATA[ ...  ]]>
            </code>
          </module>

          <module name="compute_task"
                  function="result func_compute_task()"
                  target="GPU">
            <cinclude href="..."/>
            <cxx flags="..."/>
            <ld flags="..."/>
            <code>
                <![CDATA[ ...  ]]>
            </code>
          </module>

          <module name="compute_task"
                  function="result func_compute_task()"
                  target="CPU">
            <cinclude href="..."/>
            <cxx flags="..."/>
            <ld flags="..."/>
            <code>
                <![CDATA[ ...  ]]>
            </code>
          </module>
        </modules>
      </defun>
      ...
    </transition>
    ...
```

In this example there are three preferences defined: `FPGA`, `GPU` and `CPU`.
The user must provide for each preference defined in the section
`preferences` a module implementation for the target
identified by that preference (specified also as `target` attribute in the
example). In the section `code` the specific `C++` code
implementation for the intended target should be given. Also, for each specific
implementation the user should make sure that the necessary include headers and
compiler and linker flags are specified.

Note: when such a Petri net is compiled with `pnetc`, it will output a shared
library for each target module. In order to be able to easily identify them
we use the convention to add besides the module name also the
target name (separated by an underscore) as a suffix. Thus, when considering the
example above, the generated shared libraries containing task implementations will
be`libcompute_task_FPGA.so`,`libcompute_task_GPU.so` and `libcompute_task_CPU.so`,
respectively.


## Example Use-Case

As an example, let us assume that we dispose of a heterogeneous collection of
resources consisting of `FPGAs`, `GPUs` and `CPUs`. Let us also consider that we
have a workflow-driven application that we would like to run on this heterogeneous
system, as the one in the system test `system_test/preferences_and_multimodules`.

The application requires the users to provide some parameters like the total
number of tasks to generate and the number of workers per host to use
for each target architecture. These are currently provided as arguments to the
test in the corresponding `CMakeLists.txt` file in the source tree but can also be
supplied as command line arguments when running the example standalone.

For executing the workflow the method `execute`, which takes as arguments the
workflow and the provided program options should be invoked. This method
implements the so-called driver that starts the runtime, submits the workflow,
waits for workflow termination and collects the results, as one can see in
`system_test/preferences_and_multimodules/src/execute.cpp`. The preferences are
assumed to be those mentioned above. The worker topology used per host is
`FPGA:n0 GPU:n1 CPU:n2`, where `n0`, `n1` and `n2` are the elements of the array
provided as input by the user for the number of workers to use for each target.

The workflow used in this test generates many tasks that perform the same
operation but are provided by the user multiple implementations for the
enumerated target architectures. The purpose of this example is to illustrate how
the preferences and the multimodules can be used in GPI-Space.

Although the example is annotated with comments that are meant to give the users
useful hints about the usage of preferences and multimodules, we try below to
briefly explain how it works.

The workflow contains a transition named
"transition_with_multiple_module_implementations" which contains a section
`modules` incorporating several module implementations for the considered
targets, as explained in the previous paragraph. More specifically, there is a
module containing a specific implementation provided by the user for each of
the above considered targets. Here, the implementation details are skipped since
we cannot run the specific task implementations on the continuous integration
infrastructure. Nevertheless, important in this test is to give the user
feedback about the implementations that were chosen and executed. The section
`modules` also contains a section `preferences` defined as
  ```XML
    ...
    <preferences>
      <target>FPGA</target>
      <target>GPU</target>
      <target>CPU</target>
    </preferences>
    ...
  ```
This tells to the scheduler that, whenever it is possible to choose among
different implementations to run, it should select a task implementation
according to the usual cost-driven strategies but also by taking into
consideration this order fixed by the user (preferences).

In the considered example, the transition corresponding to a task
produces output tokens containing the implementation type (target) executed by
the task. We can use these values to check at the end if the behavior is the one
assumed and the outcome is the one expected. This is done in the workflow's method
`process` that is processing the result after the execution of the workflow.

First of all, the method checks if the number of `implementation` output tokens
is the same as the number of tasks submitted. For each task, there should be
exactly one implementation selected by the scheduler and executed by a worker
that is dedicated to executing implementations for that target.

In case the total number of tasks doesn't exceed the number of workers dedicated
to executing implementations for the most preferred target (i.e. `FPGA`), it is
expected to have executed only `FPGA` implementations by workers dedicated to
this target (most preferred), despite the fact that other workers dedicated
to less preferred targets were free and could have executed the task as well.

In case when the total number of tasks exceeds the number of workers dedicated
to executing implementations for the most preferred target (i.e. `FPGA`), but
doesn't exceed the number of workers dedicated to the first two most preferred
targets (i.e. `FPGA`and `GPU`), it is expected to have executed only `FPGA` or
`GPU` implementations. All the `FPGA` workers should have executed an `FPGA`task
implementation, the rest of the tasks being executed by `GPU` workers. The
workers dedicated to the least preferred target (`CPU`) should have executed
nothing, despite they were free.

In the contrary case, if the number of tasks doesn't exceed the total number of
workers, it is expected that all workers have executed exactly one task
implementation corresponding to the target which they were dedicated
to (there may still be free workers only for the least preferred target, though).

In the case when the number of tasks exceeds the total number of workers, it is
ensured that all resources in the considered heterogeneous environment were used
and all workers have executed multiple implementations corresponding to the
targets they were dedicated to. In this case more fine grain output verification
requires internal scheduling information, aspects that are currently
tested by the internal scheduler unit tests.

The example can be run as a system test using the cmake command (i.e `cmake -R preferences_and_multimodules`) or standalone, as a `GPI-Space` application,
using a command line similar to the one below:

        $TEST_INSTALL_DIR/bin/preferences_and_multimodules   \
           "--gspc-home=/dev/shm/rotaru/gspc/install"        \
           "--nodefile=$NODEFILE"                            \
           "--rif-strategy=ssh"                              \
           "--num-tasks=10000"                               \
           "--num-workers-per-target" "5" "4" "3"

where `TEST_INSTALL_DIR` points to the location where the test/application was
installed, `NODEFILE` points to a nodefile containing unique host names,
`gspc-home` is the location where GPI-Space was installed, `rif-strategy` is the
strategy used by the bootstrapping mechanism, `num-tasks` is the total number
of tasks to execute and `num-workers-per-target`  is an array of integers (of
size number of targets/preferences), in which each element represents the number of
workers dedicated to the corresponding target in the list of preferences.
