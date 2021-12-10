# Fundamentals

## finally

A macro for ad-hoc RAII style cleanup, for use when adding a full RAII
wrapper for a resource or action would be overkill, for transactions,
temporary overrides, cleanup, notification of completion, ..., for
example, reading a file in threads using a pool of file descriptors:

```c++
size_t read (offset_t offset, void *buffer, size_t amount)
{
  auto const fd (_fds.get());
  FHG_UTIL_FINALLY ([&] { _fds.put (fd); });

  util::syscall::lseek (fd, offset, SEEK_SET);
  return util::syscall::read (fd, buffer, amount);
}
```

## in_this_scope

A macro to change the value of a given variable in the current scope
and make sure the variable is set to it's original value at scope exit.

```c++
variable = value;
{
  FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

  assert (variable == other_value);
}
assert (variable == value);
```

## hard_integral_typedef

A set of macros to create type safe integral types that do not convert
to each other unless explicitly opted in. For customization there are
additional macros to opt into other operations like serialization,
hashing, conversion or per-operator-choosable arithmetic operations.

## divru

Division with rounding up, for integrals. `divru (x, y)` is
essentially equivalent to `int (ceil (x / float (y)))`.

## dllexport

A function or type marked with this macro is exported into the public
binary API, when producing a dynamically linked binary.

# Marking suspicious code

## fallthrough

Marker for an intended fallthrough of switch-case blocks.

## unreachable

Marker for an unreachable line of code.

## unused

Marker for an unconditionally, intentionally unused variable.

## warning

Markers for suppressed warnings, requiring a reasoning.

* sign conversion
* signed-unsigned comparisons
* narrowing conversions

# Filesystem

## read_file

Read an entire file to a string or container, optionally
`lexical_cast`-ing the content to a given type.

## read_lines

Read an entire file to a vector of strings, splitting by lines as
defined per `std::ifstream().widen ('\n')`.

## write_file

Write a given object to a file.

## temporary_file

Temporarily create a file, ensuring it didn't exist before.

## temporary_path

Temporarily create a directory, ensuring it didn't exist before.

## scoped_file_with_content

Temporarily create a file with the given content.

## filesystem_lock_directory

A RAII utility to atomically lock a resource using a directory on the
filesystem.

# system call/libc/legacy/OS API wrapping

## syscall

Wrappers for most libc or system call functions, throwing exceptions
in case of errors, and "fixing" return types, e.g. removing signed
values when sign is only used for error signaling, or using return
values instead of out-pointers.

## executable_path

Helper functions to determine the absolute file path of the currently
ran executable. This is possible for both, the entire process or a
specific symbol. Specific symbols shall be preferred for any kind of
shared object to not get the path of the executable loading the shared
object but the shared object itself.

## dynamic_linking

`scoped_dlhandle` is an error-checking wrapper to the `dlopen()`
family functions for runtime loading of shared objects. Symbols can be
retrieved using the helper macro `FHG_UTIL_SCOPED_DLHANDLE_SYMBOL`
which automatically returns the right type, assuming the name is fully
qualified. `currently_loaded_libraries()` allows for basic observation
of the libraries loaded.

## interruption_handler

Block execution until a call to a notifier function is made or
`SIGINT` is received. A function to be called on interruption can be
given.

## syscall/directory

Scoped wrapper for the `dirfd` family of system calls.

## syscall/process_signal_block, syscall/signal_fd, syscall/signal_set

Higher level wrapper for the `sigprocmask`, `signalfd` and `sigset_t`
family of system calls.

## procfs

A basic abstraction for the `/proc` filesystem to inspect running
processes.

## getenv

Wrapper for `std::getenv()` with a sane return value for missing
environment variables.

## hostname

Wrapper for `gethostname()` that avoids the required buffer management
and caches the call.

## system

Wrapper around `std::system` that throws with a descriptive message in case of an error. A second version nests possible errors into a user defined exception with an additional description.

