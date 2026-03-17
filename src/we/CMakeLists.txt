gspc_add_library (NAME gspc_we
  TYPE SHARED
  SOURCES "exception.cpp"
          "expr/eval/context.cpp"
          "expr/eval/eval.cpp"
          "expr/exception.cpp"
          "expr/parse/action.cpp"
          "expr/parse/node.cpp"
          "expr/parse/parser.cpp"
          "expr/token/assoc.cpp"
          "expr/token/prec.cpp"
          "expr/token/prop.cpp"
          "expr/token/tokenizer.cpp"
          "expr/token/type.cpp"
          "expr/type/AssignResult.cpp"
          "expr/type/Context.cpp"
          "expr/type/Path.cpp"
          "expr/type/Type.cpp"
          "expr/type/infer.cpp"
          "field.cpp"
          "loader/exceptions.cpp"
          "loader/loader.cpp"
          "loader/module_call.cpp"
          "layer.cpp"
          "type/Activity.cpp"
          "type/MemoryBufferInfo.cpp"
          "type/memory_transfer.cpp"
          "type/ModuleCall.cpp"
          "require_type.cpp"
          "signature_of.cpp"
          "type/bitsetofint.cpp"
          "type/bytearray.cpp"
          "type/shared.cpp"
          "type/Expression.cpp"
          "type/literal/control.cpp"
          "type/net.cpp"
          "type/Port.cpp"
          "type/property.cpp"
          "type/Requirement.cpp"
          "type/schedule_data.cpp"
          "type/signature/complete.cpp"
          "type/signature/contains_shared.cpp"
          "${CMAKE_BINARY_DIR}/we/type/signature/cpp.cpp"
          "type/signature/dump.cpp"
          "type/signature/is_literal.cpp"
          "type/signature/names.cpp"
          "type/signature/resolve.cpp"
          "type/signature/show.cpp"
          "type/signature/specialize.cpp"
          "type/Transition.cpp"
          "type/value/function.cpp"
          "type/value/name.cpp"
          "type/value/path/append.cpp"
          "type/value/path/join.cpp"
          "type/value/path/split.cpp"
          "type/value/peek.cpp"
          "type/value/peek_or_die.cpp"
          "type/value/poke.cpp"
          "type/value/positions.cpp"
          "type/value/read.cpp"
          "type/value/show.cpp"
          "type/value/serialize.cpp"
          "type/value/dump.cpp"
          "plugin/Plugin.cpp"
          "plugin/Plugins.cpp"
          "loader/Module.cpp"
          "workflow_response.cpp"
          "type/requirements_and_preferences.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES GPISpace::header-only
            GPISpace::api_guard
            gspc_util
            gspc_scheduler_types
            gspc_worker_context
            $<$<BOOL:${GSPC_WITH_IML}>:gspc_iml_client>
            Boost::serialization
            Boost::thread
            ${CMAKE_DL_LIBS}
  COMPILE_DEFINITIONS
    -DHAS_STD_ALIGN=$<BOOL:${HAS_STD_ALIGN}>
    -DGSPC_WITH_IML=$<BOOL:${GSPC_WITH_IML}>
  INSTALL
  CREATE_BUNDLE_INFO
)

# \todo Add GSPC_EXPORT to the public API and remove this override.
set_target_properties (gspc_we
  PROPERTIES
    C_VISIBILITY_PRESET default
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN false
)

install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/expr/eval/context.hpp"
  DESTINATION include/gspc/we/expr/eval
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/expr/token/type.hpp"
  DESTINATION include/gspc/we/expr/token
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/loader/IModule.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/loader/api-guard.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/loader/macros.hpp"
  DESTINATION include/gspc/we/loader
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/bitsetofint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/bytearray.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/shared.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/shared.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/signature.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value.hpp"
  DESTINATION include/gspc/we/type
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/from_value.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/peek.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/peek_or_die.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/poke.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/read.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/serialize.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/show.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/to_value.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/unwrap.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/wrap.hpp"
  DESTINATION include/gspc/we/type/value
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/signature/show.hpp"
  DESTINATION include/gspc/we/type/signature
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/path/append.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/value/path/join.hpp"
  DESTINATION include/gspc/we/type/value/path
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/field.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/signature_of.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/we/exception.hpp"
  DESTINATION include/gspc/we
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/literal/control.hpp"
  DESTINATION include/gspc/we/type/literal
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/we/plugin/Base.hpp"
  DESTINATION include/gspc/we/plugin
)

if (GSPC_WITH_EXPRESSION_SHELL)
  gspc_add_executable (NAME gspc-expression-shell
    RUNTIME
    SOURCES "expr/shell.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${LibReadline_INCLUDE_DIR}"
    LIBRARIES gspc_we gspc_util "${LibReadline_LIBRARY}"
  )
endif()
