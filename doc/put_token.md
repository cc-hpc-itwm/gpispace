# put token: allow the workflow to progress

mark the place that shall be the function input with put_token=true

  <net>
    <place name="in" type="string" put_token="true"/>

(src/drts/test/put_token/wait_for_token_put.xpnet), then in any client that can attach to the running job, call client.put_token()

  gspc::client client (drts, certificates);
  gspc::job_id_t const job_id // result of a previous client.submit call
  pnet::type::value::value_type const bad (std::string ("bad"));
  client.put_token (job_id, "in", bad);

(src/drts/test/put_token/test.cpp) which will put the token asynchronously.

Either directly construct `pnet::type::value::value_type`s or construct them via the generated `wrap()` functions for custom/structured types.
