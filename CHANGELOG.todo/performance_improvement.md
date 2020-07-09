## Changed
 - Modified the start_pending_jobs routine, allowing interleaving 
   task submissions with reservation releases and ensuring that the
   the lookup for new pending tasks to start stops when all workers are served.
 - Improved the performance of embarrassingly parallel applications that are 
   generating large numbers of independent tasks.
 - Improved the performance of aloma core 4D tests when the number of used lanes
   is unlimited.
 - The performance of aloma core 4D tests is better when using unlimited number
   of lanes, compared to the case when using a limited number, when using up to 16 nodes.