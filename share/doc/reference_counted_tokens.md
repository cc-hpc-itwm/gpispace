# Shared Type with Garbage Collection

The `shared` type enables automatic garbage collection in Petri net workflows. It wraps any value type together with a cleanup place name. When all references to a shared value are consumed, the wrapped value is automatically placed on the designated cleanup place, allowing user-defined cleanup logic.

## Type syntax

The type name is `shared_PLACENAME` where `PLACENAME` is the name of the place that will receive cleanup tokens.

```xml
<place name="filenames" type="shared_cleanup"/>
<place name="cleanup" type="string" shared_sink="true"/>
```

## Value syntax

Shared values can be created in expressions using the `shared_PLACENAME(value)` syntax:

```
// In expressions:
shared_cleanup ("filename.txt")    // Creates shared with cleanup place "cleanup"
shared_temp (42)                    // Creates shared with cleanup place "temp"
shared_gc (${x} + ${y})             // Creates shared wrapping an expression result
```

In module code, use the `we::type::shared` constructor:

```cpp
// In module code:
we::type::shared (wrapped_value, "cleanup_place_name")
```

## Behavior

Reference counting tracks shared values across transition firings:

1. When a transition fires, the runtime compares shared values in inputs vs outputs
2. For each shared value that appears more times in outputs than inputs, the reference count is increased by the difference
3. For each shared value that appears more times in inputs than outputs, the reference count is decreased by the difference
4. When the reference count reaches zero, the wrapped value (not the shared wrapper) is automatically placed on the cleanup place
5. A user-defined cleanup transition can then handle resource cleanup (e.g., deleting temporary files)

**Important**: Two shared values are considered identical if they have the same wrapped value AND the same cleanup place name. The reference counting is based on this identity.

## Identity semantics

