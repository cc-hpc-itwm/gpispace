# Eureka Feature in GPI-Space

The `Eureka` feature enables mixing of application level with meta-application level, providing a way to control tasks of a certain type, such that:

* pending tasks are removed and
* running tasks are interrupted and canceled.

## Motivating examples

The general background of the `eureka` feature is concurrent speculative execution of multiple different computations to compute a single result. That is useful if the different computations differ in their non-functional properties, e.g. the execution time or the amount of resources they consume differs significantly. The idea is to trade compute resources with guarantees about the maximum waiting time until the result has been computed, e.g. with more compute resources it becomes more and more likely to wait for a minimum amount of time.

Assume a set `F = {f1,f2,...,fn}` of `n` computations such that all share the same functional property `r = fi (x)`. From the functional point of view the computations are all equivalent. However, their non-functional properties might differ. For example some of them might take much longer than others. Often the non-functional properties are hard or impossible to predict and in such cases a concurrent speculative execution would use multiple compute resources to start some or even all of the computations concurrently. As soon as one of them has produced the result value, the other computations can be stopped and this is what the `eureka` feature allows to implement.

### Long and Short Paths

Imagine a circle `C` with radius `r` and center `M` and two points `s` and `f` on the circle. Further assume a computation `walk(path,a,b)` that can walk a given `path` to get from point `a` to point `b` and takes time proportional to the distance (with respect to the path) between `a` and `b`.

Let `0 <= alpha <= 2 \pi` be the angle between `s`, `M` and `f`. Then the difference of the execution times of `walk(C,s,f)` and `walk(C,f,s)` is `2r (alpha - \pi)`. Using the `eureka` feature and two compute resources, both computations can be started and as soon as the first returns the other one can be interrupted and canceled. That guarantees to wait for the minimum amount of time.

Please note that if `alpha != \pi`, then the availability of only a single compute resource would decrease the chance to wait for the minimum amount of time to `50%`.

### Search in (unbalanced) Graphs

The computation `walk` from the example "long and short path" easily extends to paths in arbitrary graphs. It can be used to implement parallel graph searches and the `eureka` feature is useful if parts of the graph differ in their size: Individual searches start from different points in the graph and as soon as one search has found a solution the other searches can be interrupted and canceled.

Implementations typically expand the graph up to a certain depth and start concurrent searches for the nodes at the front of the expansion. If there are multiple compute resources, then it can be expected that the time to wait for a solution decreases as large sub-graphs do not block the search in smaller sub-graphs. To find a balance between the effort for the initial expansion and the number and size of the sub-graph that fit the number of compute resources is hard in general.

## Motivating Examples with Diverging Computations

Assume the set of computations is partitioned `F = C \cup D` into a non empty set `C` of converging computations and a set `D` of diverging computations. A diverging computation runs "for ever", which includes the case that a function runs longer than users are able (e.g. access to compute resource expires) or willing to wait.

Without the `eureka` feature no diverging computation must be started or else the complete application will diverge. As users do not know in advance whether or not a computation diverges the whole application has a good chance to diverge. (If users would know which computations diverge, then they would remove them from the set of computations beforehand.)

With the `eureka` feature it is possible to start diverging computations and later interrupt and cancel them in case another computation produced the desired result. If there are more compute resources than diverging tasks (e.g. `n` compute resources), then the `eureka` feature can be used to _guarantee_ that the application converges even in the presence of diverging computations.

### Search in Infinite Graphs

Semi-decidable problems often expose search graphs that contain finite and infinite paths. Another example of "infinite" graph shows up when the cost to expand graph nodes differs widely, e.g. in symbolic algebra it happens that the expansion cost vary by many orders of magnitude and some nodes are too expensive to be expanded within reasonable time. To walk an "infinite" path is a diverging computation and using the `eureka` feature the chance to find a solution increases dramatically.

## Support in GPI-Space

GPI-Space provides:

* The tag `<eureka-group>` as a child of the tag `<module>`. The tag `<eureka-group>` assigns a task to an eureka group. The content of the tag `<eureka-group>` is an expression that shall evaluate to a value of type `string`. The evaluation of the expression happens at execution time using the evaluation context of the transition and thus allows the eureka group to depend on input data.

A task can belong to at most one eureka group. An eureka group can consist of tasks of different type, e.g. different `<defun>`s. There can be multiple eureka groups at the same time.

