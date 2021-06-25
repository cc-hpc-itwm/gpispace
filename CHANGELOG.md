# [21.06] - 2021-06-25

## Optional Components

GPI-Space now supports disabling some components at configuration time
to reduce the dependencies needed as well as build time. The following
options can be given to CMake with either `=OFF` or `=ON` appended:

- `-DGSPC_WITH_MONITOR_APP`: The `gspc-monitor` (also known as
  "gantt") application for execution monitoring. This component
  requires Qt5.

## 'eureka-group' is now available as a tag that contains an expression

The attribute `eureka-group` of the XML tag `<module>` can only store
constant strings. To allow for data dependent eureka groups, the
element `<eureka-group>` has been introduced as an alternative.

The content of the new tag `<eureka-group>` is interpreted as an
expression and evaluated with the evaluation context of the
transition, e.g. with all the input tokens are known. The expression
shall return a value of type `string`.

Please choose between the attribute or the tag, it is an error to
specify both.

In general prefer the tag over the attribute as the tag is more
flexible. Current application code

```
<module ... eureka-group="value" ...>
</module>
```

can now be written as

```
<module ... ...>
  <eureka-group>"value"</eureka-group>
</module>
```

Note that the meaning of the double quotes '"' has changed in the
example: In the former `eureka-group="value"` they enclose the
attribute value, which is a string. Now, in
`<eureka-group>"value"</eureka-group>` the double quotes are
interpreted by the expression parser to parse the (same) string.

The possibility to let the eureka group depend on the inputs of a
transition is demonstrated by the example code:

```
<defun>
  <in name="eureka_group" type="string"/>
  <module>
    <eureka-group>${eureka_group}</eureka-group>
  </module>
</defun>
```

It determines the eureka group by the value of an input port.

String concatenation can be used to combine information into a more
complex eureka group:

```
<defun>
  <in name="eureka_group" type="string"/>
  <in name="eureka_id" type="string"/>
  <module>
    <eureka-group>${eureka_group} + "_" + ${eureka_id}</eureka-group>
  </module>
</defun>
```

## Installed libraries now hide unintended symbols

To avoid implementation details leaking into user code, GPI-Space's
libraries now no longer export all symbols but instead explicitly
export the intended API. Only symbols marked `GSPC_DLLEXPORT` in the
headers are part of the API and other symbols may vanish or change
without notice.

This implies that applications **might** break due to relying on
symbols that weren't intended to be used. This might also be the case
when using intended API but linking the wrong library. It might also
happen that due to GPI-Space's dependencies linking a previously
missing third-party dependency is now missing.

If an application fails to link with undefined symbols after updating
to this version, check whether these symbols came from GPI-Space
before. If that's the case, check

- whether the correct library is linked
- whether all third-party dependencies are linked and not relied upon
  GPI-Space to be provided
- whether the symbol used is actually part of the public GPI-Space API

The first two cases can be resolved by changing the libraries linked
to include the correct dependencies. In the latter case, a sibling
function might be part of the API and should be used instead. If there
is no such sibling function, contact GPI-Space support to discuss
whether it should be added to the API.

## [API CHANGE] gspc::worker_description hides its implementation

The structure `gspc::worker_description` now hides its implementation
in order to allow for future changes.

Applications that access the internal information must now store it by
themselves.

Also applications can not copy `gspc::worker_descriptions` any longer
but only `move` them. To adapt to this a current application code of
the form

```
gspc::worker_description const description {args...};

drts.add_worker ({description}, ...);
```

must now be written as

```
std::vector<gspc::worker_description> descriptions;
descriptions.emplace_back {args...};
drts.add_worker (descriptions, ...);
```

## CMake Utility Restructuring

GPISpace's CMake utilities underwent some restructuring as a step
towards modernizing the build infrastructure and the ongoing
modularization of GPISpace. It is now organized as a standalone
project (`util-cmake`) including install targets and a find package
config file. In addition, the directory structure of the scripts was
modified to prepend a namespace directory to better reflect the origin
of the scripts.

