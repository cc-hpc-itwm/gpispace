## Example: Condition and Number on the same place

This example demonstrates that `connect-number-of-tokens` can be used simultaneously in a single transition.

The example produces the numbers `[0..2n)` and the processing sums up the even numbers and ignores the odd numbers. It does so only if the number of available tokens is larger then `n`. To verify the number constraint is satisfied the number of tokens is accumulated and returned, too. The test driver verifies that the sum is the sum of the even number, that the unused inputs are the uneven numbers and that the transition has fired only if the constraint on the number of tokens was satisfied.