* The tag `<connect-eureka>` as a child of the tag `<transition>`. The tag `<connect-eureka>` takes a `port` as an attribute which implies the type `set`, i.e., a "set" of eureka groups can be eureka-ed together. If a transition produces an non-empty set of eureka groups connected via `<connect-eureka>`, then all running tasks that belong to those eureka groups are interrupted by a cancel notification. All pending tasks that belong to those eureka groups are removed and not started. The interrupted or removed tasks are not counted as failures, they are discarded instead.

After an eureka group has been eureka-ed and its tasks are interrupted or removed, the knowledge about that eureka group is removed from the system. Which implies that eureka groups can be reused and triggered multiple times.
It also implies that tasks that are produced after an eureka group has been eureka-ed are not affected by that past event but scheduled, started and executed.

## Usage Examples

See `$GSPC_SOURCE/src/we/test/eureka` for a number of examples for different scenarios.

The general pattern for a function in a eureka group is

```xml
<place name="eureka_group" type="string"/>
<transition name=...>
  <defun>
    <in name="eureka_group" type="string"/>
    ...
    <module name=... function=... pass_context="true">
      <eureka-group>${eureka_group}</eureka-group>
      <code><![CDATA[
        _pnetc_context->execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
        ( [=] ()        { /* implementation, forked */ }
        , []  ()        { /* on_cancel ()           */ }
        , []  (int sig) { /* on_signal (sig)        */ }
        , []  (int ec)  { /* on_exit (ec)           */ }
        );
      ]]></code>
    </module>
  </defun>
  <connect-read port="eureka-group" place="eureka-group"/>
  ...
</transition>
```

It uses an input port to read the value of the eureka group and the `execute_and_kill_on_cancel` to execute the implementation in a forked process and to react to the way the execution ends.

The transition to trigger the eureka typically looks like

```xml
<transition name=...>
  <defun>
    <in name="eureka-group" type="string"/>
    <out name="eureka" type="set"/>
    ...
    <expression>
      ${eureka} := set_insert (Set{}, ${eureka_group})
    </expression>
  </defun>
  <connect-in port="eureka-group" place="eureka-group"/>
  <connect-eureka port="eureka"/>
  ...
</transition>
```

Again an input port is used to receive the value of the eureka group. An output port `eureka` of type `set` is used to trigger the eureka. The group is inserted into that set and then the transition "connects" it using `<connect-eureka>`. The fact that `<connect-eureka>` has no other end emphasizes the change of the abstraction level: The "connection" goes into the execution environment and causes the to interrupt and cancel or to remove _other tasks_.

Please note that the trigger transition consumes the `eureka-group` using `<connect-in>`, in contrast to the function which reads the `eureka-group` using `<connect-read>`. This pattern ensures that after a eureka has been triggered no more tasks for the same eureka group are produced.

## Note on `kill_on_cancel`

This helper does not allow the `implementation` to return values using `return`. The reason is that the `implementation` is executed in a different (forked child) process in order to be able to interrupt it any time and without its cooperation. To get values back from the `implementation` the solution is to tell it a shared location like a file or a network address. The inability for the forked `implementation` to talk back to the surrounding system also includes log messages.

### Example Transformation

Assume an `implementation`:

```xml
<defun>
  <in name="x" type=.../>
  <out name="y" type=.../>
  <module name=... function="f (x, y)">
    <code><![CDATA[
    y = f_impl (x);
    ]]></code>
  </module>
</defun>
```

To use this implementation inside of `kill_and_cancel` and in an eureka group, the code could be changed into

```xml
<defun>
  <in name="eureka_group" type="string"/>
  <in name="x" type=.../>
  <out name="y" type=.../>
  <module name=... function="f (x, y)" pass_context="true">
    <eureka-group>${eureka_group}<eureka-group>
    <cinclude href="fstream"/>
    <cinclude href="string"/>
    <code><![CDATA[
    std::string const filename (random_unique_file_name());
    bool cancelled (false);

    _pnetc_context->execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
        ( [=] () // copy parameters
          {
            std::ofstream (filename) of;
            of << f_impl (x); // save result
          }
        , [&]
          {
            cancelled = true;
          }
        , &drts::worker::on_signal_unexpected
        , [] (int exit_code)
          {
            if (exit_code != 0)
            {
              drts::worker::on_exit_unexpected (exit_code);
            }
          }
        );

    if (!cancelled)
    {
      std::ifstream is (filename);
      is >> y; // read saved result
    }
    ]]></code>
  </module>
</defun>
```

to use a (temporary) file to communicate the result from the forked process back into the execution environment.

Please note that this approach requires to serialize and to deserialize the result data.

### Will there be a version of `kill_and_cancel` that supports to `return`  values?

Please contact the gpispace developers to give this task higher priority.
