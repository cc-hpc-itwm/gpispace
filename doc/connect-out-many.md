# connect-out-many

## Motivation
Sometimes it is unknown in advance how many tokens a module call or an expression produces on a certain port.  In this,
it is highly likely that the Petri  Net defines a transition with an output port of type ``list``, that generates a list
of tokens without a pre-defined size. Based on this, the Petri Net may  then require computing on the individual tokens
that is encapsulated in the  dynamic output list generated.
For example, a module call might expand a node in a tree with varying branching factor into a list its child nodes, and
the following module call may require performing operations on the individual child nodes.

In the above scenario, the ``list`` generated on the output port of the Petri Net can be broken down into individual
tokens and fed to a place (whose type matches that of the output tokens) explicitly, by using the ``stack``
utility, along the following pattern:

```
(list) -> [if list is empty, then consume list token]
(list) <-> [if list is x,xs, then put x and put back xs] -> (x)
```
This naive solution has the following drawbacks:

* the application developer needs to write code to manually decompose the list in their Petri Net. The code is
  repetitive and adds non-business logic to the Petri Net.
* the implementation copies `O(n^2)` many elements where `n` is the length of the result list.

An ideal way to alleviate the drawbacks of the above solution is to introduce an option that facilitates the application
developer to connect an output port of type ``list`` directly to an individual valued place. The list-to-tokens
decomposition can happen implicitly, thus, shifting the burden away from the application developer. Towards enabling
this, we define the new **``connect-out-many``** Petri Net feature in this document.

## connect-out-many
The **``connect-out-many``** is a new Petri Net element, that defines a special port-to-place connector.  Unlike the typical
``connect-out``, that connects port and place with same data-type signatures, ``connect-out-many`` enables the Petri Net
to directly connect a ``list`` valued output port to the individual valued place (e.g., ``unsigned long``). The list is
decomposed within the GPI-Space workflow engine, thus, avoiding the need for any boilerplate code in the Petri Net
definition.

Now, in terms of the application runtime, instead of putting (i.e., injecting) a list as a single token to a place, any
port-to-place connections defined as ``connect-to-many`` triggers the workflow engine to:

* decompose the value of type ``list`` generated at the output port (via an executed module call or expression) into
  individual elements, and,
* feed each element of the list to the connected place (as long as the element type matches that of the place).

Unlike the manual decomposition solution, ``connect-out-many`` enables a sort of **put_many port-to-place token
injection pattern**, that is performed implicitly by the workflow engine in O(n).

## Usage
The ``connect-out-many`` Petri Net element can be leveraged as follows:

```
<place name="<place-name>" type="<place-type>" />
<transition name="<transition-name>">
 <defun>
 ...
 <out name="<output-port-name>" type="list"/>
 </defun>
 ...
<connect-out-many port="<output-port-name>" place="<place-name>"/>
</transition>
```

> NOTE (1): the ``list`` can be a homogeneous list or heterogeneous list, that contains value of any valid data-type
> supported by the Petri Net (defined in ``we/type/value.hpp``).


> NOTE (2): During compile time, the output port attached to a ``connect-out-many`` is only checked for type ``list``.
> Any mismatch between the elements of the ``list`` and the connected place is detected during at runtime, which causes the
> GPI-Space workflow engine to throw a ``type_mismatch`` exception.


## Example
We illustrate below a simple example regarding how to use ``connect-out-many``.

To contrast the ease-of-use of ``connect-out-many``, we first present an example with the naive solution described in
the [Motivation](#motivation).

```
//example_putmany_manual: generate list of [0..N) and
//compute its sum using ``stack`` utility.

<defun name="sum_of_zero_to_N_by_list">
....
<net>
  <place name="N" type="unsigned long"/>
  <place name="S" type="unsigned long">
    <token><value>0UL</value></token>
  </place>
  <place name="i" type="unsigned long"/>
  <place name="is" type="list"/>

  <transition name="generate">
    <defun>
      <in name="N" type="unsigned long"/>
      <out name="out" type="list"/>
      <module name="manual" function="generate (N, out)">
        <code><![CDATA[
        for (unsigned long i (0); i < N; ++i)
        {
          out.emplace_back (i);
        }
        ]]>
        </code>
      </module>
    </defun>
    <connect-in port="N" place="N"/>
    <connect-out port="out" place="is"/>
  </transition>

  <transition name="empty">
    <defun>
      <in name="is" type="list"/>
      <expression/>
    </defun>
    <condition>
      stack_empty (${is})
    </condition>
    <connect-in port="is" place="is"/>
  </transition>
  <transition name="cons">
    <defun>
      <inout name="is" type="list"/>
      <out name="i" type="unsigned long"/>
      <expression>
        ${i} := stack_top (${is});
        ${is} := stack_pop (${is})
      </expression>
    </defun>
    <condition>
      !stack_empty (${is})
    </condition>
    <connect-inout port="is" place="is"/>
    <connect-out port="i" place="i"/>
  </transition>

  <transition name="compute_sum">
    <defun>
      <in name="i" type="unsigned long"/>
      <inout name="S" type="unsigned long"/>
      <expression>
        ${S} := ${S} + ${i}
      </expression>
    </defun>
    <connect-in port="i" place="i"/>
    <connect-inout port="S" place="S"/>
  </transition>
 </net>
....
</defun>
```

As seen from ``example_putmany_manual``, we cannot directly connect the output port ``out`` to place ``i`` that inputs
into the ``sum`` module. Instead, this needs to be done via place ``is``, which employs the ``empty`` and ``cons``
transitions with the ``stack_top/stack_pop`` functionality to decompose the list output by ``generate`` and feed it into
``sum``.

Now, to get rid of the boilerplate in the above example, that decomposes the list manually (or explicitly), we can
leverage the new ``connect-to-many`` port-to-place connection. To enable this, we need to do the following:

* change the connection type of output port ``out`` from ``connect-out`` to ``connect-out-many``.
* directly connect output port ``out`` to place ``i``, and therefore, get rid of the ``empty`` and ``cons`` transitions.

The updated and a minimalistic version of the ``example_putmany_manual`` Petri Net with ``connect-out-many`` is
illustrated below in ``example_putmany_automatic``.

```
//example_putmany_automatic: generate list of [0..N) and
//compute its sum using "connect-out-many"

<defun name="sum_of_zero_to_N_by_list">
....
<net>
  <place name="N" type="unsigned long"/>
  <place name="S" type="unsigned long">
    <token><value>0UL</value></token>
  </place>
  <place name="i" type="unsigned long"/>

  <transition name="generate">
    <defun>
      <in name="N" type="unsigned long"/>
      <out name="out" type="list"/>
      <module name="manual" function="generate (N, out)">
        <code><![CDATA[
        for (unsigned long i (0); i < N; ++i)
        {
          out.emplace_back (i);
        }
        ]]>
        </code>
      </module>
    </defun>
    <connect-in port="N" place="N"/>
    <connect-out-many port="out" place="i"/>
  </transition>

  <transition name="compute_sum">
    <defun>
      <in name="i" type="unsigned long"/>
      <inout name="S" type="unsigned long"/>
      <expression>
        ${S} := ${S} + ${i}
      </expression>
    </defun>
    <connect-in port="i" place="i"/>
    <connect-inout port="S" place="S"/>
  </transition>
 </net>
....
</defun>
```
