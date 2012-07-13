# -*- mode: cmake; -*-
set(EXTERNAL_SOFTWARE "$ENV{HOME}/external_software/")
set(COMPILER_ID "intel")
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/SDPANightly.cmake")
