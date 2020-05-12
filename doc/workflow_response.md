# workflow response: "call a function"

in addition to putting a token, a transition needs to later down the line have a `connect-response` output which is the "return value". The response value is mapped to the "call" by the place put-tokened-to not being a simple type but a structure of value and response_id. When connect-response-ing, that token needs to be referred to in order for gpispace to do the matching.

    <struct name="value_and_response_id">
      <field name="value" type="unsigned long"/>
      <field name="response_id" type="string"/>
    </struct>
    <place name="get_and_update_state" type="value_and_response_id" put_token="true"/>
    <transition name="get_and_update_state">
      <defun>
        <inout name="state" type="unsigned long"/>
        <in name="trigger" type="value_and_response_id"/>
        <out name="response" type="unsigned long"/>
        <expression>
          ${response} := ${state};
          ${state} := ${state} + ${trigger.value};
        </expression>
      </defun>
      <connect-inout port="state" place="state"/>
      <connect-in port="trigger" place="get_and_update_state"/>
      <connect-response port="response" to="trigger"/>
    </transition>

(src/drts/test/workflow_response/workflow_response_expression.xpnet) demonstrates all these pnet side parts: `get_and_update_state` is the trigger token, which then in the transition sends `response` back `to="trigger"`. On the c++ side, `client` again provides a simple function:

  gspc::client client;
  gspc::job_id_t const job_id;
  unsigned long i (0);
  pnet::type::value::value_type const response
    (client.synchronous_workflow_response (job_id, "get_and_update_state", i));

(src/drts/test/workflow_response/test.cpp) Note how you do *not* have to generate the response-id but *only* the `value` part of the place's struct. Of course, it isn't fixed to integers, any type goes.
