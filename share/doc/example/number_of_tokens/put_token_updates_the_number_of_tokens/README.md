## Example: Put_Token updates the number of tokens

This example demonstrates that `connect-number-of-tokens` is safe to be used in conjunction with `put_token`.

The scenario sums up values that are delivered from some external entity via `put_token`. The summation is delayed until the number of values passes a threshold.

### Remarks:

- The example sums up all delivered values and the external entity signals the end of the delivery using a second `put_token` place. This is a common approach when using `put_token` and allow work-flows to react on external events.

- Please note that the signal from the generator is not sufficient to trigger the "done" transition: There might be still values to be processed. Make the "done" transition wait for the number of values to be zero makes sure that no more values will be produced and all produced values are processed.
