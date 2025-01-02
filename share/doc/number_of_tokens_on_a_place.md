# Number of tokens on a place

The GPI-Space Petri-nets provide the "number of tokens" extension: Petri-Nets are able to observe the number of tokens that are currently placed on a place.

The connection type `connect-number-of-tokens` can be used to bind the number of tokens on a place to a port of type `unsigned long`. In the snippet

```
<place name="p" type="..."/>
<transition name="not">
  <defun>
    <in name="k" type="unsigned long"/>
    ...
    <condition>
      ${k} :eq: 0ul
    </condition>
  </defun>
  <connect-number-of-tokens port="k" place="p"/>
</transition>
```

the transition `not` only fires if there are no tokens on the place `p`. The connection `connect-number-of-tokens` connects a port of type `unsigned long` and a place of any type. The port is a usual place and the assigned value can be used in conditions and in the implementation of the function just like any other port of type `unsigned long`. It is possible to connect to a place and, in the same transition, to the number of tokens of the same place. It is possible to connect (multiple ports) to multiple counters in a single transition.

The value assigned is the sum of the number of tokens that have been "produced" and the number of tokens that are "currently consumed". A token `x` has been "produced" if a transition has put `x` onto `p`. A token `x` is "currently consumed" if a transition `t` consumes `x` from `p` and has not been finished. That are all transitions that are extracted by the workflow engine from the Petri-Net and handed over to the execution environment. In the execution environment the transition might be waiting to be started, might be currently executed or might have already finished execution. The token `x` is no longer "currently consumed" if the execution environment has delivered the result of the execution of `t` to the workflow engine and the workflow engine has injected the result into the state of the Petri-net.

This definition makes the number of tokens the same as if there would be no execution time: If transitions fire immediately, then the events "extract" and "inject" happens at the same time and there is no way to make a distinction between transitions that have been started and transitions that are running and are not yet finished. Similar the "number of tokens on a place" feature does not allow to observe the number of transitions "in flight" and therefore is does not introduce "execution time" as observable entity into the Petri-Net.

Please note that the folder `share/doc/examples/number_of_tokens` contains some examples of how to use `connect-number-of-tokens`.

## Why is it hard (impossible) to implement it in a Petri-Net?

Please note that it is impossible to implement a counter for the number of tokens on a place without using priorities.

One could modify Petri-nets such that tokens are not put directly to a place `p` but, instead, onto a place `pre-p` and then moved from `pre-p` to `p` when the counter `#p` is incremented at the same time. That makes `#p` store the "number of tokens that have been put onto `p`".

Turn
```
 [ b ] -> ( p )
```
into
```
 [ b ] -> ( pre-p ) -> [ move_and_inc ] -> ( p )

                            ^
                            | |
                              v

                           ( #p )
```
to count the number of tokens put onto `p`.

Now, the same approach does not work to count the tokens that have been consumed from `p`: The modified net moves a token from `p` to some `post-p` and decrements `#p` at the same time. The net
```
 ( p ) -> [ a ]
```
has become
```
 ( p ) -> [ move_and_dec ] -> ( post-p ) -> [ a ] -> ...

                ^
                | |
                  v

               ( #p )
```
Now please consider the fire sequence `b`, `move_and_inc`, `move_and_dec`. The effect is that `#p` did not change its value and there is a token on `post-p` that has not been consumed by `a`. Which means that `#p` does not store the value "number of tokens on `p`", just because the token on `post-p` is not counted correctly. That means that the increment of `#p` must happen only after `a` has returned. A modified network looks like
```
 ( p ) -> [ a ] -> ...

            |
            v

        ( post-p ) -> [ consume_and_dec ]

                              ^
                              | |
                                v

                             ( #p )
```
However, this network is not correct either: After the fire sequence `b`, `move_and_inc`, `a` the value of `#p` is bigger than the "number of tokens on `p`", just because the token on `post-p` has not been taken into account yet.

So, both modifications are only approximate the correct value, the first modifications under-approximates and the second modification over-approximates.

Using priorities the second approximation can be turned into a correct modification: If `consume_and_dec` has the highest priority, then the update of the counter will always happen before any other transition fires, so there is no chance to ever observe the wrong value. However, the use of priorities is not desired as it currently limits the compose-ability of Petri-Net. It would be a valid approach to fix the Petri-Net composition and make sure that the relation between priorities defined in sub-networks are preserved when sub-networks are combined into larger networks.
