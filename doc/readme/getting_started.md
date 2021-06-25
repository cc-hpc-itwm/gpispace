# Getting started with GPI-Space

This document provides a step-by-step guide on how to create a first
GPI-Space application.

## Introduction
To create a GPI-Space application and benefit from its automated task
management, users define the workflow pattern and the code that makes
up each individual task in that workflow. In GPI-Space, a workflow is
defined as an abstract Petri Net that can be executed by the GPI-Space
framework.

### Programming Model
To describe and design scalable and parallelizable applications,
GPI-Space leverages the concept of "Petri Nets". Petri Nets enable
modeling concurrent and distributed systems. In essence, a Petri Net
is a collection of directed arcs connecting "places" and
"transitions". It can be seen as a bipartite graph with arcs only
going from "place" to "transition" or vice versa, as shown in the
example below. A more formal definition of a Petri Net is a tuple
`N = (P, T, F, M)`, where:

* `P` is a finite set of places
* `T` is a finite set of transitions
* `M` is the "Marking", a function from `P` to the natural numbers
  `N`, where `N` is the number of tokens in a place
* Arcs or flow relations `F` emerge from `P --> T` or `T --> P` only

`Places` and `transitions` define a logical workflow, which can
execute if the transitions are ready to "fire". A transition can fire
once real values or `tokens` are put onto `places`. When the
transition fires, it consumes one token from each input place and
produces one token on each output place.

A Petri Net with multiple fire-able transitions inherently enables
'task parallelism' and 'data parallelism'. For example, in the figure
below:

* Two `t1` transitions can fire simultaneously (data-parallel), each
  consuming one token at input place `p1`.
* Transition `t1`, `t2` and `t3` can figure simultaneously
  (task-parallel), in the figure below.

![A Petri Net with four transitions Tx and six places Px: T1 has one
 input P1 and two outputs P2 and P3. T2 has input P2 and output P4. T3
 has input P3 and P5. T4 has two inputs P4 and P5 and one output
 P6. There are two tokens on P1, one token on P2, and one token on
 P3. Transitions T1, T2 and T3 are able to fire, T4 is
 not.](./img/petri_net.svg)

### Architecture
The GPI-Space framework builds on a "agent-worker" architecture, as
shown in the figure below. The agent houses the workflow engine and
the scheduler. The workers processes that execute the tasks are
distributed across the compute nodes. The Remote Interface Daemon
(RIFD) on each host coordinates startup and shutdown steps. A
distributed shared memory layer (Virtual Memory) completes the
GPI-Space ecosystem. The Virtual Memory, Workers and the RIFDs
together constitute the Distributed Runtime System (DRTS), as depicted
in the figure below.

![Architecture Overview of GPI-Space Eco-System: GPI-Space consists of
 the Agent (on top) and the DRTS (on the bottom). Within the agent,
 there is the workflow engine (WE) and the Scheduler, which have a
 bidirectional connection. The DRTS is split into two parts:
 Components on individual compute nodes and the Virtual Memory
 spanning all nodes. Within each node, there are workers tied to CPU
 cores, as well as a RIFD. The scheduler has a has a connection to
 each worker. Outside of GPI-Space is the User Application Workflow,
 containing a Petri Net, which has a connection to the
 Agent.](./img/GPISpace_arch.svg)

### Developing a GPI-Space Application
Writing a GPI-Space application requires three steps:

1. Implementing the logic behind individual tasks (domain-specific
   code).
2. Designing a Petri Net that defines the application workflow with
   the tasks defined in step (1).
3. Setting up the distributed GPI-Space cluster to run the workflow
   defined in step (2).

The following sections detail the above steps with an example.

#### Application Example
The GPI-Space application design and execution is illustrated with a
simple example we call `compute_and_aggregate`. It computes `N` values
and finally aggregates them into a sum. This example can be considered
as a simplification of any application that needs to perform a single
aggregation (a reduce operation) of values computed by tasks
distributed across workers, i.e., `N` values computed in a distributed
fashion. The Petri Net for this application workflow is illustrated
below.

