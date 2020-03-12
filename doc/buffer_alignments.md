# memory buffer alignments

In GPI-Space the local memory buffers described by the users in the Petri net 
scheme are allocated in the local memory of each worker respecting a 1-byte
alignment. However, the users have the possibility to specify for their
buffers, as in the case of buffer sizes, expressions that are evaluated to 
to positive numbers that represent the alignment to which the corresponding
buffer must be aligned. For this purpose, the Petri net xml scheme used by 
GPI-Space enables user specified alignments by providing the tag alignment 
as in the example below:

<memory-buffer name="data">
  <size>EXPRESSION</size>
  <alignment>EXPRESSION</alignment>
</memory-buffer>

The enclosed expression is evaluated to a positive integer that is the 
alignment to be respected when allocating the buffer "data" into the local
memory of the worker that executes the corresponding task/module that uses it.
Note: the alignment field is optional. In this case a default 1-byte alignment
is used. 

Examples/tests on how to use the buffer alignments are provided in:
src/we/test/buffer_alignment 