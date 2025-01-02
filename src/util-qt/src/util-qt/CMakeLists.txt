extended_add_library (NAME Qt
  NAMESPACE Util
  INCLUDE_DIRECTORIES
    PRIVATE "${PROJECT_SOURCE_DIR}/src"
  SOURCES
    "do_after_event_loop.cpp"
    "make_drop_target.cpp"
    "style.cpp"
    "testing/Application.cpp"
    "testing/CoreApplication.cpp"
    "testing/detail/FakeCommandLineArguments.cpp"
    "testing/detail/MinimalPlatformInitializer.cpp"
    "widget/file_line_edit.cpp"
    "widget/mini_button.cpp"
  MOC
    "${PROJECT_SOURCE_DIR}/include/util-qt/widget/file_line_edit.hpp"
    "${PROJECT_SOURCE_DIR}/include/util-qt/widget/mini_button.hpp"
  LIBRARIES
    PUBLIC
      Util::Qt-Headers
    PUBLIC fmt
  INSTALL
)
target_compile_features (Util-Qt
  PUBLIC "${_project_cxx_std}"
)
