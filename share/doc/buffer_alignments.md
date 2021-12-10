# Memory Buffer Alignments

In GPI-Space the local memory buffers described by the users in the Petri net
scheme are allocated in the local memory of each worker respecting a 1-byte
alignment. However, the users have the possibility to specify for their
buffers, as in the case of buffer sizes, expressions that are evaluated to
positive numbers that represent the alignment to which the corresponding
buffer must be aligned. The alignments must be always powers of 2, in the
contrary case an exception being thrown.

GPI-Space enables user specified alignments by providing the tag alignment
that can be used in Petri net XML descriptions, as in the example below:

```xml
<memory-buffer name="data">
  <size>EXPRESSION</size>
  <alignment>EXPRESSION</alignment>
</memory-buffer>
```

The enclosed expression is evaluated to a positive integer that is the
alignment to be respected when allocating the buffer "data" into the local
memory of the worker that executes the corresponding task/module that uses it.

> ---
> **NOTE:**
>
> The alignment field is optional. In this case a default 1-byte alignment
> is used.
>
> ---

Examples/tests on how to use the buffer alignments are provided in:
`src/we/test/buffer_alignment`.

To align buffers requires users to provide enough space, for the size
of the buffer plus the alignment. For each buffer of size `s` with
alignment `a` the required space might be up to `s + a - 1`. If users
provide space less than `s + a - 1`, then the behavior is undefined.

As an example consider a module call that uses two buffers with size
and alignment being `(1,2)` and `(9,4)` respectively. Then the
size of the required space might be up to `(1 + 2 - 1) + (9 + 4 - 1) = 14`.

Note: Depending on the base alignment of the memory and the order of
the buffers it might be possible to fit the buffers into a space of
size less than `14`, e.g. if the memory has base alignment `4`, then
it would be possible to fit them into a space of size `11` (or `13`):

```
45678901234567
|-------|.|
|...|-------|
```

and the same is true if the memory has a base alignment of `2`:

```
23456789012345
..|-------|.|
|.|-------|
```

However, if the base alignment is `1`, then the worst case
order requires a space of size `14`, indeed:

```
12345678901234
...|-------|.|
.|.|-------|
```

and the same is true if the memory has a base alignment of `3`:

```
34567890123456
.|-------|.|
.|...|-------|
```

> ---
> **NOTE:**
>
> In any case it would be possible to fit the buffers into
> space of size `12`. However, the implementation does no attempt to
> find the optimal order and requires users to provide enough space for
> the worst case. So: If users provide space of size `14`, then they are
> on the safe side. If they provide space of size `11`, `12`, or `13`,
> then they might be lucky but there is no guarantee the space will be
> sufficient. To provide space of size `10` (which is the sum of the
> buffer sizes) is insufficient in any case.
>
> ---
