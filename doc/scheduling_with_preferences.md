Scheduling with preferences

Current situation:

The workers may have capabilities and the transitions may have requirements. 
A capability expresses a specialization (coded as a string) of the worker: 
it can only execute tasks/transitions that have no requirements or require 
at least one of its assigned capabilities. A transition may require multiple 
capabilities, in which case these should be a subset of the worker's 
capabilities (specified at start time in the topology description).
A requirement is a capability requested by a task/activity.

Allowing multiple module implementations:

The current GPI-Space implementation (xsd description) allows the 
implementation associated to a transition to be either an expression or 
a single module. Implicitly, the transition's requirements are also the 
module's requirements. In the scenario with multiple module implementations, 
each module itself may require other additional specific capabilities. 
In this case the scheduler should deal with a set of common requirements 
(set by the transition) and a set of specific requirements for each module 
implementation. This informations should be sent to the scheduler by the 
workflow engine, together with the activity. 
The implementation of scheduling with preferences assumes that apart the 
transition requirements, the workflow engine also sends the list of 
requirements for each module implementation, referred to as preferences 
(from the scheduler's point of view).

Assumptions: 

1. For each computing resource type (i.e. "CPU", "GPU", "FPGA") there 
   is a number of workers started (for some resource types it can be zero)
             
2. Each activity submitted to the runtime has specified apart a 
   set of requirements an ordered list of preferences 
   (e.g. {"CPU", "GPU", "FPGA"}).
             
   For each preference in that list there should be an implementation provided 
   by the user for that specific compute resource type.

   Note: the requirements and preferences should be disjoint sets.
   Example: requirements: {"COMPUTE"}, 
            preferences: {"CPU", "GPU", "FPGA"}
   

Note: the order is important only when the module implementations have 
associated equal costs, in the contrary case the selection is driven in the 
first place by the costs. This means that even if a CPU is preferred to a GPU 
in the list of preferences, a GPU will be selected if the costs are minimized 
for this resource type. 

Implementation sketch:

The current implementation maintains the backward compatibility (i.e. for
the cases when the jobs have no preferences).

The preferences are collected by the agent at the moment of an activity 
submission (by the workflow engine), in a similar way as for the activity 
requirements. For this purpose, the activity should provide a public method 
__preferences()__ returning a list of capability names 
(e.g. "CPU", "GPU, "FPGA", etc).

The scheduler takes the preferences into account when calculating the matching
degrees (besides the requirements) and builds a map of type __matching_degree__
to __worker_information_, from which a set of workers and implementations
minimizing the costs are chosen (if no co-allocation is needed, just 1).

Compared to the actual situation, the methods used for finding optimal 
assignments return a set of worker and implementation pairs instead
of just a set of workers. Also, the agent's method for serving jobs 
takes as argument a set of workers and implementations instead of
a set of workers. 

The information about the implementation assigned to a worker is sent
to that worker together with the job id, the activity and the set of
workers allocated to that activity by the runtime.

The implementation is an optional string that should contain either one 
of the elements specified in the preference list provided by the activity
or set to boost::none if no preferences are set.
In the case the implementation is set to boost::none and the job has no 
preferences, the drts-worker should run the default module implementation
, as before. In the case the job has preferences and the implementation is not
in the preference list the worker should abort and send back an error to the 
agent (job failed).