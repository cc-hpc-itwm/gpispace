
# Worker-level memory caching


## Goal

The goal is to enable each GPISpace worker to maintain a local (private) cache of data 
blocks that have been loaded from the virtual memory and to re-use those blocks within 
subsequent tasks executed on the same worker.

## Use cases

### Superbin
In Superbin, the input data is a set of points (in a 2D grid), for which an array of
properties is recorded as a time series. A data chunk is represented by the array 
associated with one such grid point in this case. 
The application computes predictions for the properties of a set of output points, whose 
values depend on those of the neighboring input points. 
Thus, the data corresponding to each input point will be loaded by several computations. 
Having a cache of previously-needed input points would allow a worker to avoid redundant 
loads from the virtual memory.


### SPLOTCH
SPLOTCH is a ray tracing application for visualizing cosmological simulation data.  
It takes as input a set of snapshots of the universe, each of them described by a 
list of particles and their properties. Additionally, the application also relies 
on a scene description, including camera coordinates and a view plane, which are 
then used to produce a picture that renders all the input particles.

SPLOTCH uses a fixed description of the scenes to be rendered and a *seek table* 
that describes particles, both of which are loaded by all worker tasks in their 
local memory. This operation can be avoided by loading the data in the worker-level 
cache and re-using it on each task executed by the worker. 


## Proposed solution

The idea is to tag each piece of data stored in the virtual memory layer with a unique ID. 
Each worker can then locally cache the data that it transfers from the global memory based 
on the data ID and re-use the cache across tasks (i.e., between module calls). It is the 
application's responsibility to define unique IDs for each chunk of data that can be re-used. 

Such tagged data transfers can be implemented as a special case of *memory buffer* in the 
workflow. The *cached_memory_buffer* is defined by:
- a size (similar to a regular buffer), 
- a (single) *memory-get* operation to fill in the local buffer
- a unique **dataid** of the collected data chunks (implemented as a *value_type*).

    
    <cached-memory-buffer name="input_buffer" >
      <size>
      	/* size of the local buffer (same as in the case of a memory-buffer) */ 
      </size>
    
      <dataid [:not in the first iteration: name="identifier"]>
         /* some expression that describes the gathered data of this get with a value_type */
      </dataid>
      
      <memory-get>
        <global>
           /* global range defining the location of the data (tuple(handle, offset,size)) */ 
        </global>
        <local>
          ${range.buffer} := "input_buffer";
          ${range.offset} := 0UL;
          ${range.size} := ${config.size};
          stack_push (List(), ${range});
        </local>
      </memory-get>
    </cached-memory-buffer>


### Details

Possible alternatives to defining a new type of memory buffer for the tagged data:

- the dataid tag could be associated with each local data range of a *memory-get* 

    <memory-buffer name="input_buffer">
      <size>
        /* size of the local buffer */
      </size>
  	</memory-buffer>
  
    <memory-get>
      <global>
        ${id_and_slots.gets}
      </global>
      <local>
        ${range.buffer} := "input_buffer";
        ${range.offset} := ${config.offset};
        ${range.size} := ${config.size};
        
        ${range.dataid} := $some_id;
        stack_push (List(), ${range});
      </local> 
    </memory-get>

  	- Disadvantages:
   	 - the returned list in the *<local>* tags may contain multiple ranges. 
   	 For example, one call to *memory-get* could build a local buffer containing ranges (1,2) and 
   	 a subsequent call could fill the buffer with the same ranges in a different order (2,1); 
   	 while the loaded range dataids are the same, the local buffer (represented by (offset, size)) 
   	 would be different
  
 
- the dataid tag could be associated with a regular *memory-buffer* 
  
      <memory-buffer name="input_buffer">
      <size>
        /* size of the local buffer */
      </size>
      <dataid>
        /* some expression that describes the gathered data of this get with a value_type */  
      </dataid>
  	</memory-buffer>
  
    <memory-get>
      <global>
        ${id_and_slots.gets}
      </global>
      <local>
        ${range.buffer} := "input_buffer";
        ${range.offset} := ${config.offset1};
        ${range.size} := ${config.size};
        stack_push (List(), ${range});
      </local> 
    </memory-get>
    <memory-get>
      <global>
        ${id_and_slots.gets}
      </global>
      <local>
        ${range.buffer} := "input_buffer";
        ${range.offset} := ${config.offset2};
        ${range.size} := ${config.size};
        stack_push (List(), ${range});
      </local> 
    </memory-get>

  - Disadvantages:
    - this does not work in the case of multiple <memory-get> calls 
    that gather data into the same *memory-buffer* (at different offsets), 
    which would result in the same problem as in the previous case


- the dataid tag could be associated with each *memory-get* operation

    <memory-buffer name="input_buffer">
      <size>
        /* size of the local buffer */
      </size>
  	</memory-buffer>
  
    <memory-get>
      <global>
        ${id_and_slots.gets}
      </global>
      <local>
        ${range.buffer} := "input_buffer";
        ${range.offset} := ${config.offset};
        ${range.size} := ${config.size};
        
        stack_push (List(), ${range});
      </local> 
      
      <dataid [:not in the first iteration: name="identifier"]>
        /* some expression that describes the gathered data of this get with a value_type */  
      </dataid>
    </memory-get> 
    
   
   - Disadvantages:
     - same as above: multiple memory-get operations to the same memory-buffer will 
     have their own ID and location in the worker cache, whereas a *memory-buffer* 
     that combines multiple memory-gets expects a specific (contiguous) sequence 
     of data chunks. 
     
     
## Implementation steps

- add the new *cached-memory-byffer* tag to the XSD
- implement the cached memory buffer type in the parser (together with tests)
- attach the cached memory buffers to the modules they were defined into
- modify the module call to take into account the cached memory transfers 
  and to skip the transfers if the data is already in cache
- add caching mechanism to the worker implementation (drts)


