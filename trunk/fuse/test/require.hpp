
#ifndef _REQUIRE_HPP
#define _REQUIRE_HPP 1

#include <cstdlib>

#define REQUIRE(b) \
  if (!(b)) { std::cerr << "FAILURE in line " << __LINE__ << std::endl; \
              exit (EXIT_FAILURE); \
            }

#endif
