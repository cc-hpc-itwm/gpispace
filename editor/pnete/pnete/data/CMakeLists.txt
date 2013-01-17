# bernd.loerwald@itwm.fraunhofer.de

set(data_SRCS
  change_manager.cpp
  internal.cpp
  manager.cpp
)

set(data_HDRS
  change_manager.hpp
  internal.hpp
  manager.hpp
)

set (data_FWDS
  change_manager.fwd.hpp
  internal.fwd.hpp
)

set(data_TOMOC
  change_manager.hpp
)

add_subdirectory (handle)

QT4_WRAP_CPP(data_SRCS_MOC ${data_TOMOC})
qt4_automoc(${data_SRCS_MOC})

add_library ("${LIB_PREFIX}data" ${data_SRCS} ${data_SRCS_MOC})
target_link_libraries ("${LIB_PREFIX}data" "${LIB_PREFIX}data_handle")
