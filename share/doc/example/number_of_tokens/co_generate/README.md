## Example: Co-Generate

This example demonstrates the usage of `connect-number-of-tokens` to implement a co-generate (as in "co-routine").

In the scenario there is a "process" function that processes "tasks" produced by a generator and using up a credit "token". The decision when to stop the processing is taken by the generator of the process token, and _not_ by the generator of the tasks.

One way to implement a task generator is to let it produce an infinite number of tasks and stop it if no more processing will happen. That allows to reuse the generator in multiple context's because it is not entangled with the processing or the generator for the processing tokens. However, to generate an infinite number of tasks would lead to non-termination, so there must be a way to stop the task generation. The `connect-number-of-tokens` is used in the example to throttle the generator if the number of ready tasks would exceed the number of workers. Please note that this does not introduce any non-local dependency: The throttled task generator has no knowledge about the processing or the generation of the processing tokens. Doing so the task generator stops if all process tokens are used up as no more processing tasks will be started. Moreover, there will always be enough ready tasks to feed workers if they are available.

### Remarks:

- The example receives the number of workers as one of the parameters and the driver program sets the value to the correct number. This approach works well for applications with static resources. Applications that add or remove resources during execution time must update the number of workers using `put_token`.

- The example does not implement any cleanup. The `connect-number-of-tokens` can be used to trigger cleanup steps. The process token generator can produce a token that encodes the information "no more process tokens will be produced" and if that token exists _and_ the number of processing tokens is zero, there will be no more processing and the state can be reset. Please find an example for starting one processing phase after another phase has finished in `share/doc/example/number_of_tokens/reduce_after_process`.
