include (FetchContent)

FetchContent_Declare (fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.1.1
  GIT_SHALLOW    TRUE
  SOURCE_DIR     ${FMT_DIR}
  OVERRIDE_FIND_PACKAGE
)
set (FMT_DOC OFF)
set (FMT_INSTALL ON)
set (FMT_TEST OFF)
set (FMT_FUZZ OFF)
set (FMT_CUDA_TEST OFF)
set (FMT_OS ON)
set (FMT_MODULE OFF)
set (FMT_SYSTEM_HEADERS ON)
set (FMT_UNICODE ON)
FetchContent_MakeAvailable (fmt)
target_compile_definitions (fmt PUBLIC FMT_HEADER_ONLY)
