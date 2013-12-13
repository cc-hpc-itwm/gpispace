# mirko.rahn@itwm.fraunhofer.de

add_subdirectory (man)
add_subdirectory (xsd)
add_subdirectory (lib)

install ( FILES common.mk
          DESTINATION share/sdpa/make
          COMPONENT sdk
        )

