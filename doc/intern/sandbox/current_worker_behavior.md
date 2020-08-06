last updated: August 2020, v20.07/08, before introducing sandbox

# main thread

- set up log emitter and segfault logging
- register SIGTERM and SIGINT to stop
- if vmem given
  - connect to local vmem socket
  - create shm segment of given size
- maybe set numa socket
- construct and start "drts plugin"
- signal startup completion
- wait for SIGTERM or SIGINT
- mark as "shutting down"
- drop any outstanding `BacklogNoLongerFullEvent` that would be sent
  once jobs are done
- request any currently executed task to be canceled, mark outstanding
  tasks as canceled-due-to-shutdown to avoid them being started
- interrupt and wait for job thread, then event thread

## "drts plugin"

Really old idea was that component have various plugins, the wfe or in
this case job execution, named "drts" for whatever reason. The plugin
boilerplate was flattened at some point, but the plugin implementation
was largely kept as-is to not break things. There have been tiny
refactorings for gantt emit, how registration is done with the agents,
and user supported cancelation was added at some point, including
asking for such cancelation on shutdown.

## construct and start plugin

- library loader is constructed, name and command line arguments and
  boilerplate is stored
- a fhgcom peer is created for communication to the agent
- another termination reason is set up: if the peer has a
  receive-failure and is not shutting down, the worker acts as if
  SIGTERM/SIGINT was sent to it
- a thread for events via the peer is started
- a thread for job execution is started
- for all given agents, a connection and `WorkerRegistrationEvent` is done
  - due to that being async/event driven, class state for outstanding
    responses is used. when getting a registration response, that
    class state is notified and the constructor eventually wakes and
    returns
  - note that it may still throw an exception that was given by the
    agent. it does **not** handle error events, but those are not
    supposed to happen in response to a registration.
  - if an agent can't be connected to/the send fails, the remaining
    agents are not contacted and the ctor fails (without waiting for
    outstanding registrations either).

During registration, i.e. the ctor, the threads for events and jobs
are already running, so technically a job can be started before the
worker signals startup being complete to the rifd. After starting the
threads no class state except for agents and registration state is
modified in the ctor though, so this should^tm be fine, even if
slightly ugly in a state graph.

# event thread

The event thread cycles until interrupted by the dtor, getting events
from the network and dispatching them to `::handlexxx()`. It supports
an agent sending
- `worker_registration_response`: requires agent to be known,
  i.e. have been a ctor argument.
- the job events submit, cancel, discover and failed/finished ack.

Any exception during handling that is not a registration response
contained exception will send an un-typed `ErrorEvent` to the agent.

# job execution thread

The job execution thread cycles until interrupted by the dtor *or*
until the worker was marked tainted. In that case it **aborts** rather
than running the dtor chain.

When handling a job, it first checks if that job has freed a slot in
the backlog queue (which is currently always 1, but was designed to be
able to take more than one, throttling the agent after filling up) and
notifies agents that previously failed to submit a job due to being full.

After that, it sends gantt events, remembers the job as being
currently executed in class state, executes it, checks for taint and
task result, sends gantt events, and sends the new job state to the
agent, not waiting for a response but letting event thread do that.

# job state machine(s)

There are multiple state machines involved, which is the main reason
this document exists, before adding more states related to the
sandbox.

First, there is the `sdpa::status` world, which is used by agent and
discover. The agent keeps track of a job state over worker submission
boundaries, so "pending" means "not submitted to a worker" while
"running" on the agent may still be "pending" on the worker. Discover
does not take this into account.

Then there is the `Job::state_t` world, where jobs can also be
canceled-due-to-shutdown, but there is no canceling state.

The `wfe_task_t::state_t` world has no "running" state, but is
otherwise equivalent to `Job::state_t`. A `wfe_task` is only created
after the job thread started.

------------

> At this point I gave up, while assembling state_machine_*.dot.
>
> So I wanted to not break anything when adding sandboxes, which by
> definition adds another state machine interacted with (as the
> sandbox is some async process), so I looked at the existing state
> machines we have in the worker, and what the hell?! I have tried to
> get a nice state machine out of this mess for a few hours now and I
> can not even remotely do that. There are at least three different
> state machines interleaved, with some transitions being locked, some
> not. For example, the attached are the "task" state machine (which
> is missing "running"!?) and the "job" state machine.
>
> The job state machine transitions depend on the task state machine,
> but are also sometimes async, leading to stuff like the job state
> being "canceled (shutdown)" and the task state "pending" (==
> running) and then the task finishing and overwriting "canceled
> (shutdown)" with "finished", unless it throws, then the task state
> never changes, but the job state is overwritten with "failed"
> regardless of the current state.
>
> Mixing that with the agent alone is already a mess and I'm impressed
> it works so well.
>
> I really fear adding another async thing here… (there is already
> three: agent, worker main thread, worker execution thread), and
> moving the entire job execution thread doesn't feel trivial
> either. I didn't want to refactor things, but I feel like I have
> underestimated the chaos :(
>
> The third graph (state-machine_worker_combined) is to show
> frustration only. You're not supposed to be able to understand
> anything. I don't either. It is probably wrong. I tried finding out
> how the task + job state machines interact and at this point I give
> up.

---------

# known issues

- `m_shutting_down` is not atomic/locked but accessed from multiple
  threads
- when canceling during shutdown, the state
  `CANCELED_DUE_TO_WORKER_SHUTDOWN` is overwritten with `CANCELED` if
  user code actually react to cancelation, leading to that being sent
  to the agent rather than nothing, which might be handled weirdly as
  no cancel was ever requested.
- when canceling on agent request, user code that ignores the cancel
  and successfully terminates is sent to the agent as canceled rather
  than success, forgetting the result/requiring re-execution
- worker taint does not properly clean up the worker. events should^tm
  be sent though as sending is sync, only receiving is async. the
  agent will try to send a jobfailedack though which will error.
- there is a race between marking a task as executing and actually
  executing the task. there is also a race between finishing to
  execute and marking it as no longer executing. this has three effects:
  - when the worker is stopping or the agent sends a cancel request,
    that cancel request may end up in the void as the context state is
    not yet set up and thus the user cancel callback is not yet
    set. the task will still execute fully.
  - when the job finished/failed, a cancel may still override the
    state to canceled, letting the execution thread throw away the
    result and overwrite with canceled instead.
  - technically this is also UB as the task state is not atomic but
    may be written by main or event thread and then read/written in
    job execution thread. one may argue that the
    `currently_executed_tasks_mutex` acquired by those threads
    introduces a synchronization primitive, but the read/write of
    execution thread does not hold that lock.
