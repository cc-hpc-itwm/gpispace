# state of documentation

## API in-header

The only in-header API documentation is

- `we/plugin/Base.hpp`: Preconditions/guarantees are documented, but
  neither usage nor what function/type does what. There is a document
  about the API in `doc/expression-plugins.md` though (not
  installed).
- `rif/started_process_promise.hpp`: a side note on why the ctor
  arguments are a reference, nothing else

and additionally we have

- `we/expr/token/type.hpp`: operator precedence of the pnet expression
  language. Users never interacts with this file themselves though, it
  is a two level indirect dependency of `we/field.hpp`, which itself
  is an implementation detail of the generated wrapper (but it is
  "used" by rtm as that copied that generated file into the source tree).
- `drts/scoped_rifd.hpp`: two `\todo` comments
- `we/type/literal/control.hpp`: a `\todo` comment
- `we/loader/macros.hpp`: a note about an implementation detail
- `logging/*`: three `\todo` comments about being part of `fhg::rpc`

and sadly in the "we shouldn't even install this" part

- `fhg/util/parse/require.hpp`: pretty much fully documented
- `seis/*`: long argument names as comment instead of name

so one can effectively say: There is no documentation of the public
API in-header.

## meta

`/README.md` and `doc/readme` are installed and contain an overview
about the system, how to build it, and how to build an application.

`doc/whitepaper` contains an overview document. It is very meta and
the only connection to GPI-Space is the concepts and old screenshots
from the (long deleted) pnet editor and some iteration of parsu.

## user binaries

- `share/gspc/gspc-monitor/` gives an overview of the monitor GUI. It
  is quite old but was recently updated to match new logging command
  line arguments. (installed!)

## drts

- `doc/information_to_reattach.md`: short snippet of how to use
  `gspc::information_to_reattach`. example is available only inline,
  there is no equivalent test. links to source tree but everything
  linked is available in installation. not everything used in example
  is though (`read/write_file()`).

## xpnets

- Expression language has a man page (installed!).
- An XML schema for .xpnet exists and is installed.

- `doc/buffer_alignments.md`: memory alignments in pnet vmem
  buffers. Has example in `src/we/test/buffer_alignment` which
  document refers to but does not inline.
- `doc/connect-out-many.md`: split-into-multiple-tokens output
  connections. Has *modified* copy of inlined example in
  `src/we/test/put_many/`.
- `doc/expression-plugins.md`: overview of
  `include/we/plugin/Base.hpp` with inlined example
  `src/we/plugin/test/B.cpp`. Refers to the source tree multiple
  times and is partially unclear about target audience (GPI-Space devs
  vs users).
- `doc/partial_cross_product.md`: description of an optimization that
  is made. example is abstract, not a xpnet.
- `doc/put_token.md`: very basic, effectively is only an
  example. example is inlined only, but there are tests using it, and
  an example for `stream` includes putting tokens. The test is
  referenced in the document.
- `doc/wait_for_output.md`: very basic, like
  `doc/put_token.md`. Again, has test that is referred to from the
  document.
- `doc/workflow_response.md`: slightly longer than `doc/put_token.md`
  but same style and properties. Again, test exists and is linked.

### tutorial

We have `doc/tutorial` which contains demo petri nets and a slide
deck. The slide deck contains an assignment of key words to demo and a
screenshot of the pnet2dot-ed demo pnets and sometimes a copy of the
xpnet (which are somtimes **not** in sync!). The slide deck also
refers to pnets that are not in the tutorial folder, while some of the
tutorial folder pnets are not referenced in the slide deck.

Some of the demos not in the slide deck do have a sketch of a readme
next to them. In general, the entire collection is more of a suite of
tests and notes to a presenter than a tutorial.

The order of the following subsections is the apparently intended
sequence, based on slide deck and how they cross-include each
other. After that, in no specific order, the remainder.

Except for the two readmes and the xpnets themselves, this is
effectively the entire information that we have about these, which
indicates that this is in dire need of a lot of explanatory text to be
added. The tutorial itself seems good, it of course isn't clear what
detail needs to be where, but the flow is okay. The "build system"
part is the most outdated, the "getting started" document is more
advanced there.

All of these are tested, which is good.

#### hello_world
two demos
- "hello_world" (a single module call, "function, port, types,
external module call, build system") and
- "hello_many" (#tokens many module calls, "subnet, connection, re-use
of existing net, parallel execution, put")

- hello_many
  - includes hello_world
- hello_world

- no readme
- in slide deck: xpnet "hello_world" (modified), xpnet "hello_many",
  pnet2dot "hello_world", pnet2dot "hello_many"

#### sum
two demos, "sum_many" and "sum_expr_many". "expressions, user defined
types". only version with expression in slide deck.

- no readme
- in slide deck: xpnet "sum_expr", pnet2dot "sum_expr_many"

#### sequence
two demos: "sequence" ("conditions, inout") and "use_sequence"
("(no)inline"). variation "sequence_forward" that is used by other
demos only. "use_sequence" is also included in other demos.

- no readme
- test for "use_sequence" and "sequence_forward" are only implicit by
  being included in other demos
- in slide deck: pnet2dot "sequence", pnet2dot "use_sequence"

#### template
three demos: "dup" ("template"), "three" (dup but 3x) and
"work_and_wait" ("use template").

- no readme
- test for "three" is implicit by being included in other demos
- in slide deck: xpnet "dup", xpnet snippet from "work_and_wait" (how
  to use "dup", pnet2dot "work_and_wait"

#### virtual
"virtual places"

- no readme
- in slide deck: pnet2dot "work_and_wait"

#### credit
two demos, "work_and_wait" and "work_and_wait_credit"

- no readme
- not in slides

#### n_of_m

- no readme
- not in slides

#### parallel_inorder

- no readme
- not in slides

#### atomic
"accessing a counter atomically"

- has readme sketch (outdated in terms of build system)
- not in slides

#### avg_stddev
"process a file in parallel (no vmem, just multiple calcs working on a
subrange of the input file)"

- has readme sketch (outdated in terms of build system)
- not in slides

#### hello_world_multiple_implementations
a copy of hello_world created during the multiple-implementations
feature branch.

- no readme
- not in slides

### share/example

In addition to the tutorial we also have some additional (also
not-installed) examples. These too are all tested.

The documentation part is even more lacking though. For 28 files of
code (excluding tests), there is a whopping one comment: why an
implementation was left empty (for sake of it being a performance
test). One of the tests has two todos and one note about an API
function otherwise being unused.

Getting this folder ready for public consumption will take quite some
time to understand the examples to begin with: They are more than 2k
lines of code.

# todo

- what exists in course?
