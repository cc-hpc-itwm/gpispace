# Personal Vision for what the IML might be

## Vision

The Independent Memory layer uses and provides memory abstractions and allows applications to share data: Consistent, reliable and fast.

It is the first choice in the department and outside of the departement to couple existing alien or non-aliens applications and/or new applications or their components.

## Details

### The IML _manages_

cache/scratch space to be shared with arbitrary many clients and with arbitrary many segments. When data is transferred between the cache/scratch space and the segment, then always the IML ensures consistent data in the cache/scratch space and in the segment. The IML does not provide direct access to data in a segment in order to prevent from data corruption and to allow for sophisticated data management like improved performance using duplication or increased resistance using checkpoint copies. Segments might provide high performance and/or in-place implementations for specific operations (e.g. sort, generate) and the IML lifts the corresponding interfaces.

### The IML _uses_

an abstract segment which allows to extend the IML with more/other/new segment implementations, e.g. GASPI, MPI or bee. Segment implementations might or might not be specific to certain types of physical realizations of memory. Segment implementations provide an interface for asynchronous data transfer, e.g. split into initiation and completion. The segments might or might not be scattered and/or distributed but are presented as a single linear range of memory to the IML. The communication between the parts of a segment is managed and executed by the segment implementation. The segment implementation provides information about expected/estimated transfer costs and the IML allows clients to decide upon transfers based on expected/estimated transfer costs.

### The IML _provides_

abstract linear memory that hides the physical implementation (e.g. DRAM, NVRAM, SSD, spinning disk, ...) and presents to clients a logical view based on segments and allocations in segments. The description of segments and allocations in segments can be shared across clients and keeps its meaning to allow one client to share data with another client. Clients can transfer data between allocations and the shared cache/scratch space. Data transfers are asynchronous, e.g. the interface is split into initiation and completion. The interface to initiate data transfers makes a distinction between read-only copies and mutable copies of data and the IML uses that information to speed up the consistency management.

The interface is provided in `C++`.

### The IML _is_

#### statically distributed:

A number of IML processes is started and mutually knows each other. No IML process might join or leave. Clients and segments might join or leave any time. Clients and segments must be executed on at least one machine where an IML process is executed too in order to allow for a shared cache/scratch space based on `shmem`. If clients or segments are distributed by themselves, then it is up to them to transfer data from the shared cache/scratch space to their remote parts. It is expected that segments are distributed and clients are not.

#### a Stand Alone Tool:

- GPI-Space finds it using `find_package`.
- Self-contained documentation.
- Self-contained build system.
- Separate (github) repository for the IML process.
- Separate (github) repository for each segment.
- Minimal set of requirements (third party software, system libraries).

## Example Applications in no particular order

### IML-Shell

Is a client that provides interactive and non-interactive access to _all_ interface functions. The shell is part of the IML repository and useful for testing and debugging. It uses `libreadline` to integrate into standard mechanisms for history and search.

### IML-FUSE

Is a client that uses the kernel's fuse API to access segment data via file system operations. The first iteration ignores ownership, access control, access and modification times but provides only `create`, `remove`, `open`, `close`, `read` and `write`. Can be used for non-intrusive coupling of existing alien applications. In particular the IML FUSE will provided everything that is required to support Spectra, e.g. there might be `seek` required as well.

### IML-Storage: Immutable Blocks of Data

Deals with immutable blocks of data which allows for replication but requires _garbage collection_, e.g. distributed reference counters. All blocks have the same size. Can be used for filter pipelines in streaming applications.

## Future in no particular order and unknown relevance

### Segment: Failure Tolerance

Hidden from IML, the segment takes care by itself, e.g. using checkpoints.

### Segment: Intrinsic Operations

Like in-place sorting. The IML lifts the intrinsic operations to client level. Might use reflection or a generic API where the IML does not interpret (structures of) parameters.

### IML: Dynamically Distributed

Allow IML processes to join and/or leave. The "mutually know each other" becomes a dynamic property. To manage consistency becomes more challenging.

### IML: Failure Tolerant

Allow IML processes to fail/recover. Requires that segment failure is decoupled from IML failure, e.g. it might be the case that a failure tolerant segment can survive a node failure and a new node with a new IML process can take over the role of the IML process that has gone in the same node failure.

### IML: Remote

Use mechanisms to share data in the cache/scratch area that are not `shmem` and allow to execute clients somewhere remote.

### IML-Storage: Objects

Based on the IML-Storage for blocks implement an IML-Storage for objects. Objects must provide ways to be serialized and created from a serialized representation. Object representations might have non-unique sizes which requires more sophisticated memory management. An intermediate level is an IML-Storage for blocks of non-unique size.

### Client: Key-Value store without update

Interface on top of IML-Storage for blocks or objects. Probably different implementations when all keys/values have the same size or can differ in size,

### Client: Key-Value store with update

Does this make sense? To update a value?
