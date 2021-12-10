# Asynchronous Workflows

In some cases, not all inputs to a workflow are known and/or available at the time of submission (e.g. sensor data streaming).
GPI-Space supports the submission of asynchronous jobs and several features allowing applications to communicate with a running workflow.

## Submit Jobs

To submit an asynchronous workflow the `gspc::client` has to use the `submit` method instead of `put_and_run`.
`submit` launches a non-blocking workflow and returns a job id.
That handle is passed with every future client operation interacting with that submitted workflow.
The lines below show how to launch a non-blocking workflow.

```c++
...
gspc::client client (...);
gspc::job_id_t const job_id = client.submit (workflow_object, values_on_ports);
...
```

## Put Token

The `put_token` feature allows an application to place additional tokens into a marked Petri net place of an asynchronous workflow.
A place is marked by setting the attribute `put_token="true"` in the Petri net declaration:

```xml
<net>
  <place name="input_data" type="int" put_token="true"/>
  ...
</net>
```

After launching such a non-blocking workflow, additional tokens can be passed asynchronously by a `gspc::client`'s `put_token()` method.
A token is created by directly calling `pnet::type::value::value_type`'s constructor or by calling the generated `wrap()` functions for custom/structured types.

```c++
...
pnet::type::value::value_type const data_token (5);
client.put_token (job_id, "input_data", data_token);
...
```

## Wait for Output

In GPI-Space, workflows usually terminate and return their outputs once no transitions can be fired anymore.
This means, asynchronous workflows will terminate prematurely if the time between new input tokens becomes larger than the time for processing all possible transitions.

To prevent this, GPI-Space offers the possibility to prevent a workflow from exiting automatically and instead wait until explicitely told to do so.
There are 2 possibilites of setting this workflow property.
The first one is to set it at the top level `defun` object of the Petri net:

```xml
<defun name="async_workflow">
  <properties name="drts">
    <property key="wait_for_output" value="true"/>
  </properties>
  ...
  <net>
  ...
  </net>
</defun>
```

Alternatively, the workflow object can use its `set_wait_for_output()` method in C++ before submission:

```c++
...
gspc::workflow workflow (...);
workflow.set_wait_for_output();
auto const job_id = client.submit (workflow, values_on_ports);
...
```

The workflow is terminated by calling the clients `wait_and_extract()` method:

```c++
...
auto const job_id = client.submit (workflow, values_on_ports);
...
client.put_token (job_id, "input_data", data_token);
...
std::multimap<std::string, pnet::type::value::value_type> workflow_result = client.wait_and_extract (job_id);
...
```

## Workflow Response

> ---
> **NOTE:**
>
> Although not required, it is highly recommended to use the `wait_for_output` property in conjunction with the workflow response feature.
>
> ---

Another use case for asynchronous workflows is the ability to receive outputs from an input without terminating a workflow (e.g. services, status queries, ...).
GPI-Space offers this functionality in the form of a transition providing a `connect-response`.
In contrast to workflow outputs, a response is extracted from a transition and not a place.
A token triggering a workflow response has a special structured data type consisting of the token value and a response id:

```xml
...
<struct name="value_and_response_id">
  <field name="value" type="int"/>
  <field name="response_id" type="string"/>
</struct>
...
```

> ---
> **NOTE:**
>
> - The `response_id` type must be `string`.
> - The `value` type can be anything and is not required to be `int`.
>
> ---

The place receiving the trigger token must be of such a type and have the `put_token` attribute set:

```xml
...
<net>
  ...
  <place name="trigger_response" type="value_and_response_id" put_token="true"/>
  ...
</net>
...
```

The transition connecting to this place needs to have a special `connect-response` output which acts as a `return value`:

```xml
...
<net>
  ...
  <transition name="time_two">
    <defun>
      <in name="trigger" type="value_and_response_id"/>
      <out name="response" type="int"/>
      <expression>
        ${response} := 2 * ${trigger.value};
      </expression>
    </defun>
    <connect-in port="trigger" place="trigger_response"/>
    <connect-response port="response" to="trigger"/>
  </transition>
  ...
</net>
...
```

> ---
> **NOTE:**
>
> - A workflow response can trigger arbitrarily complex transition chains.
>   However, only a single one of these transitions is allowed to have a `connect-response`.
> - The transition having the `connect-response` is not required to be the last one.
>
> ---

On the C++ side, a workflow response isn't triggered via `put_token`, but with `gspc::client`'s `synchrounous_workflow_response()` method:

```c++
...
int i = 2;
pnet::type::value::value_type const response = client.synchronous_workflow_response (job_id, "trigger_response", i);
...
```

> ---
> **NOTE:**
>
> The `response_id` is automatically generated by GPI-Space, *only* the `value` part of the response struct is required.
>
> ---
