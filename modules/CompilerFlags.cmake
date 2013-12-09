# definitions

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions ("-DNDEBUG")
endif()

if (NOT ENABLE_BACKTRACE_ON_PARSE_ERROR)
  add_definitions ("-DNO_BACKTRACE_ON_PARSE_ERROR")
endif()


# warnings

set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -W")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wall")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wextra")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-attributes")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-system-headers")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-unknown-pragmas")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-unused-parameter")
set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wnon-virtual-dtor")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-constant-logical-operand")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-delete-non-virtual-dtor")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-deprecated-writable-strings")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-format-zero-length")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-overloaded-virtual")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-parentheses")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-unknown-warning-option")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-unneeded-internal-declaration")
endif()

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-format")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-ignored-qualifiers")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -Wno-write-strings")
endif()

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -wd191")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -wd1170")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -wd1292")
  set (FLAGS_WARNINGS "${FLAGS_WARNINGS} -wd2196")
endif()

# to avoid warnings when using gcc 4.5
add_definitions ("-fno-strict-aliasing")


# other flags

set (C_FLAGS "${C_FLAGS} -fpic -fPIC")

set (C_FLAGS_RELEASE "${C_FLAGS_RELEASE} -O3")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set(C_FLAGS_RELEASE "${C_FLAGS_RELEASE} -fstack-protector-all")

  set(C_FLAGS_DEBUG "${C_FLAGS_DEBUG} -O0")
  set(C_FLAGS_DEBUG "${C_FLAGS_DEBUG} -ggdb")
  set(C_FLAGS_DEBUG "${C_FLAGS_DEBUG} -fno-omit-frame-pointer")

  set(C_FLAGS_PROFILE "${C_FLAGS_PROFILE} -O0")
  set(C_FLAGS_PROFILE "${C_FLAGS_PROFILE} -ggdb")
  set(C_FLAGS_PROFILE "${C_FLAGS_PROFILE} -pg")
  set(C_FLAGS_PROFILE "${C_FLAGS_PROFILE} -fprofile-arcs")
  set(C_FLAGS_PROFILE "${C_FLAGS_PROFILE} -ftest-coverage")

  # TODO: Are these required?!
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO
    "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} -rdynamic")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -rdynamic")
endif ()

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set (C_FLAGS "${C_FLAGS} -ftemplate-depth=1024")
endif()


# assemble

set (C_FLAGS "${C_FLAGS} ${FLAGS_WARNINGS}")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${C_FLAGS_DEBUG}")
set (CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${C_FLAGS_PROFILE}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${C_FLAGS_RELEASE}")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${C_FLAGS_DEBUG}")
set (CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} ${C_FLAGS_PROFILE}")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${C_FLAGS_RELEASE}")
