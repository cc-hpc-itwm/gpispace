= goal =

* redo logging infrastructure to be decentralized and more usable
* extend with functionality that applications need

== original requests by mirko ==

* decentralized
 * a group of workers sends to a "local" server
 * global server knows groups
 * multiple layers should be possible
* assume there are no cycles
* a server has a backlog (worker as well?)
* a server has 1..n parents
* tracing / profiling support of workflow by application

== open questions ==

* separate process or inside rifd?
* udp or tcp?
* push or pull? both?
* behaviour in case of log server or log source crash
* how is topology described?

= State of applications and their requirements =

== frtm ==

* every module call creates job_hook which talks to job server and
  reports start and end, reporting worker names and job class ==
  activity name
* main application maintains a map of running jobs per job class
* status query lists number of job per class
* list-nodes lists workers/hosts of jobs by class

-> could likely just operate on a tcp based gui stream as well

== dlps ==

* does frtm lite, but the reported resource is an id, rather than
  class + workers
  * tracked id luckily is in set of inputs for activity

-> needs ability to track inputs in gui stream in addition to name

== gitfan ==

* counters for benchmarking/debugging: todo what is counted?
* lastWill to output something on module call termination or worker
  shutdown
* macht derzeit eigene log files damit er nach worker aufschlüsseln
  kann

-> for timings, intercepting the gui stream is fine. for more fine
   grained timings, we either need to provide something via
   drts_context (or some global static state). apparently a map name
   -> counter/data/T would be enough. I am personally not sure that we
   should do that. A generic implementation would be some worthless
   key value store that is at some point dumped. An application can
   just as easily do that itself. Since we don't want to do global
   state, we wouldn't offer anything additional.

-> the last will on module call seems like a helper that can be
   provided. it is probably just a scoped thing that logs the last set
   value on destruction and can be created in each module call.

-> splitting logs into files per worker is something we need to make
   possible. e.g. by source / by host / …

== smartb ==

In contrast to all other applications, smartb also logs to our server
outside of the workflow. We should probably still allow for that in
our interface. We currently do so since we expose pretty much all of
fhglog. The API is not really intended for public use and might thus
be improved, e.g. by doing the parameter parsing properly rather than
hiding env+argv parsing in a macro that is given an io_service.

This use also indicates that applications likely want to copy a stream
of all log messages to their application server as well, so just like
the gui stream mirroring, we should allow for log mirroring as well.

-> provide log-writing API outside of module calls

-> provide log-reading API at all

= outputs from talking to people =

* Lukas has experienced package drops in the GUI stream, and thus one
  has to assume that this too happened in logging as well.
* Lukas would like to have an easy to use log-to-file, including GUI
  stream. He currently does it himself by using netcat. Dirk would too
  like to log to file, and the GUI at the same time.
* Lukas would like to have the call site based level filtering back to
  have debug logging only happening when debugging
* Lukas wishes for gantt replayability
* Lukas would like to have more information in gantt. He would like to
  see what activity name, input is executed.
* Lukas, Dirk and Mirko think that having expressions in gantt is
  overkill. They all would like to be able to trace transitions,
  independent of function type. Mirko suggests to allow specifying a
  tag for those traced transitions. Dirk and Lukas would also like to
  use logging inside expressions.
* Dirk is logging into a node local file when debugging. He says that
  this is due to our logging sometimes getting lost in the void if a
  crash comes right after logging. This might be due to using
  UDP. Waiting for an ack when logging would be possible when logging
  to a local service only.
* Dirk can imagine that splitting into multiple files might be useful,
  and thinks of rules like one per job. A job here is an aloma job,
  not an execution of a job binary, which means that this
  configuration would not be startup time.
* BeeGFS agrees that a local unix stream socket is fast enough to do
  logging synchronously to a local service. This would make it
  possible to ensure messages are not dropped.
* BeeGFS suggests using syslogd and not reinventing the wheel.
* BeeGFS suggests keeping both APIs, the cout redirection and the
  macros for expert users. Lukas wanting to use levels suggests he is
  fine with using the macros.

= existing (relevant) issues related to logging =

* #715: allow user to nicely configure, filter and redirect logging:
  about bootstrapping though. Should probably also use the redesigned
  log infrastructure then and let the application define a sink
  (currently std::cout)
* #630: complains about GUI message drops
* #432: requests more fine grained tracing (memory transfers)
* #257: log design: new points here are
  - How to ensure thread safety?
  - What about asynchronous logging?
  - at-runtime modification via control messaes
