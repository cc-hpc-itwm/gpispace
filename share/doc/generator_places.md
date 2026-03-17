# Generator Places

Generator places are special places in a Petri net that automatically produce unique values. When a token is consumed from a generator place, the runtime system automatically puts a new token with a unique value back to the place.

## Basic Usage

A generator place is declared by adding the `generator="true"` attribute to a place:

```xml
<place name="unique_id" type="int" generator="true"/>
```

The generator place always contains exactly one token. When a transition consumes that token, the runtime automatically generates a new, unique token and puts it back on the place.

## Supported Types

Generator places support the following types:

| Type           | Initial Value | Sequence                           |
|----------------|---------------|-------------------------------------|
| `int`          | `0`           | `0, 1, 2, ...`                     |
| `long`         | `0L`          | `0, 1, 2, ...`                     |
| `unsigned int` | `0U`          | `0, 1, 2, ...`                     |
| `unsigned long`| `0UL`         | `0, 1, 2, ...`                     |
| `char`         | `'a'`         | `'a', 'b', ..., 'z', ...`          |
| `string`       | `"a"`         | `"a", "b", ..., "z", "aa", "ab", ...` |
| `bigint`       | `0`           | `0, 1, 2, ...` (unlimited)         |

### String Generation

For `string` type, the generator produces values in alphabetically increasing order:
- `"a"`, `"b"`, ..., `"z"` (26 single-character strings)
- `"aa"`, `"ab"`, ..., `"az"`, `"ba"`, ..., `"zz"` (676 two-character strings)
- `"aaa"`, ..., and so on

### Overflow Detection

For finite-domain types (`int`, `long`, `unsigned int`, `unsigned long`, `char`), the runtime system throws a `std::runtime_error` when the maximum value is reached and no more unique values can be generated.

For infinite-domain types (`string`, `bigint`), values can be generated indefinitely.

## Restrictions

Generator places have several restrictions enforced at compile time by `pnetc`:

1. **No tokens in XML**: Generator places must not have `<token>` elements defined in the XML. The initial token is created automatically.

2. **No `put_token` attribute**: Generator places must not have the `put_token="true"` attribute, as external token injection would violate uniqueness.

3. **No `shared_sink` attribute**: Generator places must not have the `shared_sink="true"` attribute.

4. **No read connections**: Generator places do not support `connect-read` connections. Only `connect-in` (consuming) connections are allowed.

5. **No incoming connections**: Generator places do not support `connect-out`, `connect-inout`, or `connect-out-many` connections. Tokens on generator places are managed exclusively by the runtime.

6. **Valid type required**: The place type must be one of the supported types listed above.

### Compile-Time Error Examples

```xml
<!-- ERROR: generator place with tokens -->
<place name="gen" type="int" generator="true">
  <token><value>0</value></token>
</place>

<!-- ERROR: generator place with put_token -->
<place name="gen" type="int" generator="true" put_token="true"/>

<!-- ERROR: generator place with shared_sink -->
<place name="gen" type="int" generator="true" shared_sink="true"/>

<!-- ERROR: invalid type for generator -->
<place name="gen" type="float" generator="true"/>

<!-- ERROR: read connection to generator place -->
<place name="gen" type="int" generator="true"/>
<transition name="t">
  <defun>
    <in name="x" type="int"/>
    <expression/>
  </defun>
  <connect-read port="x" place="gen"/>  <!-- not allowed -->
</transition>

<!-- ERROR: incoming connection to generator place -->
<place name="gen" type="int" generator="true"/>
<transition name="t">
  <defun>
    <out name="x" type="int"/>
    <expression>x := 1</expression>
  </defun>
  <connect-out port="x" place="gen"/>  <!-- not allowed -->
</transition>
```

## Example: Task ID Generation

A common use case is generating unique identifiers for tasks:

```xml
<defun name="process_items">

  <in name="items" type="list" place="items"/>
  <out name="results" type="list" place="results"/>

  <net>

    <place name="items" type="list"/>
    <place name="id_generator" type="unsigned long" generator="true"/>
    <place name="work" type="task"/>
    <place name="results" type="list">
      <token><value>List()</value></token>
    </place>

    <!-- Split items into work tasks with unique IDs -->
    <transition name="split">
      <defun>
        <inout name="items" type="list"/>
        <in name="id" type="unsigned long"/>
        <out name="task" type="task"/>
        <expression>
          ${task.id} := ${id};
          ${task.data} := stack_top (${items});
          ${items} := stack_pop (${items});
        </expression>
      </defun>
      <condition>
        !stack_empty (${items})
      </condition>
      <connect-inout port="items" place="items"/>
      <connect-in port="id" place="id_generator"/>
      <connect-out port="task" place="work"/>
    </transition>

    <!-- Process work items -->
    <transition name="process">
      <defun>
        <in name="task" type="task"/>
        <inout name="results" type="list"/>
        <expression>
          ${results} := stack_push (${results}, ${task.id});
        </expression>
      </defun>
      <connect-in port="task" place="work"/>
      <connect-inout port="results" place="results"/>
    </transition>

    <!-- Cleanup empty list -->
    <transition name="done">
      <defun>
        <in name="items" type="list"/>
        <expression/>
      </defun>
      <condition>
        stack_empty (${items})
      </condition>
      <connect-in port="items" place="items"/>
    </transition>

  </net>

</defun>
```

## Examples

Working examples are available for all supported types in:
- `share/doc/example/generator/generator_int.xpnet`
- `share/doc/example/generator/generator_long.xpnet`
- `share/doc/example/generator/generator_uint.xpnet`
- `share/doc/example/generator/generator_ulong.xpnet`
- `share/doc/example/generator/generator_char.xpnet`
- `share/doc/example/generator/generator_string.xpnet`
- `share/doc/example/generator/generator_bigint.xpnet`

## Implementation Notes

- Generator places are initialized with their initial value when the net is created.
- The generator state (current value) is serialized along with the net state.
- Token regeneration happens immediately after the token is consumed, before transitions are re-enabled.
- All tokens ever placed on a generator place are guaranteed to be pairwise different.
