## Example: Co-Generate

This example demonstrates that `connect-number-of-tokens` is safe to use in conjunction with `connect-out-many`, that is that `connect-out-many` correctly updates the number of tokens on a place.

In the demonstration teh Euler phi function is computed by producing a list of all co-prime numbers between `1` and the parameter `n` inside of a module call. The list is the split via `connect-out-many` and all co-primes are put onto the place `i`. The `count` transition puts the number of the tokens onto the place `phi`.