* #174: categorize log messages: is about what we log, not how we log
* #51: restrict file size of file sink / log rotation
* #49: add categories for application <> runtime separation

= existing solutions =

== rsyslogd ==

For the purposes of logging with our requirements, syslogd sounds
absolutely perfect. It supports aggregation, forwarding, local
logging, files, filters, categories, … but has the one drawback of
being focused on logging, while we need a solution that does event
delivery for the tasks as well. We could hack that onto the logging
system again and just use a category to separate from "real"
logging. It does not work well with being independent from the system
though. It would bind us to Linux (even BSD is doing it differently
already, of course), and from what I have gathered, starting a
second syslogd world independent from the system one is quite a
pain. It is also superseeded by systemd-journald, of course, so
probably not even available everywhere.

-> don't use

== rabbitmq as brokering infrastructure ==

We could avoid having to redo the aggregation infrastructure ourself
by relying on a third party solution like rabbitmq. rabbitmq has
support for queuing of events, adding topics, and routing them
nicely. Since it is a generic message queue rather than a logging
framework, support for our GUI message stream is also included.

Since it is a generic message broker and we essentially only need
aggregation of streams in a tree, it does feel like overkill. Also,
external dependencies is something we want to avoid.

there exists software built on top of log4j and rabbitmq to do
application logging in a cloud environment, so it is possible and has
been done. I have not done deeper investigation into it

-> probably don't use. seems like overkill, needs more investigation
   if interest exists.

= discussion =

Since the existing solutions or my research into them has shown that
using them is probably not what we want, we're back to doing
everything ourselves. Let's discuss individual topics:

== general structure ==

Modeling after syslogd, an approach where

* per host a local process is started: a log server. a log server has
  a backlog and can be communicated with a (fast) unix domain socket.
  since it should be fast enough to even support synchronous messages,
  a client does not need another backlog.
* processes on that host log to the local log server. I think we
  should have a local log server per host and not bother with
  topologies where the first hop is over the network already.
* the per host servers are connected up so that their sink is the next
  higher server. this means that at the top the machine where the
  application binary is running will essentially become the log
  server where everything is aggregated to. alternatively, one can
  stop at the "safe machine" where the agent is running on.
* sinks should probably be configurable at any level, e.g. file sinks
  being written on a per host basis if the output should be per host.

* to be reliable, tcp should be preferred over udp.
* to be reliable, it should be a separate process over putting stuff
  into rifd.

=== push or pull ===

we want to have multiple sinks. in a pull model, that implies the
number of sinks is known so that after the last pull, data can be
erased. otherwise we'd always grow which is obviously not possible.  a
push model has to know the sinks to begin with, so essentially the two
models are the same. pull needs registration and means in the case of
a sink registering but never fetching data (e.g. because it crashed)
we would just overflow. a push model would see that it was unable to
push the data, would possibly retry, but at some point just declare
the sink dead and stop pushing there, knowing that discarding data is
possible.

Based on this I would go for push. The criteria for pushing should
probably be timeout || backlog_full to reduce traffic but stay
somewhat real time. It should of course be possible to configure it to
just always push through and never cache == backlog_size=0.

If a sink is unreachable, it shall be retried for the configured
duration / times, and then dropped. If it was crashed and reappaers,
it shall re-register. For "nice" crashes, we will notice the crash and
automatically unregister (e.g. tcp connection close).

== <insert section title> ==

* currently gui messages abuse the logging framework more than they
  use it. this means that they still have fields like file and line
  even though that is never looked at and bogus (it is some
  drts-kernel implementation detail). It feels like a message should
  have this kind of information in a key-value store instead. for log
  messages this would be

  - level
  - host, pid, tid
  - timestamp
  - MISCONCEPTION: line, file were dropped at some point
  - message

  for GUI messages we instead need

  - host, pid, tid
  - timestamp
  - event type (start, cancel, fail, …)
  - activity name
  - activity id
  - inputs

  Thinking about it again, it seems like level is the only thing that
  GUI messages don't have while the stuff they have extra is just
  their message.

  -> Misconception: just keep as is, make them use the same event type.

