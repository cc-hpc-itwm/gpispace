# Goals

In 2013 we first debated adding a sandbox to our workers for the sake
of not clobbering the user code symbol namespace with implicitly
exported Boost symbols, which a year later led us to bundling the
entirety of Boost instead.

The topic of sandboxing user code has come up in other contexts as
well: To better handle segfaults (don't crash the entire worker,
leading to hanging jobs), to cancel instantly (as one can't `kill()` a
thread, leading to a quick-hack added for FRTM
`execute_and_kill_on_cancel()`), and to avoid unwanted shared global
state between module calls (as noticed in Aloma, where one module call
leaked a shared library leading to all module calls inadvertently
executing the same code), all issues that have arisen multiple times
over the past five years.

This document aims at listing these contexts/goals and discussing
which ones we focus on and what we need to do in to tackle them.

- `[#g.no export]` Avoid exporting any symbols a user may have a
  conflict with, i.e. Boost. The module execution context should only
  provide exactly what is needed to execute the context.

[]()
- `[#g.isolated segfault]` Segfaulting a module shall nicely mark the
  job as failed and not break the worker.
- `[#g.no taint]` Dynamic libraries being loaded shall not influence
  the worker.
- `[#g.no leak]` Stopping drts-kernel shall never leak sandboxes.
- `[#g.no user in worker]` Nothing from the user execution other than
  the output tokens or exception message and vmem side effects shall
  reach from sandbox into worker.

[]()
- `[#g.cancel.kill]` It shall be possible to kill a module regardless
  of cooperation.
- `[#g.cancel.coop]` It shall be possible to ask the module for
  cooperative cancellation.
- `[#g.cancel.coop plus]` It shall be possible to ask the module for
  cooperative cancellation but kill it after inaction.

[]()
- `[#g.transfer results]` In contrast to `execute_and_kill_on_cancel`
  transferring input and output shall be possible.

[]()
- `[#g.reuse]` The sandbox shall not re-implement basics like RPC or
  syscall wrappers.
- `[#g.no overhead]` There shall be no additional overhead for vmem
  transfer, i.e. the sandboxes shall share the shm segment with the
  worker.
- `[#g.logging]` Logging shall work as before, without flushing
  issues.
- `[#g.errors]` Errors when loading the module .so or bootstrapping
  the sandbox shall be properly reported.
- `[#g.pinning]` The sandbox shall be possible to be CPU pinned like
  the current workers are (i.e. inherit or re-implement).

[]()
- `[#g.policy]` User shall be able to choose between starting a
  sandbox (see `[n.policy]`)
  - per module call (instance/function invocation) or
  - per module call (implementation/function definition) or
  - per module (.so/group of function definitions) or
  - `[#g.policy.unsure]` per user defined set of .so or
  - `[#g.policy.unsure]` one sandbox for all module calls of a top
    level submitted job

## Discussion

Not all goals are compatible with each other, e.g. `[g.reuse]` limits
`[g.no export]` as RPC uses util-generic and boost en masse, also
`[g.transfer results]` implies serialization and inter process
communication, which too would be a lot of re-implementation.

`[g.no leak]` has been tried in the past and so far utterly failed, as
seen by rifds being left behind and thus all rts processes
leaking. One shall investigate Linux process groups. Coworker Dominik
Strassel has also reported that his cluster is really good at killing
leftovers from user jobs, and may provide input.

`[g.isolated segfault]` and `[g.no taint]` are instantly fulfilled
with `fork()`ing. `[g.isolated segfault]` means that the signals need
to be observed by the worker process though, e.g. by noticing the
socket failed and then calling `wait()` to get the signal. This may
degrade the quality of error messages though, where currently the
worker sends a log message with the backtrace. This is one of the few
important debugging tools we have, so it should be maintained as good
as possible. This implies having a signal handler in both, worker and
sandbox.

