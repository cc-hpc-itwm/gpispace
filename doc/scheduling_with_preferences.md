Scheduling with preferences

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
   
Proposed solution: allow scheduling with preferences. 

This means that the scheduler should take into account apart requirements 
(common to all implementations) also a sorted list of preferences
(capabilities names).

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