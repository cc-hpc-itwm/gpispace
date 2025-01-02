## Example: Plugin put_token updates the number of tokens

This example demonstrates that `connect-number-of-tokens` is safe to be used in conjunction with `put_token` triggered by an expression plugin.

The scenario sums up values that are delivered from an expression plugin via `put_token`. The summation is delayed until the number of values passes a threshold.

### Remarks:

- Please note that m==0 is not sufficient to trigger the "done" transition (that also destroys the plugin): There might be still values to be processed. Make the "done" transition wait for the number of values to be zero makes sure that no more values will be produced and all produced values are processed.