# Testing

## testing/random, testing/random/*

Facilities to generate random values for test cases. The seed used for
the random number engine is automatically printed when a test fails.

Adapters for `random<>` that generate multiple values, optionally
unique values are available. Some generators, e.g. strings, have more
fine grained control with e.g. exclusion sets.

Note: Individual functions to constraint the values generated are
deprecated and `random<T>::operator() (constraints...)` should be used
instead. Usually `random<T>::operator()()` has sane defaults and can
be used.

## testing/require_exception

Require that a given function throws an exception of a given type and
value if called. Comparison can happen by `operator==` if it exists or
`std::exception::what()`. Instead of giving a comparison exception, a
comparison exception message (or set of messages) may also be
used. Also verifies nested exceptions if given.

## testing/require_compiletime

Checks that the given condition can be compile-time evaluated,
i.e. `static_assert`ed, but only report failure at runtime for
consistency in Boost.Test based tests.

# testing/require_container_is_permutation

Checks that two containers given are of same size and their contents
are a permutation of each other. Useful for `unordered_x` or
`multimap` containers which don't guarantee order of elements.

## testing/measure_average_time

Measure the average execution time of a given function, executing it
the given times.

## testing/require_maximum_running_time

Measures the time between construction and destruction of the object
and ensures that it did not exceed a given duration, using Boost.Test.

## testing/require_serialized_to_id

A Boost.Test predicate to ensure a type's serialization produces an
equal-comparing object when serialized and deserialized after
initialization as given.

## testing/require_type

Compile time checks for type (in)equality that defers test failure to
runtime for consistency with other Boost.Test based tests.

## testing/flatten_nested_exceptions

Include to ensure nested exceptions that are automatically catched by
Boost.Test when bubbling up to the top level of a test case are
printed recursively.

## testing/printer/generic, testing/printer/*

Macros to easily define Boost.Test log value printers used when
reporting bad values in assertions. Printers for commonly asserted
types are provided as well.

# Template/Meta/Generic -programming

## callable_signature

Function traits to get the signature or the return type of a callable
`Callable t` i.e. `t()` or `Callable::operator()` and to ensure a
`Callable` is callable with a given signature.

## this_bound_mem_fn

Bind a `this` to a member function for easy creation of purely
forwarding functors without having to specify loads of
`std::placeholder`s or repeating all arguments in a lambda and
manually having to perfect forward all of them.

## cxx17/holds_alternative

`holds_alternative` implementation for `boost::variant`.
It checks if the given `boost::variant` holds the queried alternative
type.
The call is ill-formed if the queried type doesn't appear exactly once
in the variant's types.

## cxx17/void_t

Metafunction that maps a sequence of any types to the type
`void`. This is used in metaprogramming to detect existence or
ill-formedness of types or expressions in SFINAE contexts.

```c++
template<typename T, typename = void>
  struct has_operator_not : std::false_type {};

template<typename T>
  struct has_operator_not<T, void_t<decltype (!std::declval<T>())>>
///  expression required to be well formed    ^^^^^^^^^^^^^^^^^^
    : std::true_type {};

static_assert (!has_operator_not<struct some_type>{}, "");
static_assert (has_operator_not<int>{}, "");
```

## cxx17/logical_operator_type_traits

Compile time combinators for `std::integral_constant`s:
`conjunction<B...>`, `disjunction<B...>` and `negation<B>`.

## mp/find

Returns an `std::integral_constant<std::size_t>` with the first index
of type `T` in types `Types`.

## mp/exactly_one_is

Given a type `Needle`, checks that types `Haystack` contains exactly
one `Needle`, i.e. not more than one or none.

## mp/none_is

Given a type `Needle`, checks that types `Haystack` do not contain
`Needle`.

## mp/apply

Apply an `Operation<typename> op` to all elements of type-sequence
`Sequence = sequence_type<Types...>` and return `sequence_type<op
(Types)...>`.

## mp/rebind

