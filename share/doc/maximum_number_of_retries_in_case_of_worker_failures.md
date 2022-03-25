# Specifying a Maximum Number of Retries for Tasks in Case of Worker Failures

In GPI-Space, if a worker crashes while it executes a task, this is removed from
the worker manager, being not anymore considered for future task assignments and
the task being rescheduled to other available workers. The worker's crash can be
provoked by either the task itself, due to an user provided faulty implementation
(e.g. containing severe bugs or demanding insufficient or missing resources on
the worker's host) or due to an external event, such as a physical failure of the
host, an underlying system scheduler kill or a manual removal. GPI-Space is tolerant
to worker failures and in all these cases attempts to reschedule the task to
other healthy workers. By default, there is no limitation for the number of accepted
consecutive task reschedulings and, in the worst case, when the task implementation
is buggy or makes wrong assumptions about the environment or the required resources,
it is possible to provoke the crash of all workers. In this case, all the other
workflows already running or subsequently submitted to the started runtime system
may stall because of insufficient computational resources, the user being not
notified about it and waiting for new resources to be added.

In order to overcome this situation, the users are allowed to supply for a
transition containing a module call in the workflow a value for the property
`maximum_number_of_retries`, specifying an upper limit for the number of task
retries, as in example below:

```
<defun name="transition">
  <properties name="fhg">
    <properties name="drts">
      <properties name="schedule">
        <property key="maximum_number_of_retries">"5UL"</property>
      </properties>
    </properties>
  </properties>
  <module name="module" function="f()">
    <code><![CDATA[
    ...
    ]]></code>
  </module>
</defun>
```

This property is checked by the internal scheduler of GPI-Space in case of
successive reschedulings and, if the limit is exceeded, the task is declared
failed and the workflow engine stops the workflow's execution, the user being
notified with an explicit error message. If the user supplies no value for this
property, the task is rescheduled as long as sufficient workers allowing to
continue the workflow's execution are left.
