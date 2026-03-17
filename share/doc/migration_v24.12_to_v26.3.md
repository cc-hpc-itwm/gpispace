## Migration Guide: GPI-Space v24.12 to v26.3

This document provides instructions for AI agents (and developers)
to mechanically migrate an existing GPI-Space application from
version 24.12 to version 26.3. The migration consists of several
independent steps that can be applied in any order.

## Step 1: Migrate include paths

Every `#include` directive in C++ source files (`.hpp`, `.cpp`,
`.ipp`) and every `<cinclude href="…"/>` element in `.xpnet` files
must be updated. Apply the following substitutions in order (the
order matters because some prefixes are substrings of others):

1. `#include <util-generic/` → `#include <gspc/util/`
2. `#include <util-rpc/` → `#include <gspc/rpc/`
3. `#include <util-qt/` → `#include <gspc/util/qt/`
4. `#include <fhg/util/` → `#include <gspc/util/`
5. `#include <fhg/assert.hpp>` → `#include <gspc/assert.hpp>`
6. `#include <fhg/project_info.hpp>` →
   `#include <gspc/configuration/info.hpp>`
7. `#include <fhgcom/` → `#include <gspc/com/`
8. `#include <installation_path.hpp>` →
   `#include <gspc/installation_path.hpp>`
9. `#include <drts/` → `#include <gspc/drts/`
10. `#include <we/` → `#include <gspc/we/`
11. `#include <iml/` → `#include <gspc/iml/`
12. `#include <rif/` → `#include <gspc/rif/`
13. `#include <logging/` → `#include <gspc/logging/`
14. `#include <sdpa/` → `#include <gspc/scheduler/`
15. `#include <pnete/ui/` → `#include <gspc/monitor/`
16. `#include <xml/parse/` → `#include <gspc/xml/parse/`

For `.xpnet` files, apply the same transformations to
`<cinclude href="…"/>` elements (without the `#include <` and `>`
delimiters). For example:

```
<cinclude href="drts/stream.hpp"/>
```

becomes:

```
<cinclude href="gspc/drts/stream.hpp"/>
```

### Removed headers

Remove any includes of:

- `util-generic/getenv.hpp` — use `std::getenv` from `<cstdlib>`
- `util-generic/nest_exceptions.hpp` — use
  `std::throw_with_nested` directly
- `util-generic/hash/boost/filesystem/path.hpp` — use
  `std::hash<std::filesystem::path>`
- `util-generic/serialization/boost/filesystem/path.hpp` — use
  `util-generic/serialization/std/filesystem/path.hpp` (now
  `gspc/util/serialization/std/filesystem/path.hpp`)
- `FMT/boost/filesystem/path.hpp` — use
  `gspc/util/fmt/std/filesystem/path.formatter.hpp`
- `util-generic/variant_cast.hpp` — use `std::visit` with
  a converting visitor
- `util-generic/cxx17/apply.hpp` — use `std::apply` from
  `<tuple>`
- `util-generic/cxx17/logical_operator_type_traits.hpp` — use
  `std::conjunction`, `std::disjunction`, `std::negation` from
  `<type_traits>`

## Step 2: Migrate FMT formatter includes

The `FMT/` include prefix has been eliminated. Formatters are
now co-located next to their types as `.formatter.hpp` files.

Apply these transformations:

1. For type-specific formatters, change
   `#include <FMT/path/to/Type.hpp>` to
   `#include <path/to/Type.formatter.hpp>` and then apply the
   include path migration from Step 1 to the result.
2. For generic `std/` and `boost/` formatters that do not have
   a gspc type, the files moved to `gspc/util/fmt/`:
   - `#include <FMT/std/…>` →
     `#include <gspc/util/fmt/std/…>` with `.hpp` replaced by
     `.formatter.hpp`
   - `#include <FMT/boost/…>` →
     `#include <gspc/util/fmt/boost/…>` with `.hpp` replaced
     by `.formatter.hpp`

Examples:

```cpp
// Before
#include <FMT/we/type/value/show.hpp>
#include <FMT/rif/entry_point.hpp>
#include <FMT/std/filesystem/path.hpp>
#include <FMT/boost/variant.hpp>
#include <FMT/std/optional.hpp>

// After
#include <gspc/we/type/value/show.formatter.hpp>
#include <gspc/rif/entry_point.formatter.hpp>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/util/fmt/std/optional.formatter.hpp>
```

## Step 3: Migrate namespaces

