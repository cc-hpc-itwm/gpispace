# bernd.loerwald@itwm.fraunhofer.de

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${FhgLog_INCLUDE_DIR})

include_directories(${UTIL_INCLUDE_DIR})

find_package(HWLOC REQUIRED QUIET)
include_directories(SYSTEM ${HWLOC_INCLUDE_DIR})

find_package (NACL QUIET)
if (NACL_FOUND)
  if (ENABLE_LICENSE_CHECK)
    include_directories (${NACL_INCLUDE_DIR})
  endif()
else()
  set(ENABLE_LICENSE_CHECK CACHE BOOL OFF FORCE)
endif()

add_subdirectory(plugin)

add_subdirectory (fhgcom)

OPTION (ENABLE_REAL_GPI "Provide api for the *real* GPI" ON)
OPTION (ENABLE_COMPATIBILITY_API "Provide a transitional api compatible with the old fvm-pc" ON)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

if (ENABLE_REAL_GPI)
  find_package (IBVerbs)
  if (IBverbs_FOUND)
    set (GPI_HOME ${CMAKE_CURRENT_SOURCE_DIR}/gpi-space/ext)
    set (GPI_INCLUDE_DIR ${GPI_HOME}/include)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libGPI.a)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${IBverbs_LIBRARY})

    # check if we can actually use the GPI version in ext
    set (CMAKE_REQUIRED_INCLUDES ${GPI_INCLUDE_DIR})
    set (CMAKE_REQUIRED_LIBRARIES ${GPI_LIBRARIES})
    include (CheckCSourceRuns)
    CHECK_C_SOURCE_RUNS (
      "
#include <stdio.h>
#include <GPI.h>
int main ()
{
   printf(\"%f\", getVersionGPI());
   return 0;
}"
    REAL_GPI_CHECK)
    if ("${REAL_GPI_CHECK}" EQUAL 1)
      add_definitions (-DENABLE_REAL_GPI=1)
    else()
      message (WARNING "Support for real-GPI requested but not available on your platform!")
      set (ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
    endif()
  else (IBverbs_FOUND)
    message (STATUS "${IBverbs_FOUND}")
    message (WARNING "Support for real-GPI requested but not available on your platform (ibverbs not installed)!")
    set (ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
  endif()
endif()

add_subdirectory (gpi-space)

  find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED QUIET)

  include_directories ( ${CMAKE_SOURCE_DIR}/sdpa
                      )
  set (LIB_PREFIX "fhg_pnete_")

  include(${QT_USE_FILE})

  add_subdirectory (pnete)
  add_subdirectory (util)

add_library (pnet
  we/exception.cpp
  we/expr/eval/context.cpp
  we/expr/eval/eval.cpp
  we/expr/eval/refnode.cpp
  we/expr/exception.cpp
  we/expr/parse/action.cpp
  we/expr/parse/node.cpp
  we/expr/parse/parser.cpp
  we/expr/parse/simplify/constant_propagation.cpp
  we/expr/parse/simplify/copy_propagation.cpp
  we/expr/parse/simplify/dead_code_elimination.cpp
  we/expr/parse/simplify/expression_list.cpp
  we/expr/parse/simplify/simplify.cpp
  we/expr/parse/simplify/util.cpp
  we/expr/token/assoc.cpp
  we/expr/token/prec.cpp
  we/expr/token/prop.cpp
  we/expr/token/tokenizer.cpp
  we/expr/token/type.cpp
  we/field.cpp
  we/loader/loader.cpp
  we/layer.cpp
  we/type/activity.cpp
  we/require_type.cpp
  we/signature_of.cpp
  we/type/bitsetofint.cpp
  we/type/bytearray.cpp
  we/type/connection.cpp
  we/type/expression.cpp
  we/type/literal/control.cpp
  we/type/net.cpp
  we/type/place.cpp
  we/type/port.cpp
  we/type/property.cpp
  we/type/schedule_data.cpp
  we/type/signature/complete.cpp
  we/type/signature/cpp.cpp
  we/type/signature/dump.cpp
  we/type/signature/is_literal.cpp
  we/type/signature/name.cpp
  we/type/signature/names.cpp
  we/type/signature/resolve.cpp
  we/type/signature/show.cpp
  we/type/signature/signature.cpp
  we/type/signature/specialize.cpp
  we/type/transition.cpp
  we/type/value/function.cpp
  we/type/value/name.cpp
  we/type/value/name_of.cpp
  we/type/value/of_type.cpp
  we/type/value/path/append.cpp
  we/type/value/path/join.cpp
  we/type/value/path/split.cpp
  we/type/value/peek.cpp
  we/type/value/poke.cpp
  we/type/value/positions.cpp
  we/type/value/read.cpp
  we/type/value/remove.cpp
  we/type/value/show.cpp
  we/type/value/dump.cpp

  we/loader/Module.cpp

  xml/parse/error.cpp
  xml/parse/id/generic.cpp
  xml/parse/id/mapper.cpp
  xml/parse/id/types.cpp
  xml/parse/parser.cpp
  xml/parse/state.cpp
  xml/parse/type/connect.cpp
  xml/parse/type/expression.cpp
  xml/parse/type/function.cpp
  xml/parse/type/mod.cpp
  xml/parse/type/net.cpp
  xml/parse/type/place.cpp
  xml/parse/type/place_map.cpp
  xml/parse/type/port.cpp
  xml/parse/type/require.cpp
  xml/parse/type/specialize.cpp
  xml/parse/type/struct.cpp
  xml/parse/type/template.cpp
  xml/parse/type/transition.cpp
  xml/parse/type/use.cpp
  xml/parse/type/link.cpp
  xml/parse/type/with_position_of_definition.cpp
  xml/parse/util/mk_fstream.cpp
  xml/parse/util/valid_name.cpp
  xml/parse/util/cdata.cpp
  xml/parse/util/expect.cpp
  xml/parse/util/name_element.cpp
  xml/parse/util/optional.cpp
  xml/parse/util/required.cpp
  xml/parse/util/skip.cpp
  xml/parse/util/validprefix.cpp
  xml/parse/util/validstructfield.cpp
  xml/parse/util/property.cpp
  xml/parse/util/show_node_type.cpp
  xml/parse/util/weparse.cpp
  xml/parse/util/position.cpp
  xml/parse/warning.cpp
)

target_link_libraries (pnet
  fhg-util
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${CMAKE_DL_LIBS}
)

add_subdirectory (xml/tests)
add_subdirectory (xml/parse/type/test)

add_subdirectory(pnetv)
add_subdirectory (we)
add_subdirectory (mmgr)

add_subdirectory (drts)
add_subdirectory (bin)
