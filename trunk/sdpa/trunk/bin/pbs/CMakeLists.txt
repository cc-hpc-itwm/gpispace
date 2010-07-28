install(PROGRAMS start-nre
                 start-agg
                 start-orch
                 kill-nre
                 boot-nre
				 check-nodes
				 status-nre
                 fvm-cleanup
                 init-screen
        DESTINATION
                 bin/pbs)
