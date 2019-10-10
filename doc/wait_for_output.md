# cleanup, prevent workflow termination with put-token

mark the top level network as wait_for_output, meaning that even if nothing can fire, it will not terminate until all top level output ports have a token. You can either do that by setting the property

  <properties name="drts">
    <property key="wait_for_output" value="true"/>
  </properties>

(test/gspc/stencil_cache/workflow/stencil_cache.xpnet) or by marking the workflow object before submission

    gspc::workflow workflow (make.pnet());
    workflow.set_wait_for_output();
    gspc::job_id_t const job_id
      (client.submit (workflow, {{"rounds", rounds}}));

(share/example/stream/test.hpp)