Apply the following namespace replacements throughout all C++
source files and xpnet `<code>` blocks. Use word-boundary
matching to avoid false positives.

Apply in this order (longest match first):

1. `fhg::util::boost::program_options` →
   `gspc::util::boost::program_options`
2. `fhg::util::syscall` → `gspc::util::syscall`
3. `fhg::util::` → `gspc::util::`
4. `fhg::util` → `gspc::util` (for namespace declarations)
5. `fhg::com::p2p` → `gspc::com::p2p`
6. `fhg::com` → `gspc::com`
7. `fhg::rif` → `gspc::rif`
8. `fhg::logging` → `gspc::logging`
9. `fhg::pnete::ui` → `gspc::monitor`
10. `we::type::literal` → `gspc::we::type::literal`
11. `we::type::value` → `gspc::we::type::value`
12. `we::type::` → `gspc::we::type::`
13. `we::type` → `gspc::we::type` (for namespace declarations)
14. `we::` → `gspc::we::` (be careful with word boundaries)
15. `iml::` → `gspc::iml::`
16. `sdpa::daemon` → `gspc::scheduler::daemon`
17. `sdpa::client` → `gspc::scheduler::client`
18. `sdpa::events` → `gspc::scheduler::events`
19. `sdpa::com` → `gspc::scheduler::com`
20. `sdpa::` → `gspc::scheduler::`
21. `xml::parse::type` → `gspc::xml::parse::type`
22. `xml::parse::error` → `gspc::xml::parse::error`
23. `xml::parse::state` → `gspc::xml::parse::state`
24. `xml::parse` → `gspc::xml::parse`
25. `rpc::` → `gspc::rpc::` (only where it refers to the
    GPI-Space RPC library; check context)
26. `pnet::type::value` → `gspc::pnet::type::value`
27. `pnet::type::` → `gspc::pnet::type::`
28. `bitsetofint::type` → `gspc::pnet::type::bitsetofint::type`

### Renamed types

- `sdpa::events::SDPAEvent` →
  `gspc::scheduler::events::SchedulerEvent`
- `sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN` →
  `gspc::scheduler::events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN`
- `sdpa::events::ErrorEvent::SDPA_EUNKNOWN` →
  `gspc::scheduler::events::ErrorEvent::SCHEDULER_EUNKNOWN`

If code used the unqualified `Requirements_and_preferences` or
`null_transfer_cost` (previously available via `using` declarations
in the `sdpa` namespace), replace with the fully qualified
`gspc::we::type::Requirements_and_preferences` and
`gspc::we::type::null_transfer_cost`.

## Step 4: Migrate assert macros

Search and replace:

- `fhg_assert(` → `gspc_assert(`
- `fhg_assert (` → `gspc_assert (`
- `#include <fhg/assert.hpp>` → `#include <gspc/assert.hpp>`
  (covered by Step 1)

In CMake files:

- `-DFHG_ASSERT_MODE=` → `-DGSPC_ASSERT_MODE=`
- `FHG_ASSERT_DISABLED` → `GSPC_ASSERT_DISABLED`
- `FHG_ASSERT_EXCEPTION` → `GSPC_ASSERT_EXCEPTION`

## Step 5: Migrate configuration/project info

- `fhg::project_info(…)` → `gspc::configuration::info(…)`
- `#include <fhg/project_info.hpp>` →
  `#include <gspc/configuration/info.hpp>` (covered by Step 1)

## Step 6: Migrate `boost::filesystem::path`

All public APIs now use `std::filesystem::path`. Replace:

- `boost::filesystem::path` → `std::filesystem::path`
- `#include <boost/filesystem.hpp>` → `#include <filesystem>`

If you have a `boost::filesystem::path` value that needs to be
passed to a GPI-Space API, convert with:

```cpp
std::filesystem::path {boost_path.string()}
```

## Known build issues

### Boost.Coroutine deprecation warnings

GPI-Space's bundled Boost includes Boost.Coroutine, which emits
deprecation warnings. If your project uses `-Werror`, add:

```cmake
add_compile_definitions (BOOST_COROUTINES_NO_DEPRECATION_WARNING)
```

For pnetc-generated code, also add:

```
--gen-cxxflags=-DBOOST_COROUTINES_NO_DEPRECATION_WARNING
```

### pnetc C++ standard flag

If your `add_custom_command` for pnetc passes
`--gen-cxxflags=-std=c++11` (or `-std=c++14`), update it to
`--gen-cxxflags=-std=c++17`. The generated code now requires
C++17 (e.g., for `std::filesystem` headers included via
`<cinclude>` and for other C++17-dependent GPI-Space headers):

