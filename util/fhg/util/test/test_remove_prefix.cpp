// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/remove_prefix.hpp>

#include <string>
#include <iostream>

#include <cstdlib>

#define REQUIRE(p)                                                      \
  if (!(p)) { std::cout << "FAILURE IN LINE " << __LINE__ << std::endl; }

int main ()
{
  const std::string word ("ababab");

  {
    const std::string prefix ("");

    REQUIRE (fhg::util::remove_prefix (prefix, word) == "ababab");
  }

  {
    const std::string prefix ("a");

    REQUIRE (fhg::util::remove_prefix (prefix, word) == "babab");
  }

  {
    const std::string prefix ("abab");

    REQUIRE (fhg::util::remove_prefix (prefix, word) == "ab");
  }

  {
    const std::string prefix ("ababab");

    REQUIRE (fhg::util::remove_prefix (prefix, word) == "");
  }

  {
    const std::string prefix ("abababab");

    try
      {
        fhg::util::remove_prefix (prefix, word);

        REQUIRE (false);
      }
    catch (const std::string& s)
      {
        REQUIRE (s == "remove_prefix failed, rest: prefix = ab, word = ");
      }
  }

  {
    const std::string prefix ("A");

    try
      {
        fhg::util::remove_prefix (prefix, word);

        REQUIRE (false);
      }
    catch (const std::string& s)
      {
        REQUIRE (s == "remove_prefix failed, rest: prefix = A, word = ababab");
      }
  }

  return EXIT_SUCCESS;
}
