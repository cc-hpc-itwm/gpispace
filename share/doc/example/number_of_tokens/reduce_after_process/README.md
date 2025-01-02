## Example: Reduce after process

This example demonstrates the usage of `connect-number-of-tokens` to delay an activity (the "reduce") to after another activity has been completed (the "process").

The scenario starts with a standard produce-process-reduce pattern: There is a generator that generates work tasks. Those tasks are processed and produce a result. All task results are then reduced to a single sum. The Petri-Net engine ensure that generation, processing and reduction are happen concurrently, sharing the available resources.

But what if that is not the desired behavior? What if, instead, no reduction should start until all processing has been finished? The reason might be that the reduction is expensive and shall only be started if _all_ tasks have been processed successfully. Or the reason might be that processing and reduction are using similar resources and shall not interfere.

The answer is to use the `connect-number-of-tokens` and delay the reduction until the generation has been done _and_ the processing of all tasks has been finished. The `connect-number-tokens` describes the number of ready tasks _plus_ the number of currently processed tasks. If it becomes zero, then there are no unfinished tasks and the reduction is started by producing an initial reduction value.

### Remarks:

- Please note that `connect-number-of-tokens` is not essential in this example because the processing does not produce new tasks. One could duplicate the number of tasks and wait for that many tasks to finish.