Shared values use **value-based identity**, not reference-based identity. Two `shared` values are the same if and only if:
- Their wrapped values are equal (using the wrapped type's `==` operator)
- Their cleanup place names are equal

This means:

```cpp
// These are the SAME shared value (reference count shared):
we::type::shared {42, "cleanup"}
we::type::shared {42, "cleanup"}

// These are DIFFERENT shared values (independent reference counts):
we::type::shared {42, "cleanup_a"}
we::type::shared {42, "cleanup_b"}  // Different cleanup place

we::type::shared {42, "cleanup"}
we::type::shared {43, "cleanup"}    // Different wrapped value
```

**Warning**: Creating the same shared value (same wrapped value and cleanup place) in multiple places increments the same reference counter. This is intentional for sharing, but can be surprising if you accidentally create duplicates.

## Nested shared values

Shared values can be nested inside structs, lists, sets, and maps. The runtime recursively scans all values for shared content:

- **In structs**: When a struct containing a shared field is consumed, the nested shared value's reference count is decremented
- **In lists/sets/maps**: All shared values within containers are tracked
- **Shared containing shared**: A shared value can wrap a struct that itself contains shared fields. When the outer shared is cleaned up, the inner shared values are also properly reference-counted

This enables complex ownership patterns. For example, multiple outer shared values can reference the same inner shared value. The inner shared is only cleaned up when ALL outer references are consumed.

## Compile-time validation

The Petri net compiler `pnetc` validates that for each `shared_PLACENAME` type:
- A place named `PLACENAME` must exist in the same net
- The place `PLACENAME` must be marked with `shared_sink="true"`

If either condition is not met, `pnetc` will reject the input with an error.

## Runtime checking

Two conditions are checked at runtime when a shared value is used:

1. **Cleanup place existence and attribute**: The cleanup place must exist AND be marked with `shared_sink="true"`. If a shared value references a cleanup place that doesn't exist or isn't marked as a shared sink, a `std::runtime_error` is thrown immediately when the shared value is first tracked.

2. **Type compatibility**: The type of the wrapped value must match the cleanup place's type. If there is a mismatch (e.g., cleanup place has type `int` but the shared wraps a `long` value), a `type_mismatch` exception is thrown when the cleanup token is placed.

## Workflow termination

When a workflow finishes (either successfully or due to cancellation), shared values that still have non-zero reference counts are **not** automatically cleaned up. The wrapped values remain unreleased.

To ensure all resources are properly cleaned up:
- Design your workflow so that all shared values are consumed before completion
- Use a cleanup phase at the end of your workflow that explicitly consumes any remaining shared references
- Consider using `wait_for_output="true"` on the net to ensure cleanup transitions complete before the workflow returns

## Expression operations

Shared values support comparison operators: `==`, `!=`, `<`, `<=`, `>`, `>=`

Two shared values are equal if they have the same wrapped value and cleanup place name.

## Place attribute `shared_sink`

Places that serve as cleanup targets for `shared` types must be marked with the `shared_sink="true"` attribute:

```xml
<place name="cleanup" type="string" shared_sink="true"/>
```

This attribute:
- Enables compile-time validation that the cleanup place exists
- Suppresses the "unconnected place" warning (since the runtime writes to it, not explicit transitions)
- Colors the place light green in `pnet2dot` output for easy identification

## Place attribute `put_token` with `shared_sink`

A shared_sink place can also be marked with `put_token="true"` to allow explicit token placement in addition to automatic cleanup:

```xml
<place name="cleanup" type="string" shared_sink="true" put_token="true"/>
```

This combination:
- Allows the place to receive tokens both from automatic shared cleanup AND from transitions using `<connect-put>` or `put_token` attribute on output ports
- Is visualized with a gradient color (lightblue:lightgreen) in `pnet2dot` output
- Requires `wait_for_output="true"` on the net to ensure proper synchronization

## Example use case

```xml
<place name="temp_files" type="shared_cleanup"/>
<place name="cleanup" type="string" shared_sink="true"/>

<!-- Create a shared reference to a temporary file -->
<transition name="create_temp_file">
  <defun>
    <out name="filename" type="shared_cleanup"/>
    <module name="example" function="filename create_temp_file()">
      <cinclude href="we/type/shared.hpp"/>
      <code><![CDATA[
        std::string path = create_temp_file();
        return we::type::shared (path, "cleanup");
      ]]></code>
    </module>
  </defun>
  <connect-out port="filename" place="temp_files"/>
</transition>

<!-- Use the file (can be duplicated, passed around, etc.) -->
<transition name="use_file">
  <defun>
    <in name="filename" type="shared_cleanup"/>
    <!-- Process the file -->
  </defun>
  <connect-in port="filename" place="temp_files"/>
</transition>

<!-- Cleanup handler - receives the unwrapped string when refcount reaches zero -->
<transition name="cleanup_handler">
  <defun>
    <in name="to_delete" type="string"/>
    <module name="fs" function="delete_file (to_delete)"/>
  </defun>
  <connect-in port="to_delete" place="cleanup"/>
</transition>
```

## Examples

Complete working examples are available in `share/doc/example/shared/`:

- `simple.xpnet` - Basic shared creation, copying, and cleanup
- `nested_in_struct.xpnet` - Shared value as a struct field
- `nested_in_list.xpnet` - Shared values in a list
- `multiple_nested_in_struct.xpnet` - Multiple shared fields in one struct
- `nested_cleanup.xpnet` - Nested shared values (shared containing struct with shared field)
- `multiple_references_to_inner.xpnet` - Multiple outer shared values referencing the same inner shared
- `replace_shared_in_struct.xpnet` - Replacing a shared field decrements the original's reference count
- `passthrough.xpnet` - Passing shared through without consuming it
- `expression.xpnet` - Using shared values in expressions
- `consume_in_expression.xpnet` - Consuming shared values in expressions
- `cleanup_with_put_token.xpnet` - Cleanup place with both `shared_sink` and `put_token` attributes

## Distributed execution

Shared values are fully serializable and work correctly in distributed workflows across multiple nodes. The reference counting is performed in the workflow engine on the agent, not on worker nodes, so cleanup tokens are generated correctly regardless of where transitions execute.

## Performance optimization with transition annotations

By default, the runtime automatically detects which transitions handle shared values by analyzing the port type signatures. Tracking shared values involves comparing input and output tokens on each transition firing, which adds overhead for transitions that don't involve shared values.

### Automatic detection

The Petri net compiler `pnetc` automatically analyzes each transition's port type signatures to determine whether shared value tracking is needed. This detection is recursive: if a port has type `shared_cleanup`, or a `struct` type with a field of type `shared_cleanup`, the compiler recognizes that the transition handles shared values.

**Limitations**: Automatic detection only works when the type signature explicitly contains a `shared_*` type. It **cannot** detect shared values hidden inside generic container types like `list`, `set`, or `map`. For example, if a transition produces a `list` that contains shared values created at runtime, the compiler sees only `type="list"` and cannot know that the list will contain shared values.

### Manual annotations

When automatic detection fails, you can explicitly annotate transitions:

```xml
<!-- Mark a transition as producing shared values -->
<transition name="create_shared" produce_shared="true">
  ...
</transition>

<!-- Mark a transition as consuming shared values -->
<transition name="use_shared" consume_shared="true">
  ...
</transition>

<!-- Mark a transition as passing shared values through unchanged -->
<transition name="opaque_passthrough" passthrough_shared="true">
  ...
</transition>
```

### Annotation semantics

- **`produce_shared="true"`**: The transition creates or duplicates shared values in its outputs. Use this when the output type doesn't reveal that shared values are produced (e.g., a `list` containing shared values).

- **`consume_shared="true"`**: The transition consumes shared values from its inputs. Use this when the input type doesn't reveal that shared values are consumed.

- **`passthrough_shared="true"`**: The transition passes shared values through unchanged (same values in as out). This is a performance optimization that skips reference count tracking entirely. **Only use this when you are certain the transition does not modify, create, or destroy shared values.**

### Priority of settings

1. If `passthrough_shared="true"` is set, all tracking is disabled regardless of other settings
2. If `produce_shared` or `consume_shared` is explicitly set, that value is used
3. Otherwise, automatic detection from port type signatures is applied

### Validation

The compiler (`pnetc`) validates that the attributes are not contradictory. Specifically, `passthrough_shared="true"` cannot be combined with `produce_shared="true"` or `consume_shared="true"`, as passthrough semantics are incompatible with producing or consuming shared values.

### Incorrect annotations

Using incorrect annotations can cause reference counting errors:

- **Missing `consume_shared` when consuming shared values**: The reference count will not be decremented, causing a "leak" - shared values that never get cleaned up
- **Missing `produce_shared` when producing shared values**: The reference count will not be incremented, potentially causing premature cleanup or reference count underflow
- **Using `passthrough_shared` when actually modifying shared values**: Can cause either leaks or underflows depending on the modification

These errors may not cause immediate failures but will lead to incorrect cleanup behavior at runtime.

### Example - shared values in a generic list

```xml
<!-- This transition creates shared values inside a list -->
<!-- Auto-detection cannot see inside 'list', so we annotate explicitly -->
<transition name="create_list_of_shared" produce_shared="true">
  <defun>
    <in name="values" type="int"/>
    <out name="list_out" type="list"/>
    <module name="example" function="list_out create (values)">
      <code><![CDATA[
        std::list<pnet::type::value::value_type> result;
        result.push_back (we::type::shared {values, "cleanup"});
        return result;
      ]]></code>
    </module>
  </defun>
</transition>

<!-- This transition consumes the list containing shared values -->
<transition name="consume_list_of_shared" consume_shared="true">
  <defun>
    <in name="list_in" type="list"/>
    <module name="example" function="consume (list_in)">
      <code><![CDATA[
        // Process list - shared values inside will be cleaned up
      ]]></code>
    </module>
  </defun>
</transition>
```
