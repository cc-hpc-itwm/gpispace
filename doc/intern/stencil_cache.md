# Goal

Support ALOMA (and Superbin) with fast cache management.

## References

- Superbin: http://gitlab.hpc.devnet.itwm.fhg.de/gspc-applications/superbin
- Superbin talk: http://gitlab.hpc.devnet.itwm.fhg.de/gspc-applications/superbin/blob/develop/doc/talk.pdf
- ALOMA: http://gitlab.hpc.devnet.itwm.fhg.de/aloma/core
- ALOMA stencil spec: http://gitlab.hpc.devnet.itwm.fhg.de/aloma/core/blob/stencil/src/aloma/workflow_generator/stencil/spec.md
- GPI-Space workflow level cache library http://gitlab.hpc.devnet.itwm.fhg.de/top/gpispace/tree/develop/share/lib/cache
- GPI-Space workflow level cache library addition http://gitlab.hpc.devnet.itwm.fhg.de/top/gpispace/commit/fb2870c005622c4f058bcea833b1556ec78551c8

## Situation

ALOMA is about to add support for Stencil-like applications.

Superbin serves as a prototype for Stencil-like applications. As described in the "Superbin talk" the application speeds up a stencil computation by using the virtual memory as I/O cache.

The loop

```
foreach p in set of output points:
  D := dependency (p)
  L := {}
  foreach d in D:
    L += location (load (d))
  store (location (calculate (L)))
```

(talk slide 2) implements the uncached stencil operator.

- `D` is a set of "neighbors" of the output point `p`.
- Each neighbor is loaded into memory and its memory location is recorded in the set `L`.
- When all neighbors are loaded, then `calculate` computes the value of the output point that is then stored.

This simple loop is trivial parallel but reads the input points multiple times if they are in the neighborhood of multiple output points.

The addition of Superbin is the usage of the virtual memory as I/O cache which leads to the algorithm

```
foreach p in set of output points:
  D := dependency (p)
  L := {}
  foreach d in D:
    l := get_location_and_increment_refcount (cache, d)
    unless filled (cache, l, d):
      load (d, l)
    L += l
  store (location (calculate (L)))
  foreach l in L:
    decrement_refcount (cache, l)
```

- The location and the number of references of loaded input points are recorded in a `cache`.
- The cache effectively avoids repeated I/O operations for a number of scenarios pictured in the talk.

The implementation of Superbin led to the addition of the workflow level cache library into GPI-Space. This library adds a token type that is capable to maintain a mapping between "cache slots" and their content, described as a "data id". In the library the possible states for cache slots are

- "EMPTY": The slot has no known content. Initial state.
- "AVAILABLE": The slot has a known content but is not referenced.
- "FILLED": The slot has a known content and is referenced.
- "STARTED_TO_FILL": The slot is referenced and the workflow is about to fill the slot.

The "STARTED_TO_FILL" state reflects that I/O is asynchronous: After a slot has been assigned to a certain data id, the I/O takes some time to read the data and fill the slot. If, during that time, the same data id is requested again, then the later request(s) will be assigned with the same slot and get the information that the slot has the state "STARTED_TO_FILL". This way the first assignment (which got the information that the slot has the state "EMPTY") can start to fill the slot while the later assignments can avoid to start the same I/O operation again. Once the slot has been filled, its state changes from "STARTED_TO_FILL" to "FILLED" and all clients need to get that information.

## Why not stick with the workflow level cache manager?

The number of cache slots is not under control of ALOMA and might become quite large. For example if 50 machines are using 50 GiB each and a single cache slot must hold one Gather of size 4 MiB, the number slots is 640000 already. Superbin hit cache management limitations already when using 625 cache slots as seen in the first plot on slide 8 of the Superbin talk.

Why is it limited:

1. The size of the token is proportional to the number of slots. The workflow engine copies tokens whenever they move.

2. The content are `set`s and `map`s with sizes proportional to the number of slots. As tokens are immutable there are no `O(1)` operations but all operations are at least `O(n)` due to the required copy.

So after all the cache state is better considered being data rather than meta-data.

## Where to keep the cache data if not in a token?

Obvious answer would be "in the virtual memory". However this has disadvantages too, namely that the copies are not eliminated but replaced by copies to/from virtual memory and that all operations must go through the scheduler loop and be executed in some (available) worker. Not even talking about serialization and dynamic size.

The other place is to keep it directly in the workflow engine and provide a way for the workflow to modify this cache information.

## Proposed solution

### Zero copy mutable cache data: Keep it in the layer

To keep data in the workflow engine is not the full answer: Does it mean in the single threaded core `we::type::net_type` or in the thread safe wrapper `we::layer`? At first glance the single threaded core seems to be the right place. However, this state is serialized and transported across the system. This would imply another point where the cache data is copied around. Moreover the cache management wants to update the cache data concurrently. The reason is that a) there is the asynchronous I/O and b) group allocations for complete neighborhoods might block half way and continues when concurrent mechanisms report cache slots as free again.

