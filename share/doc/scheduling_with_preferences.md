# Scheduling with Preferences

In heterogeneous compute environments it is not uncommon to find multiple nodes with different hardware capabilities (e.g. GPUs, FPGAs, CPU core count, ...).
In GPI-Space these capabilties can be reflected in the topology description when setting up the distributed runtime system.
The scheduling with preferences feature now offers the possibility for a Petri net transition to provide multiple implementations based on the capabilities provided in the topology description.

## Usage

Instead of adding a single `module` tag to a transition's `defun`, it can now add a `modules` tag, signalling that the corresponding section may contain multiple module implementations, as described below.
A `modules` tag includes an ordered list of `preferences`. Each preference in this list should match one of the worker capabilities specified at start-up time in the topology description. The specified order is used as a hint by the scheduler, which finally decides what the best assignment is, such that the user specified order is respected and the performance is optimized:

```xml
...
<transition name="my_transition">
  <defun>
    ...
    <modules>
      <preferences>
        <target>CPU</target>
        <target>GPU</target>
        <target>FPGA</target>
      </preferences>
      ...
    </modules>
  </defun>
  ...
</transition>
...
```

In addition to the preference list, GPI-Space also allows to specify different implementations based on the selected workers capabilities.
After the `preferences` a module for each `target`.
Such a module needs to set the `target` attribute with a value from the `preferences` list:

```xml
...
<transition name="my_transition">
  <defun>
    ...
    <modules>
      <preferences>
        <target>CPU</target>
        <target>GPU</target>
        <target>FPGA</target>
      </preferences>

      <module name="my_module"
              function="task_result task(...)"
              target="CPU">
        ...
      </module>

      <module name="my_module"
              function="task_result task(...)"
              target="GPU">
        ...
      </module>

      <module name="my_module"
              function="task_result task(...)"
              target="FPGA">
        ...
      </module>
    </modules>
  </defun>
  ...
</transition>
...
```

> ---
> **NOTE:**
>
> - The target values need to be identical to the worker capabilities used in the topology description.
> - Transition requirements and preference targets need to be disjoint sets.
>   The worker capabilities required to run a transition is a combination of elements contained in the cartesian product of these 2 sets.
>   For example if a transition has the requirement set `{"compute"}` and the preferences `{"cpu", "gpu", "fpga"}`, then the worker capabilities required for running this transition are a combination of the following: `{"compute", "cpu"}`, `{"compute", "gpu"}`, and `{"compute", "fpga"}`.
>
> ---

When such a Petri net is compiled by `pnetc` it will output a shared library for each target module by appending the target name to the module name separated by an underscore (e.g. libmy_module_32CoreCPU.so).