- `util-cmake` enforces the usage of a compatible cmake minimum
  version (currently 3.13). Parent projects will throw a fatal error
  if they don't set an appropriate CMake version with
  `cmake_minimum_required` before adding `util-cmake`.
- All CMake utility scripts need to have the namespace `util-cmake/`
  prepended to them (e.g. `include (add_macros)` => `include
  (util-cmake/add_macros)`)
- The `add_unit_test` function is no longer part of
  `add_macros.cmake`, but resides now in script of its own
  `add_unit_test.cmake`.
- The `bundle.sh` script should no longer be accessed using a
  hard-coded path.  Instead, util-cmake exposes a variable called
  `util_cmake_bundle_sh` populated with the correct absolute path.
- `util-cmake` can now be installed if required. By default, the
  installation is deactivated.

## GPISpace CMake Minimum Version Enforcement

Same as `util-cmake`, GPISpace's package config file now enforces the
usage of a required minimum CMake version (currently 3.13).
Applications failing to use at least that version will fail with a
fatal error during configuration time.

## Fixes

- Removed methods marked `INDICATES_A_RACE` from production code.
- Respect `TESTING_RIF_STRATEGY` and `TESTING_RIF_STRATEGY_PARAMETERS`
  in all tests.
- Fixed bug related to the thread-safe creation of SSL contexts.
- Improved documentation: added documentation for the Eureka feature
  and for creating topology descriptions.

## Miscellaneous

- Applications that link `pnetc` generated libraries can now link
  `GPISpace::APIGuard` instead of defining `WE_GUARD_SYMBOL`
  themselves.
- Removed the attribute `children_allowed` from workers. The agent can
  have only terminal components as workers now.
- Made the worker manager a component that belongs only to the
  scheduler, forbidding the direct interaction between agent and
  worker manager. The agent is now just talking to the scheduler,
  which is the only one who may access the worker manager, in a
  thread-safe way.
- Moved functionality proper to scheduling from the worker manager to
  the scheduler side.

# [21.03] - 2021-03-26

## Virtual Memory is now called IML and separate

The virtual memory layer of GPI-Space has been factored out to become
a separate component, the "Independent Memory Layer", short IML. It is
still shipped with GPI-Space and the APIs are backwards compatible.

GPI-Space has learned to use an externally started IML rather than
bootstrapping its own. This allows applications to keep an IML alive
across runs to cache data, or to allow preparing and extracting data
independent of GPI-Space. The socket of an external IML can be
specified with the `--remote-iml-vmem-socket` option, which needs to
point to the IML server on all hosts used by the runtime system.

The independent API is available in `include/iml/`. It can be used
independently from GPI-Space by using `find_package (IML)`.

## CMake "config" file now provided