![Petri Net for the compute_and_aggregate example: There is a top
 level input task_generator and a top level output
 aggregated_value. There are two transitions, compute and
 aggregate. compute has one input in and one output out. aggregate has
 two inputs, i and sum, and one output sum. There are three places,
 generator, computed_values and global_sum. The place global_sum has a
 single token which is highlighted to emphasize it being the
 accumulator, the places generator and computed_values have three and
 two tokens each. compute's port in is consuming from place generator,
 the port out is producing to computed_values. aggregate's port i is
 consuming from computed_values, both the input and output port sum
 are connected to global_sum.](./img/gpispace_example_pnet.svg)

Note: The following listings assumes the set of variables as given in
the `${GPISPACE_INSTALL_DIR}/share/gspc/README.md` file and additionally

- `${APP_INSTALL_DIR}` is an empty directory on a shared filesystem
  where the example application will be installed in.

Note: You can find the complete application example source code at
`${GPISPACE_INSTALL_DIR}/share/gspc/example/compute_and_aggregate/` in
your GPI-Space installation.

#### User Code
For the `compute_and_aggregate` example, users design the code that
defines the tasks that generates the values and the sum function.

For instance, a `print_and_return_a_random_value()` function that
performs one desired computation (i.e., randomly generate a value and
print it, in our example) could be as follows:

```c++
#include <iostream>
#include <random>

unsigned int print_and_return_a_random_value()
{
  unsigned int const some_computed_value = std::random_device{}();
  std::cout << "Random value: " << some_computed_value << '\n';

  return some_computed_value;
}
```

The `N` distributed values being computed can be aggregated as they
are generated into a partial sum by invoking a `aggregate_value_sum`
function:

```c++
#include <atomic>

std::atomic<unsigned int> global_sum {0};

void aggregate_value_sum (unsigned int value)
{
  global_sum += value;
}
```

#### XML-based Workflow (.xpnet)
To define a workflow as a Petri Net an XML file is used. This file is
referred to as a "xpnet". For validation in an XML editor the scheme
`${GPISPACE_INSTALL_DIR}/share/gspc/xml/xsd/pnet.xsd` can be used.

Each `xpnet` implements a function and therefore `<defun>` must be the
top level tag of every `xpnet`. A function has a signature, described
by typed and named `<in>` and `<out>` ports. A function's
implementation might be a `module`, an `expression` or a nested Petri
`<net>`. The following snippet defines a function `sum` that takes two
arguments `lhs` and `rhs` and produces one output `sum`, `lhs + rhs`.

```xml
<defun name="sum">
  <in name="lhs" type="int"/>
  <in name="rhs" type="int"/>
  <out name="sum" type="int"/>
  <expression>
    ${sum} := ${lhs} + ${rhs}
  </expression>
</defun>
```

A nested Petri Net consists of `<place>`s and `<transition>`s as
described above. A transition calls a function by connecting its
input/output ports to input/output places. The following snippet shows
a transition `accumulate` which takes the current value of
`accumulator` and uses `sum` to add `value` to it.

```xml
<place name="value" type="int"/>
<place name="accumulator" type="int"/>
<transition name="accumulate">
  <use name="sum"/>
  <connect-in port="a" place="value"/>
  <connect-in port="b" place="accumulator"/>
  <connect-out port="sum" place="accumulator"/>
</transition>
```

`Modules` involve user-defined code and are scheduled as tasks to the
workers. They include user-defined C++ code that can invoke external
shared libraries, if necessary. The `<code><![CDATA[..]]></code>`
wraps the user-defined code to be executed by a worker.

For example, the `print_and_return_a_random_value()` functionality can
be wrapped into a transition `compute` with a `module`
(`compute_random_value`) as follows:

