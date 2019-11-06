Partial cross product:
======================

## Motivation:

Sometimes conditions are not referencing all input ports and a partial cross product of the input tokens is sufficient to determine the truth value of the condition.

Suppose a transition `t` has `k` input ports connected to the places `p_0`, `p_1`, ..., `p_{k-1}` and a condition `c`. In order to find whether or not `t` can fire, the condition is evaluated up to `#p_0 * #p_1 * ... * #p_{k-1}` many times, where `#pi` denotes the number of tokens on place `pi`.

Now: If `c` does not refer to place `pi` the number of evaluations can be reduced by a factor of `#pi` by excluding `pi` when traversing the cross product.

## Partial cross product:

GPI-Space automatically computes the set of referenced ports (the places behind them) and computes the minimal part of the cross product.

## Example 1

Look at a transition `t` with input ports `x` and `y` and let `#y` be the number of tokens on the place `y`:

```
(x:bool) --> [t, if: ${x}] <-- (y:any)
```

Now, if a token is put onto place `x`, the condition can be evaluated on `O(1)` due to the partial cross product optimization even though there are `#y` many candidates in the full cross product.

## Example 2

In the wild this allocation pattern was produced:
```
(slot state) <--> [generate, if: !end (state)] ----------------------> (id, slot) --> [process] --> (processed id)
     :                  ^                                                  ^              |
     :                  |                                                  |              |
     :                (id) <-- [generate id] <--> (id state)               |              |
     :                  |                                                  |              |
     :                  |           +--------------------------------------+              |
     :                  v           |                                                     |
     + - - - - > [reuse, if: end (state)]  <-- (reusable slot) <--------------------------+
```
Suppose the `id state` and the `slot state` are initialized and the `id` generator starts to generate `id`s. Now, as long as the `slot state` has not reached its end, the transition `generate` produces new `slot`s. Once the `slot state` has reached its end, the transition `reuse` takes already used slots to assign them with `id`s.

_Without the partial cross product optimization_: How many condition evaluations are executed when `n` tokens are available on place `id` and `slot state` was updated:
1. if `slot state` has not reached its end:
   * the condition of `generate` is executed once to figure that `generate` can fire with one random `id`
   * the condition of `reuse` is executed `n = 1 * n` times to figure that `reuse` can not fire
2. if `slot state` has reached its end:
   * the condition of `generate` is executed `n = 1 * n` times to figure that `generate` can not fire
   * the condition of `reuse` is executed once to figure that `reuse` can fire with one random `id`

In both cases there are `n + 1` condition evaluations even though `2` evaluations would be sufficient to determine the enabled transitions.

Now suppose the `id` generator has produced `N` many `id`s and the `slot state` allows to produce `S` many `slot`s. Then the transition `generate` will fire (and update the `slot state`) `S` times and the transition `reuse` will fire (and not update the `slot state`) `N-S` times. This leads to `S` times case 1. and `1` time case 2 which sums up to `(S+1)*(N+1)` many condition evaluations where `(S+1)*2` many evaluations would be sufficient, computing a `O(N)` property in `O(N^2)` steps.

_With the partial cross product optimization_: The `O(N)` property is correctly computed in `O(N)` steps.
