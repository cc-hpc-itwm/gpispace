## Demonstrate the use of priorities:

The Petri net allows to assign "priorities" to transitions. Priorities are integral numbers from the range `[0,2^16)` and are used to resolve "conflicts". A "conflict" describes the situation when two transitions `t1` and `t2` are both enabled but to fire one disables the other. This happens when transitions share input places.

If `t1` and `t2` have no priorities assigned or they have the same priority assigned, then GPI-Space randomly fires one of the conflicting transitions (and thereby disables the other one).

If, however, the transitions `t1` and `t2` have different priorities, then GPI-Space fires the transition with the higher priority.

The folder contains two examples:

1. Return the sum of all inputs provided. This test uses priorities to select between multiple enabled transitions: If there are two arguments provided, then all transitions are enabled and the sum has the highest priority. If there is one argument provided, then the priority is to take that argument. Only if there is no argument provided the value zero is returned.

2. Combine priorities and conditions. Make sure the priorities are only relevant for transitions that are enabled. For that always provide inputs to all ports of all transitions. The transitions have overlapping conditions and are ordered by their priorities.