```cmake
# Before
--gen-cxxflags=-std=c++11

# After
--gen-cxxflags=-std=c++17
```

## Step 6a: Migrate `boost::optional` to `std::optional`

Several public APIs (notably `gspc::scoped_runtime_system`) now
accept `std::optional` instead of `boost::optional`. Replace:

- `boost::optional<T>` → `std::optional<T>`
- `boost::none` → `std::nullopt`
- `#include <boost/optional.hpp>` → `#include <optional>`

### `std::filesystem::last_write_time` behavioral change

When migrating from `boost::filesystem` to `std::filesystem`,
note that `std::filesystem::last_write_time` returns
`std::filesystem::file_time_type`, not `std::time_t`. In C++17,
there is no direct conversion to `std::chrono::system_clock`.
Use an approximation:

```cpp
auto file_time_to_system_time (std::filesystem::file_time_type ft)
  -> std::chrono::system_clock::time_point
{
  auto const ft_now = std::filesystem::file_time_type::clock::now();
  auto const sys_now = std::chrono::system_clock::now();
  return sys_now + (ft - ft_now);
}
```

## Step 7: Migrate linker flags in `.xpnet` files

Search for `<ld flag="-lwe-dev"/>` and replace with
`<ld flag="-lgspc_we"/>`.

## Step 8: Migrate CMake targets

If your `CMakeLists.txt` directly links against any of the
following targets, update them:

| Old target | New target |
|---|---|
| `GPISpacePrivate::we-dev` | `GPISpacePrivate::gspc_we` |
| `GPISpace::APIGuard` | `GPISpace::api_guard` |
| `GPISpacePrivate::drts-context` | `GPISpacePrivate::gspc_worker_context` |
| `drts-context` | `gspc_worker_context` |
| `IML::Client` | `gspc_iml_client` |
| `Util::Generic` | `gspc_util` |
| `Util::RPC` | `gspc_rpc` |
| `Util::Qt` | `gspc_util_qt` |

The main public targets (`GPISpace::header-only`,
`GPISpace::workflow_development`, `GPISpace::execution`,
`GPISpace::pnetc`) are unchanged.

Note: `Util::Generic` (now `gspc_util`) may also appear in pnetc
`--gen-ldflags` generator expressions (e.g.,
`$<TARGET_LINKER_FILE:Util::Generic>`) and in `DEPENDS` clauses
of `add_custom_command` calls that invoke pnetc. Update those
references to `gspc_util` as well. Since `gspc_util` is an
imported target from the GPI-Space installation (not built
locally), it may be removed from the `DEPENDS` list entirely
if no local build dependency exists.

If your project uses `find_package (util-cmake)`,
`find_package (util-generic)`, `find_package (util-rpc)`,
`find_package (util-qt)`, or `find_package (iml)`, remove
those calls. The libraries are now provided directly by
`find_package (GPISpace)`.

If your project uses `include (util-cmake/add_macros)`, change it
to `include (add_macros)`. The `util-cmake/` prefix is no longer
valid since `util-cmake` is no longer a separate package.

If your project links worker context directly, replace
`-ldrts-context` with `-lgspc_worker_context`.

### RPATH for executables linking GPI-Space libraries

Any installed executable that links against GPI-Space libraries
(e.g., `gspc_util`, `gspc_rpc`) needs RPATH entries pointing
into the bundled GPI-Space. If the executable is defined via
`extended_add_executable`, the bundling system handles
`libexec/bundle/lib` automatically. However, for executables
installed outside `bin/`, you must also call:

```cmake
bundle_GPISpace_add_rpath (
  TARGET <target_name>
  INSTALL_DIRECTORY "<relative/install/path>"
)
```

## Step 8a: Migrate libexec binary names

The internal binaries in `libexec/gspc` now use the `gspc-`
prefix:

| Old binary | New binary |
|---|---|
| `libexec/gspc/agent` | `libexec/gspc/gspc-agent` |
| `libexec/gspc/drts-kernel` | `libexec/gspc/gspc-drts-kernel` |

Remove any `find_package` components
`DO_NOT_CHECK_GIT_SUBMODULES` or `ALLOW_DIFFERENT_GIT_SUBMODULES`.

## Step 8b: Migrate CMake cache variable names

GPI-Space specific CMake cache variables now use a `GSPC_` prefix.

