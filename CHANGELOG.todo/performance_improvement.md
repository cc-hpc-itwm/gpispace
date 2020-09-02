## Changed
 - Modified the start_pending_jobs routine, allowing interleaving 
   task submissions with reservation releases and stopping looking up 
   for new tasks to start when all workers are served.
 - Improved the performance of embarrassingly parallel applications that are 
   generating large numbers of independent tasks.