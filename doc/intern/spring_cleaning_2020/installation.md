# what we currently install

## meta information

```
git.submodules
revision
share/gspc/licenses.txt
```

`revision` and `git.submodules` probably should be moved to
`share/gspc` or alike, to allow for system installations and to be
next to `licenses.txt`. They may be combined into one
file. `licenses.txt` *maybe* should be moved to `share/doc/gspc`.

## boost

```
external/boost/
```

is still required as long as we have `boost::` in our public API,
which we probably will have forever. The place is not a good one when
doing a system installation though, so moving it to `share/gspc` is
probably also a good idea.

## top level user binaries

```
bin/gspc-logging-to-stdout.exe
bin/gspc-monitor
bin/pnet2dot
bin/pnetc
```

are fine. We should decide on `.exe` suffix or not for consistency
though. Also, not all of these are prefixed with `gspc-` which is
something we may want to get in sync as well.

## gspc-rifd "api"

```
bin/gspc-bootstrap-rifd
bin/gspc-teardown-rifd
```

are 100% unused, but are probably fine. We could decide to fully drop
those and only provide rifd as a library (we only do that wrapped
within `libgspc.so`, not the two functions directly). Seeing nobody
ever used these binaries, maybe such an API is not needed though. The
idea was to allow for non-scoped rifds.

## internal binaries

```
bin/gspc-rifd
libexec/gspc/agent
libexec/gspc/drts-kernel
libexec/gspc/gpi-space
libexec/gspc/gspc-logging-demultiplexer.exe
```

`bin/gspc-rifd` is not something a user can use (it requires
bootstrapping to have started a server to register at). It should be
moved to `libexec/gspc` to be in line with the others. As with the
public binaries, we may want to prefix them all with `gspc-` for
consistency and `pkill -9 'gspc-*'`-ability.

## "gspc monitor"

```
bin/gspcmonc
include/fhg/util/parse/error.hpp
include/fhg/util/parse/position.hpp
include/fhg/util/parse/require.hpp
lib/libfhg-util.a
```

is unmaintained and has no clients except for an old version of the
RTM. It should probably be moved there. It only uses `fhg/util` from
this repo, which is also why we install that.

## SDK

### C++ API

Main thing to not here is that of course the `gspc/` namespace is
missing in the path. Also that namespace is inconsistent and sometimes
`fhg` or `gpi` instead.

All files installed into `include/` should contain that. Otherwise the
location is fine in general.

#### drts

```
include/drts/client.fwd.hpp
include/drts/drts.fwd.hpp
include/drts/information_to_reattach.fwd.hpp
include/drts/rifd_entry_points.fwd.hpp
include/drts/scoped_rifd.fwd.hpp
include/drts/stream.fwd.hpp
include/drts/virtual_memory.fwd.hpp
include/drts/worker/context_fwd.hpp
```

None of our applications are using these. Users don't really do that
if they want to use an API, they just include the real thing
instead. We use them in the headers to include siblings, but also
there, it isn't making too much sense. If for a `client` you need a
`scoped_runtime_system const&`, we can just include that. Keeping
compile times down a few cycles might have less effect than having a
lean API. Alternatively they could be moved to `include/gspc/detail/`
together with

```
include/drts/pimpl.hpp
```

which shouldn't be a top level member of the API if the entire point
is to make stuff private.

The remainder is the actual API:

```
include/drts/certificates.hpp
include/drts/client.hpp
include/drts/drts.hpp
include/drts/information_to_reattach.hpp
include/drts/rifd_entry_points.hpp
include/drts/scoped_rifd.hpp
include/drts/stream.hpp
include/drts/virtual_memory.hpp
include/drts/worker_description.hpp
lib/libgspc.so
```

which might be merged/split, and as noted should be moved to
namespace, but are otherwise fine. The library should probably be
renamed to `libgspc-rts.so` to start a prefix thing there as
well. Note that

```
include/logging/endpoint.hpp
include/logging/endpoint.ipp
include/logging/socket_endpoint.hpp
include/logging/socket_endpoint.ipp
include/logging/tcp_endpoint.hpp
include/logging/tcp_endpoint.ipp
```

is not really a logging API but just a requirement as we return the
`top_level_log_demultiplexer` as `logging::endpoint` in the drts API,
not in serialized form as we do for pretty much everything else.

#### xpnet interaction

There are two parts to interaction with xpnets: inside and outside of
the modules. Inside the modules, there is

```
include/drts/worker/context.hpp
include/we/loader/IModule.hpp
include/we/loader/api-guard.hpp
include/we/loader/macros.hpp
lib/libdrts-context.so
```

of which the user shall only ever use `drts/worker/context.hpp`. We
insert an include to that automatically though. The remaining three
are used in the auto-generated wrapper code. All these files may thus
be considered to be moved to `include/gspc/detail`/`libexec/gspc/`.

