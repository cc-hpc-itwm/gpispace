// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/cpp.hpp>

#include <iostream>

int
main ()
{
  signature::structured_t point;
  point.insert ("x", literal::DOUBLE());
  point.insert ("y", literal::DOUBLE());

  signature::structured_t line;
  line.insert ("start", point);
  line.insert ("end", point);
  line.insert ("descr", literal::STRING());

  signature::structured_t cross;
  cross.insert ("h", line);
  cross.insert ("v", line);
  cross.insert ("_control", literal::CONTROL());
  cross.insert ("_bool", literal::BOOL());
  cross.insert ("_long", literal::LONG());
  cross.insert ("_char", literal::CHAR());
  cross.insert ("_double", literal::DOUBLE());
  cross.insert ("bitset", literal::BITSET());

  signature::type sig (cross, "cross");

  signature::cpp::cpp_header ( std::cout
                             , sig
                             , sig.nice()
                             , boost::filesystem::path ("")
                             , boost::filesystem::path ("")
                             );

  return EXIT_SUCCESS;
}