| Old variable | New variable |
|---|---|
| `INSTALL_DO_NOT_BUNDLE` | `GSPC_INSTALL_DO_NOT_BUNDLE` |
| `INSTALL_RPATH_DIRS` | `GSPC_INSTALL_RPATH_DIRS` |
| `SHARED_DIRECTORY_FOR_TESTS` | `GSPC_SHARED_DIRECTORY_FOR_TESTS` |
| `TESTING_RIF_STRATEGY` | `GSPC_TESTING_RIF_STRATEGY` |
| `TESTING_RIF_STRATEGY_PARAMETERS` | `GSPC_TESTING_RIF_STRATEGY_PARAMETERS` |
| `IML_TESTING_BEEGFS_DIRECTORY` | `GSPC_IML_TESTING_BEEGFS_DIRECTORY` |
| `ALLOW_ANY_GPISPACE_VERSION` | removed -- no replacement needed |

Examples:

- `-DINSTALL_DO_NOT_BUNDLE=ON` ->
  `-DGSPC_INSTALL_DO_NOT_BUNDLE=ON`
- `-DSHARED_DIRECTORY_FOR_TESTS=/path/to/shared` ->
  `-DGSPC_SHARED_DIRECTORY_FOR_TESTS=/path/to/shared`

Old variable names are currently still accepted through a compatibility
layer. Each usage emits a deprecation warning and legacy names will be
removed by the end of 2026.

## Step 8c: Use `GSPC_LIBRARY_INSTALL_DIR` instead of hardcoded `lib`

GPI-Space libraries are installed to `lib` or `lib64` depending
on the platform (via `GNUInstallDirs`). The actual directory name
is exported as the CMake variable `GSPC_LIBRARY_INSTALL_DIR`
(e.g., `"lib64"` on 64-bit RPM-based systems, `"lib"` on Debian).

