# Locate Xsd from code synthesis include paths and binary
# Xsd can be found at http://codesynthesis.com/products/xsd/
# Written by Frederic Heem, frederic.heem _at_ telsey.it

# This module defines
# XSD_INCLUDE_DIR, where to find elements.hxx, etc.
# XSD_EXECUTABLE, where is the xsd compiler
# XSD_FOUND, If false, don't try to use xsd

set(ADDITIONAL_XSD_PATH $ENV{XSDDIR})
#debugging
MESSAGE("XSD additional search path is: ${ADDITIONAL_XSD_PATH}")

FIND_PATH(XSD_INCLUDE_DIR xsd/cxx/parser/elements.hxx
  "[HKEY_CURRENT_USER\\software\\xsd\\include]"
  "[HKEY_CURRENT_USER]\\xsd\\include]"
  $ENV{XSD_INCLUDE_DIR}
  $ENV{XSDDIR}/libxsd
  /usr/local/include
  /usr/include
)

FIND_PROGRAM(XSD_EXECUTABLE
  NAMES 
    xsd
  PATHS
    "[HKEY_CURRENT_USER]\\xsd\\bin"
    $ENV{XSDDIR}/bin
    /usr/local/bin
    /usr/bin
    ENV XSD_BIN_DIR
  DOC "Path to the CodeSynthesis Schema Compiler"
  NO_DEFAULT_PATH
)

MESSAGE("Found XSD executable: ${XSD_EXECUTABLE}")


# if the include and the program are found then we have it
IF(XSD_INCLUDE_DIR)
  IF(XSD_EXECUTABLE)
    SET( XSD_FOUND "YES" )
  ENDIF(XSD_EXECUTABLE)
ENDIF(XSD_INCLUDE_DIR)

MARK_AS_ADVANCED(
  XSD_INCLUDE_DIR
  XSD_EXECUTABLE
)
