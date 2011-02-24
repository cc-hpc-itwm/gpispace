// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_NAME_HPP
#define _WE_TYPE_LITERAL_NAME_HPP

#include <string>

namespace literal
{
  typedef std::string type_name_t;

  static const type_name_t CONTROL   ("control");
  static const type_name_t BOOL      ("bool");
  static const type_name_t LONG      ("long");
  static const type_name_t DOUBLE    ("double");
  static const type_name_t CHAR      ("char");
  static const type_name_t STRING    ("string");
  static const type_name_t BITSET    ("bitset");
  static const type_name_t STACK     ("stack");
  static const type_name_t MAP       ("map");
  static const type_name_t SET       ("set");
  static const type_name_t BYTEARRAY ("bytearray");
}

#endif
