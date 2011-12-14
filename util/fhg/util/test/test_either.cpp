// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/either.hpp>

#include <string>
#include <iostream>

#define REQUIRE(p) if (!(p)) { std::cout << "FAILURE IN LINE " << __LINE__ << std::endl; }

int main ()
{
  typedef fhg::util::either::type<std::string, int> string_or_int_type;

  {
    string_or_int_type e ("Hi");

    REQUIRE(e.is_left());
    REQUIRE(e.left() == "Hi");

    e = "Beep";

    REQUIRE(e.is_left());
    REQUIRE(e.left() == "Beep");

    e = 42;

    REQUIRE(e.is_right());
    REQUIRE(e.right() == 42);
  }

  {
    string_or_int_type e (42);

    REQUIRE(e.is_right());
    REQUIRE(e.right() == 42);
  }

  {
    string_or_int_type e ("left");

    try
      {
        std::cout << e.right() << std::endl;

        REQUIRE (false);
      }
    catch (const boost::bad_get&)
      {
        REQUIRE (true);
      }
    catch (...)
      {
        REQUIRE (false);
      }
  }

  return EXIT_SUCCESS;
}
