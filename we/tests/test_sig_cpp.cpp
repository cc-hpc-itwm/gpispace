// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/cpp.hpp>

#include <iostream>

int
main ()
{
  signature::structured_t point;
  point["x"] = literal::DOUBLE();
  point["y"] = literal::DOUBLE();

  signature::structured_t line;
  line["start"] = point;
  line["end"] = point;
  line["descr"] = literal::STRING();

  signature::structured_t cross;
  cross["h"] = line;
  cross["v"] = line;
  cross["_control"] = literal::CONTROL();
  cross["_bool"] = literal::BOOL();
  cross["_long"] = literal::LONG();
  cross["_char"] = literal::CHAR();
  cross["_double"] = literal::DOUBLE();
  cross["bitset"] = literal::BITSET();

  signature::type sig (cross, "cross");

  signature::cpp::cpp_header (std::cout, sig);

  return EXIT_SUCCESS;
}