```xml
    <place name="trigger" type="control"/>
    <place name="computed_values" type="unsigned int"/>

    <transition name="compute">
      <defun>
        <in name="in" type="control"/>
        <out name="out" type="unsigned int"/>

        <module name="compute_random_value"
                function="out print_and_return_a_random_value()">
        <cinclude href="iostream"/>
        <cinclude href="random"/>
        <code><![CDATA[
          unsigned int const some_computed_value = std::random_device{}();
          std::cout << "Random value: " << some_computed_value << '\n';

          return some_computed_value;
        ]]></code>
        </module>
      </defun>

      <connect-in port="in" place="trigger"/>
      <connect-out port="out" place="computed_values"/>
    </transition>
```

Input tokens that trigger a `transition 'compute'` are consumed from
`place 'trigger'` via input port `in`. The value computed by the
worker (`some_computed_value`) is returned back to the workflow at
`place 'compute_values'` via port `out`. Such input/output ports and
the place connection relationships in the Petri Net are specified via
`<connect-in/out/inout>` XML tags. Note that the "computation" does
not depend on the input triggering it, the mere existence is enough,
which is why it can use a `control` token rather than an integer or
alike.

Unlike a `module`, an `expression` is executed in the agent (i.e., no
task scheduling to workers). While it could also be written as a
`module`, a simple aggregation such as `aggregate_value_sum` could
benefit from being centralized at the agent that receives output
values computed from the distributed workers. To compute an on-the-fly
aggregation of values  generated at the `place 'compute_values'` as
done by the `aggregate_value_sum` function described above, a
`transition` with an `expression` can be defined as follows:

```xml
    <place name="global_sum" type="unsigned int">
      <token><value>0U</value></token>
    </place>

    <transition name="aggregate_value_sum">
      <defun>
        <in name="i" type="unsigned int"/>
        <inout name="S" type="unsigned int"/>
        <expression>
          ${S} := ${S} + ${i}
        </expression>
      </defun>
      <connect-in port="i" place="computed_values"/>
      <connect-inout port="S" place="global_sum"/>
    </transition>
```

The `<connect-inout>` tag enables the `place 'global_sum'` to be
updated incrementally with partial sums, via port `S` that serves as
both an input and an output port. It is equivalent to a `unsigned
int&` argument in C++.

To put together the above two transitions into a Petri Net for the
`compute_and_aggregate` example, the XML-based workflow
`compute_and_aggregate.xpnet` can be written as:

```xml
<defun name="compute_and_aggregate">
  <in name="task_trigger" type="control" place="trigger"/>
  <out name="aggregated_value" type="unsigned int" place="global_sum"/>

  <net>

    <place name="trigger" type="control"/>
    <place name="computed_values" type="unsigned int"/>
    <place name="global_sum" type="unsigned int">
      <token><value>0U</value></token>
    </place>

    <transition name="compute">
      <defun>
        <in name="in" type="control"/>
        <out name="out" type="unsigned int"/>

        <module name="compute_random_value"
                function="out print_and_return_a_random_value()">
        <cinclude href="iostream"/>
        <cinclude href="random"/>
        <code><![CDATA[
          unsigned int const some_computed_value = std::random_device{}();
          std::cout << "Random value: " << some_computed_value << '\n';

          return some_computed_value;
        ]]></code>
        </module>
      </defun>

      <connect-in port="in" place="trigger"/>
      <connect-out port="out" place="computed_values"/>
    </transition>

    <transition name="aggregate_value_sum">
      <defun>
        <in name="i" type="unsigned int"/>
        <inout name="S" type="unsigned int"/>
        <expression>
          ${S} := ${S} + ${i}
        </expression>
      </defun>
      <connect-in port="i" place="computed_values"/>
      <connect-inout port="S" place="global_sum"/>
    </transition>

  </net>
</defun>
```

With the above Petri Net workflow, `N` values can be generated by
placing `N` tokens onto the `place 'trigger'`. This can be done by
another `transition` or from the application driver program through
the input port `task_trigger` connected to `place 'trigger'` (the
choice for this example is described below). The `place 'global_sum'`
computes partial sums as the tasks for `transition 'compute'`
execute. The total aggregated value is available to application user
via the output variable `aggregated_value`, once the workflow
completes running all of its tasks.

