list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include (git)
include (git_submodules)

determine_git_revision ("${CMAKE_SOURCE_DIR}" REVISION)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/.internal/external.patch.in
    ${CMAKE_CURRENT_SOURCE_DIR}/external.patch
    @ONLY
)

list_and_store_git_submodules_if_not_exists ("git.submodules")