To keep the cache data in the thread safe `we::layer`

- allows for zero copy and mutable cache data AND
- allows for concurrent updates.

Similar to the solution for workflow responses the thread safe `we::layer` might provide the single threaded core with callback functions.

Note: To keep data for multiple caches is as simply as turning from `T _x` to `std::unordered_map<key, T> _xs` where `key` is defined by the workflow.

### Workflow level modification: Define "magic" expressions

To enable the workflow to modify the cache data specific transitions are required as this is the only interface that connects those two levels. One option would be to define a new class of transitions, e.g. "stencil_cache" transitions. That would imply to touch all places where currently "module" and "expression" has meaning and add meaning for "stencil_cache". In the long term this seems to be the right solution.

However, in the short term there is a less intrusive option: Use properties to annotate existing expression transitions. Support for parsing and transporting properties across all levels (from `xpnet` down into `we::type::transition`) is available. To use expressions avoids the scheduler loop and is possible as the current workflow level cache management uses expressions too.

The proposed key for the "magic" stencil cache expressions is "fhg.we.stencil_cache". There are two ways to distinguish the several cache operations and transport their parameters: The property tree or the expression itself can be used. To use the expression itself has the advantage that no additional expression evaluation is required and that the "magic" stencil cache expressions behave like normal expressions _plus_ the ability to modify the cache.

The dynamic cost for "magic" expressions include to determine whether or not the properties contains the "magic" key. This needs to done by every call to `fire_expression`. A specific type might safe that cost and move it towards network creation time. However, currently this is not done for "expression" versus "module" either.

### Bring it all together

To summarize the proposal includes to:

- add mutable cache state(s) to `we::layer`
- use annotated expressions to modify the cache state(s)
- use the existing mechanisms for properties for the annotation

## Low level structures

Got you, but what is included in the mutable cache data and what are the operations?

### Single slot "stack cache"

Starting with a data structure that has no meaning for stencil but only manages the assignment and references between slots and their content.

For that lets start with a "stack cache" manager. That structure provides a very simple interface, namely:

```
struct StackCache
{
  StackCache ([Slot..Slot));

  struct Allocation
  {
    enum State {Emtpy, Assigned};

    Slot slot;
    State state;
  }

  Allocation alloc (DataID);
  void free (DataID);
}
```

- the constructor tells the `StackCache` about the available cache slots
- the cache slot states have the meaning:
  * "EMPTY": The slot has no known assignment. Initial state.
  * "ASSIGNED": The slot has a known assignment and is referenced.

- the semantics is that the very first allocation returns "EMPTY" and later allocations return "ASSIGNED" as the slot state. Each allocation increments the reference counter and each free decrements it. When a slot is no longer referenced, the assignment is simply forgotten:

```
StackCache cache ([0..1));
DataID data (anything);

REQUIRE (cache.alloc (data) == (slot: 0, state: Empty));
  REQUIRE (cache.alloc (data) == (slot: 0, state: Assigned));   // 1
  cache.free (data);                                            // 1
  REQUIRE (cache.alloc (data) == (slot: 0, state: Assigned));   // 2
    REQUIRE (cache.alloc (data) == (slot: 0, state: Assigned)); // 3
  cache.free (data);                                            // 2
    cache.free (data);                                          // 3
cache.free (data);

REQUIRE (cache.alloc (data) == (slot: 0, state: Empty));
```

- Note how this works with any kind of overlapping usages. The commented numbers picture the relation between `alloc` and `free`. So "Stack" in fact is a kind of "weak interleaving stack" and not a "strict stack".

- However, if the usage of a certain piece of data happens in non-overlapping phases, then the `StackCache` lacks efficiency. Note how the last `alloc` gets the information "EMPTY", event though the slot was not used for any other data.

- The `StackCache` implements a certain assignment strategy, e.g. in

```
StackCache cache ([0..2));
DataID a,b (anything and pairwise different);

REQUIRE (cache.alloc (a) == (slot: 0, state: Empty));
REQUIRE (cache.alloc (b) == (slot: 1, state: Empty));
```

might fail for certain assignment strategies. The non-failing test would be

```
StackCache cache ([0..2));
DataID a,b (anything and pairwise different);

auto A (cache.alloc (a));
auto B (cache.alloc (b));

REQUIRE (Set{A.state, B.state} == Set{Empty});
REQUIRE (Set{A.slot, B.slot} == Set{0,1});
```

- The `StackCache` partially solves the requirement of the stencil workflow: Only one allocation gets the state "EMPTY" and later allocations can avoid to initiate the same I/O operation again.

- Note how the `StackCache` has another problem: While it can be used to tell clients whether or not the asynchronous I/O has been _started_, there is no way to get the information whether it has been _completed_.