Note that the order of printing might differ from the order of
summation: GPI-Space does not define any order on transitions being
executed. In case of integral values the summation order doesn't
matter. However, in case of fractional values (or floating point
addition), the order is important. Multiple runs will produce
different print orders of printing. Multiple runs with the same order
of printing will produce different aggregated sums.

#### Startup Binary
The startup binary -- also called application driver -- is responsible
for initializing the DRTS and the Agent (i.e., RIFD and Workers),
submitting the job to the Agent, and monitoring for the results that
needs to be relayed back to the user. The following sections present a
detailed breakdown of the driver program (`driver.cpp`).

##### Input parameters and Setting Program Options
Input parameters required to set up the distributed cluster, such as
the RIFD startup strategy, the nodefile path (listing of all hosts to
use), etc., can be supplied to the startup binary as command line
arguments. The `gspc::options::*` functions provide groups of options
for `logging`, `scoped_rifd`, the `drts`, `virtual_memory` and the
GPI-Space `installation`. These command line inputs can be parsed
using Boost's Program Options library. User-defined values can also be
defined via the same options options description to provide generic
GPI-Space and application specific parameters in the same command
line.

```c++
#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
try
{
  boost::program_options::options_description options ("compute_and_aggregate");
  options
    .add (gspc::options::drts())
    .add (gspc::options::logging())
    .add (gspc::options::scoped_rifd())
    .add_options()
       ("N", boost::program_options::value<int>()->required())
       ("workers-per-node", boost::program_options::value<int>()->required())
       ("help", boost::program_options::bool_switch()->default_value (false));

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options).run()
    , vm
    );

  if (vm.at ("help").as<bool>()) {
    std::cout << "Usage:\n" << options << "\n";
    return 0;
  }

  vm.notify();

  int const workers_per_node (vm.at ("workers-per-node").as<int>());
  int const N (vm.at ("N").as<int>());

  // See the following sections.

  return 0;
}
catch (std::exception const& ex)
{
  std::cerr << "Error: " << ex.what() << "\n";
  return 1;
}
```

##### Setting up GPI-Space
The first step in the startup binary is to setup the RIFD daemons with
relevant information regarding the GPI-Space installation, the
hostnames and port to use as well as the communication strategy to
start up the distributed components.

```c++
  boost::filesystem::path const app_install_dir (APP_INSTALL_DIR);
  gspc::set_application_search_path (vm, app_install_dir / "lib");

  gspc::installation const gspc_installation (GPISPACE_INSTALL_DIR);

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}   // ssh or pbdsh
    , gspc::rifd::hostnames {vm}  // a vector of host names
    , gspc::rifd::port {vm}       // port for communication
    , gspc_installation
    );
```

Next, the runtime system object is created, to start workers on all
the machines. A user-defined topology string is used to setup the
cluster, as shown below. At this point, the workers in the DRTS are
ready to receive work.

```c++
  // define topology for GPI-Space: workers_per_node workers with
  // capability/name "worker" per host.
  std::string const topology_description
    ("worker:" + std::to_string (workers_per_node));

  gspc::scoped_runtime_system drts
    ( vm                    // variables_map containing GPI-Space options
    , gspc_installation
    , topology_description
    , rifds.entry_points()  // entry points to start agent and workers on
    );
```

##### Creating workflow and client
Once the runtime system is setup, a GPI-Space client (job submitter or
user) objects are created and attached to the DRTS. Additionally, the
path to the Petri Net is furnished to submit the workflow to the
runtime (this path is a pointer to the Petri Net definition obtained
after compiling the XML-based definition, and is described below).

```c++
  gspc::workflow const workflow
    (app_install_dir / "compute_and_aggregate.pnet");

  gspc::client client (drts);
```

##### Submitting workflow
Once the GPI-Space client and workflow are created, the application
can be launched with the following steps:

* place any input tokens to trigger the workflow execution (in case of
  our example, place `N` tokens onto the input port `task_trigger`,
  and,
* execute a Petri Net-based workflow by calling `put_and_run()` on the
  `client`.

```c++
  std::multimap<std::string, pnet::type::value::value_type> values_on_ports;

  // put N values onto place "trigger" via input port "task_trigger"
  // to trigger N "compute" tasks
  for (int i (0); i < N; ++i) {
    values_on_ports.emplace ("task_trigger", we::type::literal::control{});
  }

  auto const results (client.put_and_run (workflow, values_on_ports));
```

##### Retrieving Output Results
The output or output port values will be available in the
`std::multimap<> result`. For the `compute_and_aggregate` example, the
result at the output port `aggregated_value` can be extracted as
follows:

```c++
  if (results.size() != 1 || results.count ("aggregated_value") != 1) {
    throw std::logic_error ("unexpected output");
  }

  pnet::type::value::value_type const final_result_value
    (results.find ("aggregated_value")->second);

  std::cout << boost::get<unsigned int> (final_result_value) << std::endl;
```

`pnet::type::value::value_type` refers to generic Petri Net types that
are handled by the GPI-Space workflow engine.

### Running the GPI-Space Application
Having defined the workflow and the startup binary, this section
discusses how to manually build and run the GPI-Space application.

#### Compiling a Petri Net XML Workflow
First, the GPI-Space-provided Petri Net compiler (`pnetc`) is used to
generate an internal representation (a `.pnet` file) of the XML-based
Petri Net defined in the `xpnet` file (see above).     The path to the
`.pnet` file generated by the `pnetc` compiler needs to be passed to
the startup binary, while creating a `gspc::workflow` (see "Creating
Workflow and client" above).

For the `compute_and_aggregate` example, the Petri Net can be compiled
as follows:

```bash
"${GPISPACE_INSTALL_DIR}/bin/pnetc"                               \
  --input "compute_and_aggregate.xpnet"                           \
  --output "${APP_INSTALL_DIR}/compute_and_aggregate.pnet"
```

Next, `pnetc` is used to generate wrapper code for the modules defined
in the `.xpnet`, which are then compiled using the generated
`Makefile` into a shared library that can be employed by the workers
at runtime. For the `compute_and_aggregate` example, the Petri Net
modules can be built as follows:

```bash
"${GPISPACE_INSTALL_DIR}/bin/pnetc"                               \
  --input="compute_and_aggregate.xpnet"                           \
  --output="/dev/null"                                            \
  --gen-cxxflags=-O3                                              \
  --path-to-cpp="${APP_INSTALL_DIR}/src"

make install                                                      \
  -C "${APP_INSTALL_DIR}/src"                                     \
  LIB_DESTDIR="${APP_INSTALL_DIR}/lib"
```

#### Building startup binary
Once the user-defined code library containing the workflow has been
created, we need to compile and build the startup binary that is need
to launch and execute the Petri Net-based workflow.

The startup binary for `compute_and_aggregate` can be built with the
GPI-Space libraries as follows:

```bash
"${CXX}"                                                          \
  -Wall -Wextra -Werror                                           \
  -std=c++11                                                      \
  -DAPP_INSTALL_DIR="\"${APP_INSTALL_DIR}\""                      \
  -DGPISPACE_INSTALL_DIR="\"${GPISPACE_INSTALL_DIR}\""            \
  driver.cpp                                                      \
  -o "${APP_INSTALL_DIR}/bin/compute_and_aggregate"               \
                                                                  \
  -isystem "${GPISPACE_INSTALL_DIR}/include"                      \
  -L "${GPISPACE_INSTALL_DIR}/lib/"                               \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/lib/"                       \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/libexec/bundle/lib/"        \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/libexec/iml/"               \
  -lgspc                                                          \
                                                                  \
  -isystem "${GPISPACE_INSTALL_DIR}/external/boost/include"       \
  -L "${GPISPACE_INSTALL_DIR}/external/boost/lib/"                \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/external/boost/lib/"        \
  -Wl,--exclude-libs,libboost_program_options.a                   \
  -lboost_program_options                                         \
  -lboost_filesystem                                              \
  -lboost_system
```

As a result, the `run_compute_and_aggregate` application is ready to
be used and can be executed.

#### Start the GPI-Space monitor
GPI-Space provides a graphical interface for the logging messages from
the application workflow (i.e., `std::cout` within modules is
redirected here) if built with option `GSPC_WITH_MONITOR_APP`
enabled. This can be launched (with a user-specified port number that
shall also be given to the runtime system) as follows:

```bash
"${GPISPACE_INSTALL_DIR}/bin/gspc-monitor" --port "${LOG_PORT}" &
```

Alternatively, one can use
`${GPISPACE_INSTALL_DIR}/bin/gspc-logging-to-stdout.exe` if no
graphical session is available or one is mostly interested in getting
logging messages. Also see
`${GPISPACE_INSTALL_DIR}/share/gspc/gspc-monitor.html` for more
details on the monitor GUI.

#### Deploying
For the sake of brevity, this example hard-codes the paths to
`${APP_INSTALL_DIR}` and `${GPISPACE_INSTALL_DIR}` via `-D` or
`-Wl,-rpath` in the compilation step. Thus this example is not movable
and the GPI-Space installation it uses is not movable or replaceable
either, making it hard to ship an application to users.

The GPI-Space installation itself is self-contained and freely
movable, so applications may copy it into their own installation
directories, e.g. `libexec/bundle/gpispace`, and then at runtime use
the executable's path (determined for example using `dladdr()`) and
the knowledge of the relative path to construct a
`gspc::installation`. The same method can also be used to avoid
compiling `${APP_INSTALL_DIR}` into the binaries, to get the `.pnet`
and module call library location.

The libraries and executables built want to change the `-rpath` which
in the example is set to point into `${GPISPACE_INSTALL_DIR}` to be
relative as well, using `$ORIGIN/../libexec/bundle/gpispace/lib/` or
alike. Make sure to correctly quote for shells and `Makefile`,
especially when you're using multiple layers of them.

#### Running
The startup binary `${APP_INSTALL_DIR}/bin/run_compute_and_aggregate`
can be executed with necessary command-line parameters to launch the
GPI-Space application.

For example, for `compute_and_aggregate`, the driver
`${APP_INSTALL_DIR}/bin/run_compute_and_aggregate` can be used to
launch the workflow on a single node with 4 cores, as follows:

```bash
hostname > "${APP_INSTALL_DIR}/nodefile"
# note: the location doesn't matter for the execution
# note: this script puts the nodefile into the ${APP_INSTALL_DIR} as
# this directory is known to be writeable
# note: to test in a cluster allocation, for `--nodefile` below, use
# Slurm: "$(generate_pbs_nodefile)"
# PBS/Torque: "${PBS_NODEFILE}"

"${APP_INSTALL_DIR}/bin/compute_and_aggregate"                    \
  --rif-strategy "${RIF_STRATEGY:-ssh}"                           \
  --nodefile "${APP_INSTALL_DIR}/nodefile"                        \
  ${LOG_PORT:+--log-host ${HOSTNAME} --log-port ${LOG_PORT}}      \
  --N 20                                                          \
  --workers-per-node 4                                            \
  "${@}"
```

First, a `nodefile` containing the hostname of the system is
created. Then the application is invoked, providing it with the
necessary command line parameters:

- `--rif-strategy`: the strategy used to bootstrap runtime system
  components, usually "ssh" which uses password- and passphrase-less
  `ssh` to the given nodes. The `ssh` strategy uses `~/.ssh/id_rsa` by
  default, which can be overwritten by passing
  `--rif-strategy-parameters="--ssh-private-key='${HOME}/.ssh/other_key'
  --ssh-public-key='${HOME}/.ssh/other_key.pub'"` as well.
- `--nodefile`: the file containing the hostnames where to run the
  application. In this case, the just created `nodefile` with a single
  host
- `--log-port` and `--log-host`: the hostname and port used in order
  to connect to the gsp-monitor (see above), if started
- `--N`: the application specific option with the number of values to
  generate
- `--workers-per-node`: the application specific option with the
  number of workers to use

You can see more parameters that may be passed to the GPI-Space
runtime system using `--help`. The snippet above symbolizes this as
`"${@}"`.