Rebind a variadic template `T = Sequence<U...>` to `To<U...>`.

## mp/remove_duplicates

Given a type-sequence `Sequence = sequence_types<Types...>`, returns
`sequence_type<unique (Types...)>`.

## cxx17/apply

Call a given function with the arguments in the given tuple.

## va_args

Macros for handling variadic arguments to macros and emulating default
arguments.

* `FHG_UTIL_REQUIRED_VA_ARG`: get nth argument
* `FHG_UTIL_OPTIONAL_VA_ARG`: get nth argument or fallback-th if not
  enough arguments given

```c++
#define OPERATOR_SIGNATURE(/*lhs, rhs = lhs*/...)                \
  bool operator== ( FHG_UTIL_REQUIRED_VA_ARG (1, __VA_ARGS__)    \
                  , FHG_UTIL_OPTIONAL_VA_ARG (2, 1, __VA_ARGS__) \
                  )
OPERATOR_SIGNATURE (T);    // bool operator== (T, T);
OPERATOR_SIGNATURE (T, U); // bool operator== (T, U);
```

## ndebug

A macro that evaluates to nothing if not building for debug and the
given value if debugging, or the other way around.

## temporary_return_value_holder

Metaprogramming helper to call a `void` function the same way as
calling a non-`void` function, when needing to temporarily store the
return value:

```c++
temporary_return_value_holder<T> const result (fun);
// note: T const result (fun); ill-formed if T=void!
do_something_unrelated();
return *result;
```

# Networking

## connectable_to_address_string

Given a Boost.ASIO `ip::address`, get a connectable-to address usable
on other hosts.

## scoped_boost_asio_io_service_with_threads

A scoped wrapper for Boost.ASIO's `io_service` that ensures a given
number of threads are running and waiting for work until destruction,
to avoid thread management and `io_service.stop()` call hell. Also
contains a variant that supports forking.

# Concurrency

## asynchronous

Call a function on a given collection of values using `std::async()`
and collect exceptions.

## blocked

Call a function on a given collection of values in blocks of the given
size.

Variants to call the function asynchronously (using `std::async()`)
and to collect the function call results exist.

## latch

A threadsafe countdown which offers to wait for count reaching zero
from multiple threads, for example to wait for outstanding work to be
completed by a thread pool before continuing.

## threadsafe_queue

Variations of threadsafe queues that block until a value can be
retrieved. Optionally interruptible and bounded capacity.

# Exceptions

## nest_exceptions

Call a functor and wrap any exception thrown inside with a given
exception.

## print_exception

Nicely print given or current in flight exception, including nested
exceptions.

## wait_and_collect_exceptions

Given a vector of exception pointers, throw an aggregating exception
joining all messages, or do nothing if empty.

Given a collection and an operation, the exceptions can also be
collected first. A convenience overload for a vector of `future<void>`
exists that collects by calling `get()`.

# Extended Algorithms and Containers

## join, print_container

Join the values of a range or container with given separator, prefix
and suffix, and a element limit. Joining happens lazily during
streaming to an ostream, or to a string.

## split

Split a given string or container by a given separator.

## refcounted_set

A `set` container that stores unique objects of type `T`, but allows
multiple references to them and only erases once as many `erase`s
happened as `emplace`s happened before.

## cxx17/future_error

A way to construct a `std::future_error` independent of standard
library version.

## functor_visitor

Visit a Boost.Variant with ad-hoc assembled visitors using lambdas, or
functors that are not derived of `boost::static_visitor<>`.

```c++
boost::variant<int, float> variant;
visit (variant, [] (int) {}, [] (float) {});
```

## make_optional

A macro equivalent to `boost::make_optional (cond, value)` except that
evaluation of value is deferred, which is often a requirement when
value is dependend on cond and would require `cond ?
boost::optional<T> (value) : boost::optional<T>()` instead.

## boost/program_options/separated_argument_list_parser