**Important:** To use the standard CMake variable
`CMAKE_INSTALL_LIBDIR` (which mirrors this platform-dependent
choice for your own project's libraries), add
`include (GNUInstallDirs)` near the top of your root
`CMakeLists.txt`, after `find_package (GPISpace)`.

This step affects **three** areas that must all be updated:

### 1. pnetc `--gen-ldflags` RPATH references

If your `CMakeLists.txt` or pnetc `--gen-ldflags` contains
hardcoded RPATH references to `gpispace/lib`, replace them with
`gpispace/${GSPC_LIBRARY_INSTALL_DIR}`.

For example, in pnetc `--gen-ldflags`:

```cmake
# Before (v24.12 — lib was always correct)
--gen-ldflags="-Wl,-rpath='$$$$ORIGIN/../libexec/bundle/gpispace/lib'"

# After (v26.3 — use variable)
--gen-ldflags="-Wl,-rpath='$$$$ORIGIN/../libexec/bundle/gpispace/${GSPC_LIBRARY_INSTALL_DIR}'"
```

### 2. CMake `install(DESTINATION lib)` for application libraries

Any `install(FILES … DESTINATION lib)` or
`install(TARGETS … DESTINATION lib)` directive that installs
application-built libraries (including pnetc-generated wrapper
libraries) must be updated to use `${CMAKE_INSTALL_LIBDIR}`
instead of hardcoded `lib`. Otherwise, on `lib64` platforms
the application executable will look for libraries in `lib64/`
but they will have been installed to `lib/`.

```cmake
# Before
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/gen/pnetc/op/libfoo.so"
  DESTINATION lib
)

# After
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/gen/pnetc/op/libfoo.so"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
```

### 3. Hardcoded `"lib"` paths in C++ source files

Application code often contains hardcoded paths such as
`installation_path / "lib" / "libfoo.so"` to locate
application-specific shared libraries at runtime. On `lib64`
platforms these paths will be wrong.

The recommended fix is to pass the library directory name as
a compile definition and use it in C++ code:

```cmake
# In CMakeLists.txt (after include(GNUInstallDirs))
add_compile_definitions (MY_APP_LIBDIR="${CMAKE_INSTALL_LIBDIR}")
```

```cpp
// Before (in C++ source)
std::filesystem::path const lib_path
  (installation_path / "lib" / "libfoo.so");

// After
std::filesystem::path const lib_path
  (installation_path / MY_APP_LIBDIR / "libfoo.so");
```

To find all affected locations, grep for hardcoded `"lib"`
path components:

```
grep -rn '/ "lib" /' --include='*.cpp' --include='*.hpp'
```

## Step 8d: Migrate Boost finding infrastructure

The `util-cmake` package is no longer separate. In addition to
changing `include (util-cmake/add_macros)` to
`include (add_macros)` (see Step 8), the following CMake
utilities from `util-cmake` have been removed entirely:

- `include (util-cmake/beautify_find_boost)` — **remove this line**
- The `find_boost(…)` macro — **no longer exists**

If your project used `find_boost(…)`, replace it with a standard
`find_package(Boost …)` call. The `FROM_GPISPACE_INSTALLATION`
option is no longer needed because `GPISpaceConfig.cmake` already
sets `BOOST_ROOT` to the bundled Boost location.

**Important:** On systems where a system-wide Boost installation
exists (e.g., via RPM packages), CMake may find the system Boost
instead of the GPI-Space bundled Boost. To prevent this, set
`Boost_NO_SYSTEM_PATHS` before calling `find_package`:

```cmake
# Before (v24.12)
include (util-cmake/beautify_find_boost)
find_boost (1.61 REQUIRED QUIET FROM_GPISPACE_INSTALLATION COMPONENTS
  filesystem program_options serialization …
)

# After (v26.3)
set (Boost_NO_SYSTEM_PATHS ON)
find_package (Boost 1.61 REQUIRED QUIET COMPONENTS
  filesystem program_options serialization …
)
```

If your project used `find_package (util-cmake)`, remove that
call as well.

## Step 9: Migrate `bytearray` usage

The constructors `we::type::bytearray(T)`,
`we::type::bytearray(T*)`, and `we::type::bytearray::copy(T*)`
now statically assert that `T` is `std::is_trivially_copyable`.

If your code passes a `std::tuple` to `bytearray`, define a plain
struct instead:

```cpp
// Before — fails static_assert in v26.3
using T = std::tuple<bool, double>;
we::type::bytearray ba (T {true, 3.14});

// After
struct S { bool _0; double _1; };
gspc::we::type::bytearray ba (S {true, 3.14});
```

## Step 10: Remove uses of deleted utilities

- `util-generic::temporary_return_value_holder` — removed, inline
  the logic at the call site.
- `util-generic/variant_cast.hpp` — removed, use `std::visit`.
- `util-generic/cxx17/apply.hpp` — removed, use `std::apply`.
- `util-generic/cxx17/logical_operator_type_traits.hpp` — removed,
  use `std::conjunction`, `std::disjunction`, `std::negation`.
- `fhg::util::cxx17::make_future_error` — removed, construct
  `std::future_error` directly.
- `util-generic/nest_exceptions.hpp` — removed, use
  `std::throw_with_nested`.
- `util-generic/getenv.hpp` — removed, use `std::getenv`.

## Step 11: Update environment variables (testing)

- `IML_NODEFILE_FOR_TESTS` → `GSPC_NODEFILE_FOR_TESTS`

## Step 12: Update license file paths (if referenced)

The installed license layout has changed:

- Old: single aggregated `share/GPISpace/LICENSES`
- New: `share/GPISpace/LICENSE` (project license) and
  `share/GPISpace/licenses/<Dependency>/LICENSE` (third-party)

## Verification

After applying all steps:

1. Grep for any remaining old include prefixes:
   `grep -rn '#include <drts/\|#include <we/\|#include <iml/\|#include <rif/\|#include <logging/\|#include <sdpa/\|#include <fhgcom/\|#include <util-generic/\|#include <util-rpc/\|#include <util-qt/\|#include <fhg/\|#include <pnete/\|#include <xml/parse/\|#include <FMT/\|#include <installation_path'`
2. Grep for old namespaces:
   `grep -rn 'fhg::util\|fhg::com\|fhg::rif\|fhg::logging\|fhg::pnete\|[^:]we::type\|[^:]sdpa::'`
3. Grep for old macros:
   `grep -rn 'fhg_assert\|FHG_ASSERT'`
4. Grep for old linker flags:
   `grep -rn 'lwe-dev'`
5. Grep for old CMake infrastructure:
   `grep -rn 'find_boost\|beautify_find_boost\|find_package.*util-cmake\|Util::Generic\|Util::RPC\|Util::Qt'`
6. Grep for hardcoded `lib` paths that should use
   `CMAKE_INSTALL_LIBDIR` or `GSPC_LIBRARY_INSTALL_DIR`:
   `grep -rn 'DESTINATION lib$\|DESTINATION "lib"\|/ "lib" /' --include='*.txt' --include='*.cmake' --include='*.cpp' --include='*.hpp'`
7. Build the project and fix any remaining compile errors.