### Single slot cache

To address the shortcomings of the `StackCache` the `Cache` introduces a third state and one more interface function:

```
struct Cache
{
  struct Allocation
  {
    enum State {Emtpy, Assigned, Remembered};

    Slot slot;
    State state;
  }

  Allocation alloc (DataID);
  void free (DataID);

  void remember (DataID);
}
```

- When only `alloc` and free are used, then the `Cache` behaves exactly like the `StackCache`.

- the cache slot states have the meaning:
  * "EMPTY": The slot has no known assignment. Initial state.
  * "ASSIGNED": The slot has a known assignment and is referenced.
  * "REMEMBER": The slot has a known assignment and is not referenced.

- The method `remember` can be called for data that is assigned and, well, remembers it. It allows to escaped from the stack:

```
Cache cache ([0..1));
DataID data (anything);

REQUIRE (cache.alloc (data) == (slot: 0, state: Empty));
  cache.remember (data);
cache.free (data);

REQUIRE (cache.alloc (data) == (slot: 0, state: Remembered));
```

- The `Cache` now allows to observe the _completion_ of asynchronous I/O like here:

```
time   client 1               client 2                 client 3
0      alloc (d) -> Empty
       start_async_IO (d)

1                             alloc (d) -> Assigned

2      finished_async_IO (d)
       remember (d)

3                                                      alloc (d) -> Remembered

4      use (d)                ?                        use (d)
       free (d)                                        free (d)
```

- "client 3" and "client 1" work well together. Once "client 1" told the cache about the completion of the I/O operation, the later allocation receives that information and can immediately use the data.