`[g.no user in worker]` and `[g.transfer results]` are kind of the
same. Essentially this means that the worker shall never load any so
and shall only transfer byte blobs (pnet value types) or exception
strings (not exception_ptrs -> top/gpispace#828). `[#gd.vmem]` `[g.no
overhead]` implies that the vmem memory is shared between all
sandboxes.  GPI-Space vmem "areas" already provide "attach" semantics,
so this should be straight forward. The transfers should probably
happen in the sandbox process though as it assembles pointers, which
will not be identical across processes. This also avoids having a
possible memory corruption in the non-sandbox process.

`[g.logging]`, `[g.errors]` and `[g.transfer results]` are slightly
interleaved: One question is whether the sandbox starts an individual
log emitter and one is who is reporting errors. Errors have to be
handled in both, sandbox and worker, as noted above with signals. A
sandbox should also be able to ask the worker to kill it after a job
termination if it knows it is tainted (e.g. after a SIGSEGV one
probably shouldn't continue). Emitting Gantt events should probably
happen only in the worker as only that has the full knowledge of if
the sandbox even survived.

The `[g.cancel.kill]`, `[g.cancel.coop]` and `[g.cancel.coop plus]`
goals are pure additions to the current implementation. They should be
treated as a separate stretch goal/task and should not be a criteria
for success of the sandbox implementation. As with `[g.policy]` they
also require user configuration, i.e. changes to the pnet
description. Of course, there shouldn't be any design decisions to
make adding that feature afterwards harder. For now one should only
implement "the straight forward one", i.e. cancel always forwards to
sandbox coop cancel and not killing, and sandbox starting may default
to per module (.so).

# Relevant code/components

Everything that is currently involved is contained in
 `src/drts/worker` and `src/we/loader`.

- `[#c.context]` `drts::worker::context` is a handle for module calls
  to query information about the worker and to implement cancel.
  - `worker_name`
  - `workers` of a co-allocation job
  - `worker_to_hostname` mapping for co-allocation jobs
  - `module_call_do_cancel` for the module to trigger cancellation
     itself
  - `[#c.context cancel]` `execute_and_kill_on_cancel` as the only way
     for the module to implement cancellation by the predefined method
     of forking and killing the forked binary. There is no way to
     customize this.
  - `[#c.context logging]` the part hidden from the user also
    implements the `cout` to logging redirection.
- `[#c.logic]` `drts-kernel` (main) and `DRTSImpl` implement the
  worker logic.
  - sets up vmem API and allocates the shared memory to be used.
  - registers with the runtime system.
  - implements the job state machine client side
  - emits Gantt messages about the job status
  - monitors `dlopen`/`dlclose` nounload taint status
  - has a thread that executes the submitted jobs:
    - has two local state machines: one for the job, one for the job
      execution, which are semi in sync, but not completely.
    - implements canceling (`[#c.logic.race]` with a race: if canceled
      before really started, cancel will not be done)
    - next layer towards execution is the local `wfe_task_t` +
      `wfe_exec_context` (in a recent refactoring partially moved into
      `src/we/type/activity.cpp` for sake of encapsulation, but still
      the same semantically). `activity_t` provides the actual
      `we::type::module_call_t` implementation as well as the
      `evaluation_context()`, i.e. the input tokens.
  - has a `we::loader::loader`, vmem API and shmem handle
- `[#c.loader]` `we::loader::loader` is essentially a `map<path,
  dlhandle>`. it ensures only a specific search path is used rather
  than `LD_LIBRARY_PATH`.
- `[#c.module]` `we::loader::Module` is essentially a wrapper around a
  function call and the registry of the functions available.
  - module calls don't define symbols for the module call
    implementations directly but instead have a `we_mod_initialize`
    symbol that registers the functions in `Module` in `loader`.
  - modules are loaded and unloaded again to check if there are
    dynamic libraries leaked, and only if they unloaded fine
    execution/loading is continued
  - module call implementations shall have the arguments
    - worker context
    - input
    - output out-parameter
    - a map of buffer name to void* for memory buffers
- `[#c.call]` `we::loader::module_call` takes over inside
  `wfe_exec_context` and translates/converts/constructs the
  information needed for the actual module call implementation call.
  - evaluates vmem buffer + get + const_put description, later put
    description based on output
  - fills shmem with the memory buffers, copies from global to local
    memory
  - sets up log redirection via the `context`
  - `[#c.call.load]` ensures the module is loaded
  - actually executes the module call via `loader` -> `Module`
  - ensures there were no dynamic libraries loaded and leaked during
    execution
  - transfers memory back to global vmem
- `src/we/loader/api-guard.hpp` contains a global
  `MODULE_DETECTED_INCOMPATIBLE_BOOST_VERION` symbol that tries to
  detect modules compiled with an unbundled boost.

## Discussion

`[c.context]` should for now stay as it is to avoid user visible API
changes. The data (participating `workers`, `worker_to_hostname`
mapping) except for `worker_name` is job specific, so it should just
be created per job. The `[c.context cancel]` cancel infrastructure of
course needs an sandbox API endpoint to be callable by the worker, but
should otherwise not be touched until the follow up stretch goal.

`[c.context logging]` also needs to be done within the sandbox to
avoid `[g.logging]` issues. This means that connection to the top
level multiplexer is going to be created *per sandbox*. This may
impose a bottleneck, so one may want to add a logging demultiplexer to
the worker in preparation for node-local demultiplexers, but more
local.

In `[c.logic]` one should also not move too much into the sandbox. The
virtual memory API and memory mapping need to be available within the
sandbox, but should be owned by the worker to avoid resource
duplication. The interaction with the runtime system and maintaining
the state machine shall stay, and thus having the knowledge about the
job state, it should also be the component sending Gantt events.

`[c.loader]` is effectively the equivalent to having the sandbox
registry. The issue here is the level at which the loaded is fed
information: While the object is within `DRTSImpl`, the modules are
only fed into the loader one line before the actual module call. This
includes being behind memory transfers. `[gd.vmem]` implies that this
needs to be the other way around, but this in turn means that either
the sandbox logic needs to be in `we::type::activity_t` or that class
needs to expose information again. Exposing specific information does
not feel like a too bad change, even if there recently was a
refactoring the other way around: The gspc19 prototype also has the
implementation information independent of the actual task description
or workflow engine, so the activity giving out implementation
information does fit that. One question to discuss is how that ties in
with `[g.policy]`: Will the policy be decided by the activity, or are
activity/task/wfe implementations required to provide information for
all policies?

`[c.module]` can stay as is. It throws a few exceptions so those need
to be ensured to be properly transported, but otherwise it should be
fine.

`[c.call]` needs to be split up. Most of it shall happen inside the
sandbox, pretty much everything but `[c.call.load]`. One should
probably defer deleting the require-(module|function)-unloads-without-rest
detection until `[g.policy]` is fully implemented: That would be the
user control for exactly that, with the default being what we
currently try to ensure by default: have one sandbox per module call
implementation (not .so).

`[#cd.rpc]` It is unclear whether the interaction with the sandbox
shall be blocking (one directional RPC calls), async (RPC calls with
callbacks on the worker) or event driven.

It is unclear how to address `[c.logic.race]`, which will still exist,
but even more pronounced due to IPC needed. A sticky flag would only
allow to prevent cancel happening strictly before run, but not cancel
and run happening at the same time and the implementation not yet
having set a handler. Maybe be sticky until a handler has been set or
the job terminated?

## Tasks

### Minimum Viable "Product"

- `[#ct.info]` collect information for `[c.context]` construction and
  job start into a single function call. hide execution and
  cancellation behind a mock RPC client interface to ensure all data
  is available.
- The interaction with `[c.loader]` needs to be moved further outside
  so that `module_call()` no longer loads a `Module` but gets one:
  That's the thing the sandbox was spawned for.
- `[#ct.bootstrap]` A top level sandbox bootstrapping needs to be
  written. Depending on `[cd.rpc]`, this may reuse
  `rif::started_process_promise` for sanity in forking. If
  `[g.policy.unsure]` is not decided with "that's bullshit", one
  should not pass the module implementation on the command line but
  let actual loading be part of the API it provides.
- `[#ct.api]` The sandbox API needs to be verified to be complete. It
  appears to be
  - `load (module info) -> handle` which loads the `Module`
    equivalent. Note: inlined into `run` if `[g.policy.unsure]` is
    discarded.
  - `run(handle, call info as per [ct.info]) -> Result
    {Success|Error|Canceled}, {TaintedPlsKill|EverythingFine}`,
    blocking until user code returns or aborts.
  - `cancel()`? There should only be one task at a time, so "cancel
    *it*" should be enough and avoids an ID generation bootstrap issue
    with `run()`. `[#ct.cancel]` This call can later be extended to
    return not `void` but a variant of `DidSo`, `JustKillMe`,
    `WontPleaseDontKill`, `WontPleaseKillInTenSeconds` or alike to
    implement `[g.cancel.*]`.
  - As the sandbox has no state besides having a module loaded, a
    `quit()` is not required: Just `kill()` it, there is only one
    client so the state is known.
- `activity_t` needs to expose information about the implementation
  again. This currently is the `we::type::module_call_t`
  implementation detail, where the vmem buffers+transfers and the
  module call implementation itself is retrieved from.
- Implement the actual logic when getting a job:
  - determine implementation info from it (via activity)
  - request a sandbox from the `loader` equivalent
    - may determine it already has one (based on "policy", i.e. module
      name)
    - shall fork a new one otherwise and load the implementation
      requested.
    - there needs to be a tracking of job id -> sandbox for cancel.
  - collect information and tell that sandbox to run the job.
  - while the above happens in the job execution thread as it does
    right now, the event thread may get a cancel request which it
    shall forward to the sandbox instance of that job. the current
    logic implementation should be fine and should not introduce a new
    race. the race whether the job was actually started already when
    trying to cancel still exists.

The above tasks form can be scaled slightly, mostly by omitting
`[ct.cancel]` and deferring a decision on `[g.policy.unsure]`:

- `[ct.bootstrap]` might be slightly easier depending on a consensus
  on `[g.policy.unsure]`, but deciding to omit (i.e. specify on
  command line, not RPC call) does not pose a big obstacle if
  re-deciding later on). Might save a bit of time (load once and at
  the start vs maintain a handle->so state), making `[ct.api]` smaller
  as well.