A Boost.ProgramOptions parser that takes all arguments between two
sentinel values as the value for a given argument name, e.g. for
complex values or nested command line parsers.

## variant_cast

Cast a `boost::variant<T...>` to `boost::variant<U...>` with runtime
check to throw on cases where a `T` is not in `U`.

# Serialization

## serialization/exception

Serialization and deserialization of `std::exception_ptr`s. Extendable
to non-STL exception types. Handles nested exceptions and tries to
fall back to basic STL exceptions if custom types are known on
serialization site only.

## serialization/by_member

Provide a Boost.Serialization implementation for a type by serializing
all given base classes and members individually.

## serialization/trivial

Mark a type as trivially serializable using Boost.Serialization, with
a sanity check that the given base classes or members are also
trivially serializable.

## serialization/boost/*, serialization/std/*

Various Boost.Serialization implementations for STL or Boost types.

# Hashing

## hash/combined_hash

Helpers for easily creating hash functions.

* `size_t combined_hash (Values...)` calls `std::hash` on all values
  and combines their hashes automatically.
* `FHG_UTIL_MAKE_COMBINED_STD_HASH` defines `std::hash` for the given
  type, using `combined_hash()` on the given members.

## hash/boost/asio/ip/tcp/endpoint, hash/std/pair, hash/std/tuple

Implementations of `std::hash` for the given types.

# Streaming Output

## first_then

An ostream output function that returns different values for the first
than all other invocations, e.g to avoid a leading comma using
`first_then separator ("", ", ");`.

## ostream/line_by_line

A stream buffer implementation that calls a function for every line
written to the stream.

## ostream/line_by_line

A stream buffer implementation that puts a (constant) prefix before
each line written to the stream.

## ostream/echo

An output stream that prefixes every line with the current timestamp
before forwarding to another output stream.

## ostream/callback/print, ostream/callback/*

A composable system of callbacks to transform how a value is printed
to an output stream.

## ostream/modifier

An abstract base for explicit opt in to ostream-ability. Provides a
convenience conversion to string.

## ostream/put_time

Print the given time point, or the current time.

## ostream/redirect

A stream buffer implementation that, like ostream/line_by_line calls a
function per line written to the stream, but does so for a given
output stream, replacing the original buffer. It ensures the original
buffer is restored during calling the given function so that it may be
used to non-instrusively modify output while keeping the output
destination, e.g. to prefix lines with a timestamp.

## ostream/redirect_standard_streams

Redirect all three standard output streams (`cout`, `clog` and `cerr`)
to a vector.

## ostream/to_string

Convert a given value to string.

# Timer

## scoped

Print START and DONE of a scope including a description and the
execution time with given resolution.

## sections

Maintains a scoped timer and allows to start and stop section timers while to overall timer runs. Sections can be started any time and end the previous section. Running sections can be stopped any time manually too.

## application

Combines ostream/echo and a section timer. This example usage

```c++
int main()
{
  fhg::util::default_application_timer out {"holla"};
  FHG_UTIL_FINALLY ([&] { out << "bye\n"; });

  out << "welcome\n";

  out.section ("yippie");
  out << "message\n";

  out.section ("yeah");
  out << "fun fun fun\n";

  out.section ("i go home");
}
```

will produce output (on std::cout) similar to this:

```
[2018-08-30 10:26:55] START: holla...
[2018-08-30 10:26:55] welcome
[2018-08-30 10:26:55] START: yippie...
[2018-08-30 10:26:55] message
[2018-08-30 10:26:55] DONE: yippie [2 ms]
[2018-08-30 10:26:55] START: yeah...
[2018-08-30 10:26:55] fun fun fun
[2018-08-30 10:26:55] DONE: yeah [0 ms]
[2018-08-30 10:26:55] START: i go home...
[2018-08-30 10:26:55] bye
[2018-08-30 10:26:55] DONE: i go home [9 ms]
[2018-08-30 10:26:55] DONE: holla [12 ms]
```