Inside and outside, there is also the pnet value type interaction:

```
include/we/exception.hpp
include/we/expr/eval/context.hpp
include/we/expr/token/type.hpp
include/we/field.hpp
include/we/signature_of.hpp
include/we/type/bitsetofint.hpp
include/we/type/bytearray.hpp
include/we/type/literal/control.hpp
include/we/type/signature.hpp
include/we/type/value.hpp
include/we/type/value/from_value.hpp
include/we/type/value/path/append.hpp
include/we/type/value/path/join.hpp
include/we/type/value/peek.hpp
include/we/type/value/peek_or_die.hpp
include/we/type/value/poke.hpp
include/we/type/value/read.hpp
include/we/type/value/serialize.hpp
include/we/type/value/show.hpp
include/we/type/value/to_value.hpp
include/we/type/value/unwrap.hpp
include/we/type/value/wrap.hpp
lib/libwe-dev.so
```

This API is in dire need of a cleanup for consistency and usability
from a namespace, name, and split-across-too-many-files point of
view. And again, it needs moving to `include/gspc/pnet(?)` and
`lib/libgspc-pnet.so`.

From the files listed above, the following are not really intended for
interaction by the user and no application uses them

```
include/we/exception.hpp
include/we/expr/token/type.hpp
include/we/field.hpp             # rtm: in copy of generated file
include/we/loader/IModule.hpp
include/we/loader/macros.hpp
include/we/signature_of.hpp
include/we/type/signature.hpp
include/we/type/value/path/append.hpp
include/we/type/value/path/join.hpp
```

and the following are not originally intended for public use but are
now required:

```
include/we/eval/context.hpp      # plugin API only
include/we/loader/api-guard.hpp  # required to be included when using
wrapper generated types in drivers!
```

### xpnet lib

```
share/gspc/xml/lib/memory/global/handle.xpnet
share/gspc/xml/lib/memory/global/range.xpnet

share/gspc/xml/lib/stream/mark_free.xpnet
share/gspc/xml/lib/stream/work_package.xpnet

share/gspc/xml/lib/cache/done_with_slot.xpnet
share/gspc/xml/lib/cache/fill.xpnet
share/gspc/xml/lib/cache/get_slot_for_id.xpnet
share/gspc/xml/lib/cache/init.xpnet
share/gspc/xml/lib/cache/maybe_start_to_fill.xpnet
share/gspc/xml/lib/cache/start_to_fill.xpnet
share/gspc/xml/lib/cache/type.xpnet
share/gspc/xml/lib/grid/nth.xpnet
share/gspc/xml/lib/grid/size.xpnet
share/gspc/xml/lib/grid/type.xpnet
share/gspc/xml/lib/point/type.xpnet

share/gspc/xml/lib/dup.xml
share/gspc/xml/lib/triple.xml
share/gspc/xml/lib/4.xml
share/gspc/xml/lib/5.xml
share/gspc/xml/lib/6.xml
share/gspc/xml/lib/eatN.xml
share/gspc/xml/lib/make_pair.xml
share/gspc/xml/lib/replicate.xpnet
share/gspc/xml/lib/sequence.xml
share/gspc/xml/lib/sequence/interval.xpnet
share/gspc/xml/lib/sequence/ntom.xpnet
share/gspc/xml/lib/sequence_bounded.xml
share/gspc/xml/lib/sequence_control.xml
share/gspc/xml/lib/tagged_sequence.xml
share/gspc/xml/lib/tagged_sequence_bounded.xml
share/gspc/xml/lib/trigger_if.xml
share/gspc/xml/lib/trigger_when.xml
share/gspc/xml/lib/wait.xml
```

We may want to rename `xml` to `xpnet` for consistency in our
language, but other than that this seems fine. Whether we actually
need this library is a different question. Some examples make use of
them and early applications did use some:

- adaptive-stacking: `memory/global/`
- mapreduce: `memory/global/`, `dup`, `triple`, `sequence`
- parsu: `dup`, `triple`, `4`, `sequence`, `wait`
- superbin: `grid/`, `cache/`, `sequence`, `point/`, `memory/global/`
- smartb: `stream/`

The `cache/` family (superbin) seems to be specific to that pretty
much. `memory/` and `stream/` are obviously something we need to
provide. The remainder is generic, but at the same time basic as hell,
and while some of them can be reused, people tend to forget they can
just include `dup` instead of writing the few lines themselves.

With the shift to xpnet generators, this library seems even less
relevant. We may want to strip it down to the minimum, i.e. `memory/`
and `stream/`, as that has to match the API on the C++ side.

### expression plugins

```
include/we/plugin/Base.hpp
plugin/libPlugin-Log.so
plugin/libPlugin-Tunnel.so
```

Is a recent addition to our installation used by Aloma, and should
have the two example(?) plugins moved to `share/gspc/plugins/` or
alike.

