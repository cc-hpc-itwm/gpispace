extended_add_executable (NAME dummy_server
  SOURCES dummy_server.cpp
  MOC dummy_server.hpp
  LIBRARIES fhg_util_qt
            fhg-util
            Util::Generic
            Qt5::Gui
            Qt5::Widgets
            Qt5::Network
)

#extended_add_executable (NAME serialize_without_default_ctor
#  SOURCES serialize_without_default_ctor.cpp
#  LIBRARIES Boost::serialization Util::Generic
#)