* the topology description in the first step should probably be
  mirroring the one of our workers. it works there and it doesn't seem
  like we have a bottlenet with that currently. this means that per
  host 1 server is started, who all log to the server on the agent
  host. extra sinks should be easily configurable with --log-file
  (adding a global file sink on top host), --log-file-prefix-per-host
  (adding a local file sink per host), --log-file-prefix-per-source
  (adding a local file sink per host splitting by source =
  worker). Additional sinks (e.g. the application wanting a copy of
  the stream) do not need to be known at startup time and are thus
  irrelevant to the topology description.

== logging API ==

* provide std::c** bound logging that falls back to a given channel
  and log level. in module calls automatically provide that.

* provide a logger that can communicate to a host-local log server via
  socket. a category and level should be set per message. a level
  filter should be set per logger to discard lower levels already.

* on xml level, add the ability to tag transitions with
  `emit_trace_events:bool,default=false` to have expressions show up
  in gantt just like module calls. since those events would include
  the activity name and inputs like module calls would, a "tag" is not
  needed since it can only be something derived from that to begin
  with. also, add `traced_inputs:list<string>,default={}` and
  `traced_outputs:list<string>,default={}`, which indicate which
  inputs and outputs are sent to the sink.

* on xml/expression level, add the ability to log a string. this will
  probably result in wanting to log data as well, which would open a
  hole for n-ary functions in expressions. instead, log (value_type)
  and don't care for now.

== receiver API ==

* provide sink registration to a log server. making the sink push
  protocol part of the API is probably overkill. a class that
  registers and calls a callback per push is probably enough. it
  should be possible to configure level or category filters at this
  point.

== server / implementation ==

* open a local socket to receive data
* open a socket to be queried for

== how are requirements fulfilled? ==

=== frtm + dlps ===

job hook + server are a sink for the GUI stream which on callback
maintains the number of running tasks. job hook just provides an event
for start and end of an activity providing the name. this information
is available.

=== gitfan ===

timing can be done by registering a sink for the GUI stream. I
currently don't see a reason to provide more fine grained tracing
capabilities for sub-module-call granularity. if this was added, it
should just push to the GUI log topic. I would ignore that for
now. logic that is currently implemented in his module calls will have
to be moved to the application server.

we do not provide a lastWill. It shall be done by having a scope guard
that logs on exit whatever is currently set. We should not encourage
cross-module-call state.

splitting logs into files per worker is just another way of setting up
the local sinks and provided with  --log-file-prefix-per-source

=== smartb ===

it is not yet clear to me if we can assume that a log server will also
be started on the application server host. we can probably just add
that to the minimal topology description we have. then, one can just
use our logging API to push to that, and the installed sinks will take
care of the rest.

=== misc ===

* tcp is used so drops reported by lukas and dirk or #630 are gone
* --log-to-file was added, specifying both, --gui-host and a
  --$category-to-file provides him with a copy of the stream
* call site level filter can be set per logger. we do have a
  --log-level which is currently only verbose/not for the agent. we
  can pump that api up to specify levels for sinks. I would rank this
  low priority and not do for now.
* gantt replayability is not handled but since a file sink can be
  installed for the gui stream, can be implemented at a later point
* we provide more data in the gui messages, so more information can be
  shown in gantt, including in- and output
* logging from expressions is possible, specific expressions can be
  tagged to fire gantt events

* dirks wish for splitting logs into multiple files per jobs is not
  handled. this implies dynamic sink reconfiguration on the global
  level which there is currently no other reason for. since this was a
  "nice to have", I think one can ignore it for now.

=== issues ===

* #715: is untouched. since bootstrapping is setting up the logging
  infrastructre to begin with, i don't really have a solution for
  that. the focus of that issue should probably be not part of this
  task since it is more about "what do we log to begin with?".
* #432: is not really having influence on the design since it just is
  adding more event types and emitting more events. -> defer
* #257
  - I don't know of a case where we did end up with threading
    issues yet. I suspect we could say that the log function can just be
    mutex'd. in current use cases that would never have congestion. in
    threaded use cases it would avoid the issue.
  - We decide to not do asynchronous logging since it removes the
    basic assumption "my messages do show up". with backlog and
    interval based push we do have asynchronousity on the aggregation
    side though.
  - We currently only know of one requirement needing runtime control
    measures which is the files per aloma job. this is not important
    enough as of now to invest time in designing a full blown feature
    and should be omitted imho.
* #174, #49: adding categories is trivial. loggers and thus log events
  have a category added.
* #51: we currently don't really have cases where the log files get
  too huge to actually warrant log rotation. file size limits too
  sound overkill for now. they are an implementation detail of the
  sink though, so can be added later on.