GPI-Space now provides a CMake package config file to replace
`FindGPISpace.cmake` scripts. `find_package (GPISpace)` will now find
GPI-Space by searching `GPISpace_ROOT` and `GPISpace_DIR` or
`CMAKE_PREFIX_PATH` (see documentation of [CMake's `find_package`](https://cmake.org/cmake/help/v3.13/command/find_package.html#search-procedure)
for details).

Applications using `beautify_find_gpispace.cmake` will need to update
that script and remove their copy of `FindGPISpace.cmake`. The script
was updated to be backwards compatible with the existing arguments.

Applications using `find_package (GPISpace)` need to change how the
version requirement is specified as the standard CMake syntax is now
used:

```
# old
find_package (GPISpace REQUIRED COMPONENTS VERSION=20.12)
# new
find_package (GPISpace 21.03 EXACT REQUIRED)
```

The argument `REVISION` has been removed. Its usage will cause an
error instead. `beautify_find_gpispace` handling of `VERSION` has been
fixed.

The old `FindGPISpace.cmake` script is no longer supported and was
removed from the repository.

## Expression plugins: Nest exceptions, print context

When a plugin callback throws an exception, then the exception is
wrapped by an runtime_error that adds the evaluation context to the
error message.

That frees clients from adding the context and helps developers to
debug plugins.

## Fixes

- Information about dependencies found are no longer hidden when
  configuring.
- The XSD and RNC schemas now specify the attribute
  `allow-empty-ranges` at the correct location.
- `pnetc` now honors `<memory-buffer read_only="false">`, rather than
  treating presence of the attribute as `true`.
- Petri-net modules no longer link against `drts-context` if
  `pass_context="false"`.
- Fix error handling in client when receiving data in case of
  disconnection.
- Fixed races between event handlers and layer callbacks in agent that
  might cause `Error: reason := receiving response failed:
  asio. misc:2 code := 2` in rare cases.

### Allow multiple specializations of templates with module calls

It is now possible to use multiple specializations of templates that
contain module calls. In order to get unique wrappers, the type map is
mangled into the function name, e.g. if a template with the type
parameter `T` is specialized by `int` the function name is extended by
`_T_int`.

### Respect the `BUILD_TESTING` option

Allow the users to enable/disable building the tests by forcing the system to
properly handle the CMake `BUILD_TESTING` option.

## Miscellaneous

- Various improvements have been made to the README document to ease
  installation process of dependencies and GPI-Space and testing a
  fresh installation.
- The upper Boost version requirement of 1.63.0 is now enforced when
  configuring.
- The XSD and RNC schemas now document defaults.

# [20.12] - 2020-12-14

## Added an example that demonstrates "How to accumulate results from multiple submissions of the same workflow"

When a `gspc::workflow` is submitted multiple times, then each submission is independent and a later submission does not include the output of an earlier submission.
It is the responsibility of the application to accumulate results from multiple submissions of the same workflow.

## Add modules with target and correct the xml schema

- added new type for modules with user specified targets
- allowed multiple occurrences only for this type of modules

## Added the stochastic heureka example

Adds multiple stochastic algorithms examples using GPI-Space.

## Added a demonstration of how to use priorities to resolve conflicts

The Petri net allows to assign "priorities" to transitions. Priorities are integral numbers from the range `[0,2^16)` and are used to resolve "conflicts". A "conflict" describes the situation when two transitions `t1` and `t2` are both enabled but to fire one disables the other. This happens when transitions share input places.

If `t1` and `t2` have no priorities assigned or they have the same priority assigned, then GPI-Space randomly fires one of the conflicting transitions (and thereby disables the other one).

If, however, the transitions `t1` and `t2` have different priorities, then GPI-Space fires the transition with the higher priority.

Two examples are added:

1. Return the sum of all inputs provided. This tests uses priorities to select between multiple enabled transitions: If there are two arguments provided, then all transitions are enabled and the sum has the highest priority. If there is one argument provided, then the priority is to take that argument. Only if there is no argument provided the value zero is returned.

2. Combine priorities and conditions. Make sure the priorities are only relevant for transitions that are enabled. For that always provide inputs to all ports of all transitions. The transitions have overlapping conditions and are ordered by their priorities.

## Emphasize that execute_and_kill_on_cancel requires to not do any output to standard streams

The function `execute_and_kill_on_cancel` has been renamed to
`execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN`
in order to emphasize that property. Please adopt and make sure to not
output to standard streams from within, or else the call will block
and never return.

## API CHANGE: fhg/util/parse is no longer installed

If those includes are required, then please write a message to gpispace-support@itwm.fraunhofer.de to receive a copy.

## Fixed race related to subscriber notifications

The fix prevents the loss of notifications for a (failing) job while a
subscriber (client) not having completed the subscription process

## Skip empty memory transfers if the user explicitly allows this

If the memory transfers have the attribute 'allow-empty-ranges' set on true the
memory transfers that are empty are skipped, otherwise an error is triggered.

## Updating the GPI-2 dependency to version 1.3.2

GPI-2 version 1.3.2 includes a hotfix addressing an `mkdir: cannot create directory `xyz`: File exists` error during installation.

## Documentation Improvements

Restructuring and updating GPI-Space's installation instructions based on user feedback.
The released source code now also provides a CMake configuration file to ease the build process.

## Fixes

- `we_expr_parser: value_too_big<unsigned long, float> PARSE ERROR for -1f` has been fixed
- Several issues with the `sdpa_Preferences` test have been resolved
- A race condition causing `sdpa_InvalidNbWorkersReq` to segfault has been fixed
- Fixed several version inconsistencies

## Meta

- Ubuntu 18.04, Ubuntu 20.04, and Centos 8 were added to the platforms GPI-Space is tested on
- GPI-Space is now also tested using the Clang compiler
- GPI-Space is now using C++14 features

# [20.09.1] - 2020-10-16

## Compatibility hotfix

This hotfix introduces minor code changes to enable building and
running GPI-Space on Ubuntu 20.04. The ReadMe got updated to reflect
these changes.

## Fixes

- The CMake project version is now the same as the git tag version.
- The minimum required Boost version in the CMake script is updated
  to reflect the minimum requirements stated in the ReadMe.
- The copyright statement is now automatically updated with the
  current year.
- Increased the generated SSH key size for testing for compatibility
  with newer versions of OpenSSL.

# [20.09] - 2020-09-04

## Ability to override worker description per batch

When starting a `gspc::scoped_runtime_system` one traditionally
specifies a `topology_description` for the workers, which is reused
for any `add_worker()` call. An overload has been added that allows
giving a specific description per call, in order to allow asymmetric
topologies.

## RIF strategy 'local'

Developers testing on machines that support neither `ssh` nor `pbsdsh`
may now at least validate their application on the `local` node by
specifying `--rif-strategy=local`. The strategy expects the only node
given to be `hostname()`.

## Modify worker environment variables

It is now possible to specify the environment variables to be set when
starting a runtime system's workers, rather than having a predefined,
empty, environment.

By default the environment still is completely empty, and no variables
other than the ones referenced in the options will be set in the
remote process. The following options have been added:

- `--worker-env-set-variable` to explicitly set the value of a
  variable on the command line
- `--worker-env-copy-file path` to set all variables in the given
  files, where every line is a key-value pair in `env` style (`key=value`)
- `--worker-env-copy-variable name` and `--worker-env-copy-current` to
  copy variables set in the process starting up by allow-list or
  blanket copy respectively

## Ability to disable function-does-not-unload check

It is now possible to disable the check for a module call function not
leaking any dynamic libraries (and thus tainting the worker
process). Workflow developers may tag their `module` with
`require_function_unloads_without_rest="false"` (default: `true`) to
mark the taint as known and okay.

## Orchestrator was inlined into Agent

The `orchestrator` process has been removed and the functionality was
moved into `agent`. This has also removed the `--orchestrator-port`
command line option.

Due to removing this level, log messages may slightly differ. Besides
that post-job cleanup scripts should be adopted to no longer try to
kill `orchestrator`.

- `$install/lib` and `$install/libexec/gspc` is no longer added to
  `LD_LIBRARY_PATH` of worker automatically. Users may use
  `--worker-env-set-variable` to reconstruct this behavior, but proper
  `rpath`s are suggested instead.
- The `stencil_cache` library was removed as it was too specific for
  generic use.

## Fixes

- When raising a `missing_binding` exception, the full path into a
  struct is given, instead of just the root.
- It is now possible to start `0` workers per node in a topology
  description, to free clients from having to filter the description
  in degenerated cases.
- Error messages have been improved when using the RIF `ssh` strategy
  but neither having `USER` or `HOME` exported or having an `id_rsa`
  file, nor specifying the respective option on the command line.
- All RIF strategies now support `--help`, if they support command
  line arguments.
- Improved the performance of parallel applications that are
  generating large numbers of independent tasks, but have way fewer
  workers than tasks.
- `pnetc` no longer throws an exception when synthesizing virtual
  places with tokens on them.
- The virtual memory client no longer fails to transfer when a large
  amount of ranges is given.
- The expression language parser now correctly handles `integral 'f'`
  for floats.
- Properly clean up processes when using rif strategy `pbsdsh`

## Meta

- Support for GCC 9 and 10 has been added.
- (Source-build only) Added support for Qt5 versions 5.10 and up,
  tested with 5.9.7 and 5.14.2.

# Old Versions
## [20.05] - 2020-05-12
### Added
	- Eureka feature
		* Petri net extensions to identify and skip ongoing tasks
		* Conditional execution of workflow transitions
	- Buffer alignment
		* Enables user-specified buffer alignments
		* Applies to local buffers allocated in the worker
	- Support for Petri net workflows for heterogeneous clusters
		* XML support for defining multiple module implementations
		* XML support for Petri net transitions with preference order
		* Co-allocation scheduling with target device preferences
	- Stencil Cache
		* using virtual memory as I/O cache for stencil computations
	- Expression plugins
		* hook for user-provided callbacks within expression evaluation
	- ``connect-out-many`` feature
		* new Petri net edge for implicit decomposition of output list
 	- Dynamic requirements in the workflow
		* support for target compute device to be specified at runtime
	- User-specified worker description
		* Start DRTS workers with user-specified descriptions
		* Enable adding required worker type to transition modules
 	- Safeclouds SSL
		* support for SSL security protocol for cloud users
 	- Supports GCC 10 release
### Changed
	- Performance improvement to scheduling
		* reduced overhead of dynamic re-calculation for tasks assignment
		* dynamic worker-class and worker-state aware scheduling
		* up to 11.15x improvement in the scheduler performance
	- Updated logging infrastructure
		* decentralized and better usability
		* multiple log sink support via RPC
		* enables logging to the console (in addition to the gspc-monitor)
### Removed
	- Remove pnetd and pnetv daemons for simplified architecture
### Fixes
	- Fix for SSL context access in the network layer
	- Fix for FRTM memory leak in the 'Agent'
		* correctly handle worker job deletion
### Meta
	- Minimum tested GCC version is 4.9.4
		* support for lower versions discontinued

## [16.04] - 2016-04-15
### Added
	- execute_and_kill_on_cancel
	- vmem segment BeeGFS
	- work stealing for equivalence classes avoids unnecessary attempts
	- vmem support for more than 1024 nodes
	- gspc-rifd support for pbsdsh
### Removed
	- support for make in wrapper generation
	- set_module_call_on_cancel: replaced by execute_and_kill_on_cancel
	- option virtual-memory-per-node: each allocation creates a segment
### Changed
	- std::cerr as default for startup logging
	- vmem segment type must be specified by user
	- wrapper compilation uses hard coded paths into installation
	- gantt diagram: show multi line messages
	- gantt diagram: allow to disable merging (for better performance)
	- all installed binaries now have support for '-h'
### Fixes
	- rif requires child exit after pipe close
	- vmem semantics for all devices
	- correct bundling of libssh eliminates crashes
	- rpath settings
	- runtime system ownership and locking behavior
	- scheduling performance and crashes
### Meta
	- removed outdated examples
	- use value_type::read/show instead of boost::lexical_cast
	- enable Werror
	- ci running on centos5.7 and centos6.5
	- implement mmgr with c+11
	- code cleanup and choice of faster/correct data structures
	- use coroutine based rpc
	- remove id-indirection in xml representation
	- run ci with FHG_ASSERT_MODE=1

## [15.11] - 2015-11-12
### Added
	- basic petri net debugger
	- bundling to moveable installation
	- c++ api
	- add/remove_worker
	- explicit memory management
	- put_token, workflow_response
	- streaming support
	- rifd, rpc, scalable startup/shutdown
	- work stealing, locality aware scheduling

### Removed
	- pnete
	- kvsd
	- fhgcom
	- fake vmem
	- licensing
	- scripts

### Fixes
	- semantics of virtual memory layer
	- simplified logging for clients

### Meta
	- tests, use c++11
	- moved applications into separate repositories
	- moved generic parts into separate repositories
