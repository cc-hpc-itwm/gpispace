Expression plugins:
===================

## Motivation:

- Debugging of expressions is hard as they provide no way to print to a side channel.

- To maintain mutable state is hard:
  - To use a token quickly exhausts the memory bus as the tokens are immutable and thus copied a lot when modified. Also the possible modifications are limited by the capabilities of the expression language. To modify parts of the state is possible only by manual management.
  - To use the virtual memory requires repeated memory transfers (this time distributed!) and repeated serialization. To transfer/modify parts of the state is possible only by manual management.

## Expression plugins:

They provide a hook for user provided callbacks inside the expression evaluation. The callbacks can access a mutable state that is stored along with the Petri Net.

## Usage:

### Use a plugin in a Petri net:

Decorate the expression by the property "gspc.we.plugin.COMMAND" where command is one of

- `call_before_eval`
- `destroy`
- `create`
- `call_after_eval`

When an expression is evaluated, the following actions are executed in the given order:

```
* 'call_before_eval': call all plugins given in the property value
|
* 'destroy': destroy the plugin given by the context value 'plugin_id'
\
  evaluation of the expression
/
* 'create': create the plugin given by the context value 'plugin_path'
|           and store its id in the context value 'plugin_id'
|
* 'call_after_eval' call all plugins given in the property value
```

Note: `call_before_eval` and `call_after_eval` both accept multiple plugin identifiers while `create` and `destroy` both talk about a single plugin identifier.

Note: `destroy` is executed before expression evaluation which implies that the `plugin_id` can not be the result of a computation inside the expression.

Note: `create` is executed after expression evaluation which implies that the construction might depend on input tokens and on results of computations inside the expression.

Note: It is allowed to call a plugin before evaluation and to destroy it in a single expression.

Note: It is allowed to call a plugin to create and to call it after evaluation in a single expression.

Note: It is allowed to create multiple plugins of the same or of different type.

Please see the example below for more details.

### Write a plugin:

To implement a Plugin `P` implement the interface provided in `we/plugin/Base.hpp` that is the three methods:

- a constructor `P::P (Context const&, PutToken)`
- two callback functions `void before_eval (Context const&)` and `void after_eval (Context const&)`

The `Context` is the same context that is used to evaluate the expression itself. The function `before_eval` will be called after the input tokens are bound in the evaluation context but before the expression itself is evaluated. The function `after_eval` will be called after the expression has been evaluated and, potentially, modified the evaluation context. In all calls the `Context` can be used to transport parameters from the applications state (e.g. token values) into the plugin.

The `PutToken` is a function that can be used to, well, put tokens into the application state the plugin belongs to. The plugin is allowed to save the function in is internal state and GPI-Space guarantees the validity of the function during the lifetime of the plugin.

In `we/plugin/Base.hpp` some convenience macros are provided.

### Compile a plugin:

Produce a position independent shared object and link with `GPISpace::execution`.

Note: Inside of the GPI-Space repository link with `pnet`, e.g.

```
extended_add_library (NAME Log
  NAMESPACE Plugin
  SOURCES "Log.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
  INSTALL
  INSTALL_DESTINATION "plugin"
)
```

## Example

Minimal plugin that puts the number of its invocations back into the applications state to a place that is specified in the constructor:

### Plugin implementation:

```
#include <we/plugin/Base.hpp>

#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Count : public Base
      {
        Count (Context const& context, PutToken put_token)
          : _ (boost::get<std::string> (context.value ({"plugin", "count", "place"})))
          , _put_token (std::move (put_token))
        {}

        virtual void before_eval (Context const&) override
        {
          _put_token (_place, _before++);
        }
        virtual void after_eval (Context const&) override
        {
          _put_token (_place, _after++);
        }

      private:
        std::string _place;
        PutToken _put_token;
        unsigned _before = 0;
        unsigned _after = 0;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::Count)
```

Note: Inside of the GPI-Space repository this is the file `we/plugin/test/B.cpp`.

### Plugin usage:

#### Create:

```
<defun>
  <properties name="gspc">
    <properties name="we">
      <properties name="plugin">
        <property key="create"/>
      </properties>
    </properties>
  </properties>
  <in name="plugin_path" type="string"/>
  <out name="plugin_id" type="unsigned long"/>
  <expression>
    ${plugin.count.place} := "place_name"
  </expression>
</defun>
```

This will read the path to the implementation from the variable `plugin_path` and produce the `plugin_id` to identify the particular instance of the plugin. Creation happens after expression evaluation, the plugin constructor can read the parameter from the context.

Of course, besides the interaction with the plugin system this is a standard expression function. There could be more inputs and more outputs. The parameter to the plugin might be another input or might be the result of a computation inside the expression.

#### Call before evaluation:

```
<defun>
  <properties name="gspc">
    <properties name="we">
      <properties name="plugin">
        <property key="call_before_eval">
          "stack_push (List(), ${plugin_id})"
        </property>
      </properties>
    </properties>
  </properties>
  <in name="plugin_id" type="unsigned long"/>
  <expression/>
</defun>
```

The value of the property key `gspc.we.plugin.call_before_eval` is an expression that evaluates to a list of plugin identifiers when evaluated with the bound context. Each plugin in the list is called before the expression is evaluated.

To transport parameters into the expression is achieved by more inputs.

Note: Computations inside of the expression can neither be used for parameters of calls to `call_before_eval` nor to compute which plugins are to be called.

#### Call after eval:

```
<defun>
  <properties name="gspc">
    <properties name="we">
      <properties name="plugin">
        <property key="call_after_eval">
          "stack_push (List(), ${plugin_id})"
        </property>
      </properties>
    </properties>
  </properties>
  <in name="plugin_id" type="unsigned long"/>
  <expression/>
</defun>
```

The value of the property key `gspc.we.plugin.call_after_eval` is an expression that evaluates to a list of plugin identifiers when evaluated with the context after the expression has been evaluated. Each plugin in the list is called after the expression is evaluated.

Again, to transport parameters into the expression is achieved by more inputs and/or computations inside the expression.

Note that, in principle, the expression itself might modify the list of plugin identifiers, e.g.

```
<defun>
  <properties name="gspc">
    <properties name="we">
      <properties name="plugin">
        <property key="call_after_eval">
          "${plugin_ids}"
        </property>
      </properties>
    </properties>
  </properties>
  <in name="plugin_id" type="unsigned long"/>
  <expression>
    ${plugin_ids} := stack_push (List(), ${plugin_id})
  </expression>
</defun>
```

Please, if an application starts to modify the list of plugins to be executed inside the expression, then contact the GPI-Space team in order for them to understand why the application would do so.

#### Destroy:

```
<defun>
  <properties name="gspc">
    <properties name="we">
      <properties name="plugin">
        <property key="destroy"/>
      </properties>
    </properties>
  </properties>
  <in name="plugin_id" type="unsigned long"/>
  <expression/>
</defun>
```

Destroys the plugin with the given id. All subsequent usages will throw an exception from the plugin system.
