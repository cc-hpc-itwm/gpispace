# Locate XML-Security-C include paths  and libraries XML-Security-C can be
# found  at http://santuario.apache.org/c/index.html

# Adapted by  Alexander Petry petry _at_ itwm.fhg.de,  original version by
# Frederic Heem, frederic.heem _at_ telsey.it

# This module defines
# XMLSecurityC_INCLUDE_DIR, where to find xsec/framework/XSECConfig.hpp, etc.
# XMLSecurityC_LIBRARIES, the libraries to link against to use xml security.
# XMLSecurityC_FOUND, If false, don't try to use xml-security.

FIND_PATH(XMLSecurityC_INCLUDE_DIR xsec/framework/XSECConfig.hpp
  "[HKEY_CURRENT_USER\\software\\xml-security-c\\src]"
  "[HKEY_CURRENT_USER\\xml-security-c\\src]"
  $ENV{XMLSECROOT}/src/
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(XMLSecurityC_LIBRARIES
  NAMES 
    xml-security-c
  PATHS
    "[HKEY_CURRENT_USER\\software\\xml-security-c\\lib]"
    "[HKEY_CURRENT_USER\\xml-security-c\\lib]"
    $ENV{XMLSECROOT}/lib
    /usr/local/lib
    /usr/lib
)

# if the include and the library are found then we have it
IF(XMLSecurityC_INCLUDE_DIR)
  IF(XMLSecurityC_LIBRARIES)
    SET( XMLSecurityC_FOUND "YES" )
  ENDIF(XMLSecurityC_LIBRARIES)
ENDIF(XMLSecurityC_INCLUDE_DIR)



MARK_AS_ADVANCED(
  XMLSecurityC_INCLUDE_DIR
  XMLSecurityC_LIBRARIES
)