- `[ct.cancel]` can be omitted in the first block of work. A MVP would
  do the same as we currently do: use the `execute_and_kill_on_cancel()`
  API (yes, forking from within the fork). Adding cooperative cancel
  (that doesn't fork again), and adding killing the entire sandbox on
  cancel forms a second stretch goal.

The main task is likely in the design of `[ct.api]`, and the
refactoring needed to have the information where we need it. `[g.no
leak]` may pose a struggle as well to get right. One needs to be sure
to not degrade the current UX/functionality, so even as this is "only
the basic features, no additions", it might prove to not be entirely
trivial.

### Feature Set "Cancel"

As noted in the above discussion, the `[g.cancel.kill]`,
`[g.cancel.coop]` and `[g.cancel.coop plus]` goals are pure additions
to the current implementation.

- One needs to properly define the cancel options we want to
  support. There are currently ideas in various heads, but no fully
  written down version, not in this document either. This includes
  debating on where exactly to annotate the policy to be used, and how
  to implement cooperation.
- `[g.cancel.kill]` requires splitting the cancellation into two: The
  MVP just forwards a cancel request to the sandbox / module
  implementation. If killing is requested, this shall be replaced with
  a hard `kill()` of the sandbox process. As the MVP already has to
  handle random failures of the sandbox, one only needs to override
  failure with `canceled` and it should be done.
- `[g.cancel.coop]` is likely to reuse the existing drts context. The
  easiest implementation is a bool with condition variable for the
  user implementation to wait on and the worker+sandbox to set when
  canceling is requested. A more complex form that blends into
  `[g.cancel.coop plus]` forwards the worker's `cancel()` call to the
  implementation using an API as sketched in `[ct.api]`, adding
  communication between the three parties involved: worker, sandbox
  and user implementation. It is unclear how interactive this
  communication is.

The policies appear to be strict super sets: `kill` is `cancel()`
always returning `JustKillMe`, `cancel.coop` is `cancel()` never
returning anything but `DidSo` or `WontPleaseDontKill`. The aim for a
minimum success should likely be being able to specify "kill me" in
the pnet, and having that bool+condition variable otherwise, which is
a nicer API to what we currently provide and adds `[g.transfer
results]` as well as `[g.cancel.kill]` which are likely the most
urgent variations.

Main task is likely agreeing on the definition of the cancellation
options, as well as making sure there are no races (currently: there
is a race when a job is canceled right after it was started, and the
user code did call `execute_and_kill_on_cancel` yet).

### Feature Set "Policy"

In order to revert the "* does not unload" checks -- which turned out
to be a UX degradation, and in the Singular case even require a
GPI-Space patch -- users shall be able to specify which checks to
enable more fine grained, but likely the other way around: Instead of
disabling checks, define what worker cleanliness state is expected by
the implementation.

- Agree on policies.
- Extend pnet description XML to support setting policy (note: not
  clear where this will happen, see discussion of linker flags in
  multiple-implementation, is the policy per implementation? per .so?
  per…).
- In the worker, extend the mapping from `module_call_t` metadata to
  sandbox process to include/handle the policies:
  - per module call instance implies killing sandboxes directly
  - per module call implementation implies the mapping key to include
    the function name
  - per module is the MVP approach already
  - per user defined set of modules requires a user specified key
    (likely an expression in a property) to be included in the mapping
    key.
  - per top level job implies transporting the job id from the topmost
    agent in `SubmitJobEvent` and including that in the mapping key
    again.

Again, the main task is likely in definition of policies. The
worker/sandbox side looks straight forward, the boiler plate needed to
transport things all over the place may be tedious.

### Feature Set "Keep Context Small"

Independent of all others, `[g.no export]` can be addressed (likely
not solved in entirety) at any time. As it isn't really sandbox
specific it may even be tackled before the MVP, by starting to
re-evaluate what our public API contains, what symbols we need to
export, and investigating how we make the linker do what we want,
which is also the main task most likely.

# Notes

[#n.policy] This is not about "node local state" (as it could crash
and be restarted), but rather about `fork` and `dlopen` "safety":
`module-do-not-unload` and `function-do-not-unload` checks are the
same, enforcing that "one sandbox for all module calls of a top level
submitted job" is safe. `module-dnu` failing implies a sandbox for
sanity should do a sandbox per module (.so, group of function
definitions), `function-dnu` failing implies the worker should do a
sandbox per call instance/module function call. The user being able to
specify the policy allows us to be heuristic-free and also
override. This is a sliding scale of how "clean" a user expects an
execution context to be ("not a single function was ever invoked" to
"there may even have been completely other applications run in this
context, and whatever they left is irrelevant"), and how much overhead
we avoid by `fork`ing less.