### Legacy

```
include/fhg/util/dl.hpp
```

stems from the days before util-generic and is 100% superseded by
`util-generic/dynamic_linking.hpp` with an API that only differs in
taking a `boost::filesystem::path` instead of a `std::string`. Even
the namespace is identical. Note that that file is header-only, so
`libfhg-util.a` is not required to keep this but is there for the
`parse/` headers (see ## "gspc monitor").

### start-me-via-rif

```
lib/librif-started_process_promise.a
include/rif/started_process_promise.hpp
include/rif/started_process_promise.ipp
```

was not intended to be public API, but in RTM we abused RIF to execute
third party binaries instead of GPI-Space ones. `gspc::rifds::execute()`
and `fhg::rif::client::execute_and_get_startup_messages_and_wait()`
exist for this purpose alone. With RIF maybe becoming a generic
component for sake of IML, this API might be okay, in the current
GPI-Space API it is a hack. If the goal is only to get rid of this RIF
API one could change it to be stdout/stderr capturing only, which
would be able to provide the same API in `gspc::rifs::execute()`. This
might be a good middle ground.

### seis

```
include/seis/determine_size.hpp
include/seis/do_load.hpp
seis/do_write.hpp
libexec/gspc/libdetermine_size.so
libexec/gspc/libdo_load.so
libexec/gspc/libdo_write.so
```

These are **really** old copies from @merten's seismic IO
library. They probably should be flat out removed. `parsu` is probably
the only application still having code including those, but that
should instead just get that copy (or be deleted as it is superseded
by Aloma).

## Documentation

### readme and getting-started

```
share/gspc/README.md
share/gspc/example-compute_and_aggregate/build.sh
share/gspc/example-compute_and_aggregate/compute_and_aggregate.xpnet
share/gspc/example-compute_and_aggregate/driver.cpp
share/gspc/example-compute_and_aggregate/nodefile
share/gspc/getting_started.md
share/gspc/img/GPISpace_arch.png
share/gspc/img/gpispace_example_pnet.png
share/gspc/img/petri_net.png
```

This should probably be moved to `share/doc/gspc/`.

### xpnets

```
share/man/man5/xpnet.5
```

Correct, but the content is far from xpnet and rather expression
language fundamentals only.

```
share/gspc/xml/xsd/pnet.rnc
share/gspc/xml/xsd/pnet.xsd
share/gspc/xml/xsd/schemas.xml
```

It feels like a bad idea to interleave the schema and the SDK, so
these should likely be moved to `share/doc/gspc/xsd` as well. There is
also `share/xml`, which is really rarely used, but apparently the
"standard" path to install "architecture-independent files used by XML
applications", so `share/xml/gspc/` may also be a valid target.

The `schemas.xml` is emacs specific (nXML mode) and maybe should not
be installed right there, maybe shouldn't be installed at all, or
should be installed to `share/emacs/site-lisp` instead (GNU wants you
to do that, butthat requires some elisp to be added as well).

### monitor

```
share/gspc/gspc-monitor/gspc-monitor-gantt-clear.png
share/gspc/gspc-monitor/gspc-monitor-gantt-columnops.png
share/gspc/gspc-monitor/gspc-monitor-gantt-filter.png
share/gspc/gspc-monitor/gspc-monitor-gantt-header.png
share/gspc/gspc-monitor/gspc-monitor-gantt-legend.png
share/gspc/gspc-monitor/gspc-monitor-gantt-namefilter.png
share/gspc/gspc-monitor/gspc-monitor-gantt-nomerge.png
share/gspc/gspc-monitor/gspc-monitor-gantt.png
share/gspc/gspc-monitor/gspc-monitor-log-bottom.png
share/gspc/gspc-monitor/gspc-monitor-log-save.png
share/gspc/gspc-monitor/gspc-monitor-logging.png
share/gspc/gspc-monitor/gspc-monitor-overview.png
share/gspc/gspc-monitor/gspc-monitor.md
```

As with the above, should probably be installed to
`share/doc/gspc/monitor` instead.

## Bundling

```
libexec/bundle/info/agent
libexec/bundle/info/drts-kernel
libexec/bundle/info/gpi-space
libexec/bundle/info/gspc-bootstrap-rifd
libexec/bundle/info/gspc-logging-demultiplexer.exe
libexec/bundle/info/gspc-monitor
libexec/bundle/info/gspc-rifd
libexec/bundle/info/gspc-teardown-rifd
libexec/bundle/info/gspcmonc
libexec/bundle/lib/
```

In case we don't build with `cmake -DINSTALL_DO_NOT_BUNDLE=on`, we
copy libraries to `libexec/bundle/lib`. `libexec/bundle/info` contains
lists of bundled libraries per target for the sake of re-bundling
GPI-Space partially. These can't be removed, but should probably be
moved to `libexec/gspc/bundle` instead.