- The situation if different for "client 2" though. When allocating it receives the information that the I/O has been started but there is no simple way to use the cache to receive a notification about the completion of the I/O. This problem will be solved with the [stencil cache](#stencil-cache) below.

- But before doing that let's realize that the `Cache` (unlike the `StackCache`) needs to implement an eviction strategy. The `StackCache` either has available slots or not. The `Cache` might have no empty slots but slots available that are remembered and needs to evict one of those in order to satisfy a request for a new data id:

```
Cache cache ([0..2));
DataID a,b,c (anything and pairwise different);

cache.alloc (a);
cache.alloc (b);
  cache.remember (a);
  cache.remember (b);
cache.free (a);
cache.free (b);

REQUIRE (cache.alloc (c) == (slot: ?, state: Empty));
```

Note the question mark: The eviction policy (and not the assignment policy) decides which slot is returned.

### Forgetful single slot cache

Sometimes clients know that a certain piece of data will never be used again. It would be wise to let the cache know in order to steer the eviction. For that the forgetful single slot cache introduces a new method:

```
struct Cache
{
  struct Allocation
  {
    enum State {Emtpy, Assigned, Remembered};

    Slot slot;
    State state;
  }

  Allocation alloc (DataID);
  void free (DataID);

  void remember (DataID);
  bool forget (DataID);
}
```

The method `forget` forgets data that has been remembered and is not currently in use:

```
Cache cache ([0..1));
DataID data (anything);

REQUIRE (cache.alloc (data) == (slot: 0, state: Empty));
  cache.remember (data);
cache.free (data);
cache.forget (data);

REQUIRE (cache.alloc (data) == (slot: 0, state: Empty));
```

With the help of forget the eviction can be steered:

```
Cache cache ([0..2));
DataID a,b,c (anything and pairwise different);

auto slot = cache.alloc (a).slot;
cache.alloc (b);
  cache.remember (a);
  cache.remember (b);
cache.free (a);
cache.free (b);
cache.forget (a);

REQUIRE (cache.alloc (c) == (slot: slot, state: Empty));
```

Note how the former question mark has turned into the knowledge that the received slot is the same slot that was handed to `a` before.

### Stencil cache

The (forgetful) single slot cache almost fulfills the requirements for the stencil workflow. What is missing is:

- the notification about I/O completion for clients that assign while the I/O is running
- the allocation for groups of stencil inputs

Both are addressed in

```
struct StencilCache
{
  StencilCache ( map: DataID -> #Usages
               , callback: prepare (Slot, DataID)
               , callback: ready (OutputPoint, list<Slot -> DataID>)
               , [Slot..Slot)
               )

  void alloc (OutputPoint, list<DataID>);
  void prepared (DataID);
  void free (DataID);
}
```

which needs some explanation:

1. Constructor receives:
  * the information how often each piece of input data will be used. This requires to process the stencil in advance and to count how often each input data point is used. The main reason is that currently there is no implementation that works without that information. (The reason for that being probably just lack of skills. There is no proof such an implementation can not exist.) However, to have that information simplifies not only the implementation but also allows to make good use of `forget`.

  * a callback to start the asynchronous I/O. If data is assigned with an empty slot, then this callback tells the workflow to start the asynchronous I/O. The method `prepared()` is used by the workflow to inform the `StencilCache` about the completion of the I/O operation. The `StencilCache` maintains a list of all clients that wait for this specific I/O operation and informs them using

  * a callback to inform waiting clients about the requested input data being available. The callback is talking (like the alloc) about complete groups of input points. The list of assigned slots is in the same order as the list when the client called the allocate, freeing the client from any duplicate book keeping tasks.

  * the information about the slot that are available, nothing special here

2. Alloc

Called to allocate slots for a complete neighborhood of some output point. The `alloc` might callback into the workflow with either `prepare (slot, data)` in case some data has not been loaded yet or even directly with `ready (output, assignment)` in case all requested input point have been loaded already.

Note: Alloc might block when all slots are currently in use!

3. Prepared

Called to inform the `StencilCache` about the completion of a certain I/O operation.

4. Free

Called to inform the `StencilCache` that a certain piece of data is not required any longer.

The `StencilCache` is made for asynchronous usage in all dimensions: Asynchronous I/O and asynchronous calls to the API. The prototype usage looks like:

```
ASYNC: on_prepare (slot, data):
  LOAD (data) into slot;
  cache.prepared (data);

ASYNC: on_ready (output_point, assignment):
  assert (data in assignment == dependency (output_point) in the same order)
  output_point == COMPUTE (data in assignment)
  foreach data in assignment:
    cache.free (data);
  --N;

MAIN:
  map:<DataID -> #usages> usages;
  Count N = 0;

  foreach p in set of output points:
    ++N;
    foreach n in dependency (p):
      ++usages[n];

  StencilCache cache ( usages
                     , on_prepare
                     , on_ready
                     , [Slot..Slot)
                     );

  foreach p in set of output points:
    cache.alloc (D, dependency (p));

  wait until N == 0;
```

The main loop processes the stencil operator to compute the number of future usages for every input point and creates the cache using the asynchronous callbacks. It then allocates the slots for all output points and waits for the computations to finish. Note that `alloc` might block when not enough slots are available. The asynchronous calls to `free` will eventually release the blocking `alloc`. Given there is enough space which is checked in the constructor of the `StencilCache`.

The asynchronous callback `on_prepare` loads the data and then informs the cache about the completion of the I/O operation.

The asynchronous callback `on_ready` computes the value for the output point. It is called with all input points loaded into the slots given in the assignment. The assignment mentions the input points in the same order as they were specified in `alloc`.

- Note that accesses to the memory are not synchronized. To put data into the memory (in LOAD) and to read (in COMPUTE) requires no further synchronization!

- Note that multiple parallel LOAD and COMPUTE are possible: The callback would put the information in shared queues and the (many) LOAD and COMPUTE clients would read out of those queues.

- Note that a place in the Petri net is the equivalent to a shared queue.

- Note that a blocking `alloc` implies that the main loop determines the order in which the slots for the output points are allocated. This is important for cache blocking. (Increase input data reuse by splitting the input field into several blocks and work block after block.)

- Note that it is straight forward to let the `StencilCache` also handle the slot assignment for the _output point_ by simply adding it to the list of the dependencies. (See also discussion in future directions.)

## Blocking and interruption

All low level data structures discussed will block in the `alloc` calls when not enough slots are available. The calls will block until corresponding calls to `free` will release enough resources to satisfy the `alloc`. This allows for straight forward usage like in the main loop of the stencil cache prototype usage. However, there might be situations where the client wants to abort the allocation and no longer wait. For this purpose the data structures provide a method `interrupt()` that has the effect:

All currently blocking calls and all future calls to any API function are throwing "interrupted".

## Future directions

### Full memory management

The `StencilCache` (and the other caches too) are talking about slots. This implies that the available memory is decomposed by the application into, well, slots. As the assignment between data and slot is arbitrary, this implies that all slots represent memory of the same size, a size that is sufficient for any piece of input data.

This approach has drawbacks if the size of the input data varies. Which is easily the case if the size to store a single output point differs from the size to store a single input point and the output point is added to the dependency of itself in order to let the `StencilCache` manage the assignment for output points too.

To help application with that topic the interface if the `CacheManager` would not only take the identifiers of the data blocks but also their size and work over a ranges of memory rather than a set of slots.

While a simple and efficient memory allocator

```
struct Memory
{
  Memory (Size);

  Offset alloc (Size);
  void free (Offset);
}
```

is almost straight forward, the equivalent with caching

```
struct CachedMemory
{
  CachedMemory (Size);

  Offset alloc (DataID, Size);
  void free (DataID);
}
```

is harder to implement because the decision space is bigger, e.g. what to evict.

However, such a `CachedMemory` is probably the way to go when thinking about solutions for fully dynamic _and_ deadlock free memory management for complex workflows.
