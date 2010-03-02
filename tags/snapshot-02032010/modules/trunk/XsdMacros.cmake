# -*- mode: cmake; -*-
#
# This file contains a convenience macro for the use with the XSD schema compiler
#

macro(add_xsd_schema BASE_NAME PATH_TO_SCHEMA)
  if(XSD_FOUND)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.hpp
      COMMAND ${XSD_EXECUTABLE} cxx-tree --generate-serialization
      --hxx-suffix .hpp --cxx-suffix .cpp
      --root-element-all
      --generate-wildcard
      --reserved-name "LINUX=LINUX_OS"
      --namespace-map "http://www.xenbee.net/schema/2008/02/xbe-msg=xbemsg"
      --namespace-map "http://schemas.ggf.org/jsdl/2005/11/jsdl-posix=jsdlPosix"
      --namespace-map "http://www.w3.org/2000/09/xmldsig#=dsig"
      --namespace-map "http://www.w3.org/2001/04/xmlenc#=xenc"
      --location-map "http://www.w3.org/TR/2002/REC-xmldsig-core-20020212/xmldsig-core-schema.xsd=dsig.xsd"
      --location-map "http://schemas.ggf.org/jsdl/2005/11/jsdl=jsdl.xsd"
      --location-map "http://www.w3.org/TR/2002/REC-xmlenc-core-20021210/xenc-schema.xsd=xenc.xsd"
      ${PATH_TO_SCHEMA}
      COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.cpp
      COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.hpp
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      MAIN_DEPENDENCY ${PATH_TO_SCHEMA}
      COMMENT "Generating '${BASE_NAME}' schema data binding...")
  else(XSD_FOUND)
    if ("${PATH_TO_SCHEMA}" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.cpp")
      message(FATAL_ERROR "Data binding for the ${BASE_NAME} schema needs to be updated but XSD could not be found!")
    endif("${PATH_TO_SCHEMA}" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.cpp")
  endif(XSD_FOUND)
endmacro(add_xsd_schema)
