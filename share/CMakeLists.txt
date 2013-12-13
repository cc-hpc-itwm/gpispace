# mirko.rahn@itwm.fraunhofer.de

add_subdirectory (man)
add_subdirectory (xsd)

install ( FILES common.mk
          DESTINATION share/sdpa/make
          COMPONENT sdk
        )

