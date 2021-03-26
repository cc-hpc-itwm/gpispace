// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-generic/va_args.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    //! \todo is there some absurd way we could implement this?
#define REQUIRE_COMPILE_ERR(...)

    BOOST_AUTO_TEST_CASE (required_va_arg)
    {
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (1)                     );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (1,    1)            , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (1,    1, 2)         , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (1,    1, 2, 3)      , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (1,    1, 2, 3, 4)   , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (1,    1, 2, 3, 4, 5), 1);

      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (2)                     );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (2,    1)               );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (2,    1, 2)         , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (2,    1, 2, 3)      , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (2,    1, 2, 3, 4)   , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (2,    1, 2, 3, 4, 5), 2);

      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (3)                     );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (3,    1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (3,    1, 2)            );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (3,    1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (3,    1, 2, 3, 4)   , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (3,    1, 2, 3, 4, 5), 3);

      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (4)                     );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (4,    1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (4,    1, 2)            );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (4,    1, 2, 3)         );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (4,    1, 2, 3, 4)   , 4);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (4,    1, 2, 3, 4, 5), 4);

      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (5)                     );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (5,    1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (5,    1, 2)            );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (5,    1, 2, 3)         );
      REQUIRE_COMPILE_ERR (FHG_UTIL_REQUIRED_VA_ARG (5,    1, 2, 3, 4)      );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_REQUIRED_VA_ARG (5,    1, 2, 3, 4, 5), 5);
    }

    BOOST_AUTO_TEST_CASE (optional_va_arg)
    {
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (1, 1)                  );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (1, 1, 1)            , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (1, 1, 1, 2)         , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (1, 1, 1, 2, 3)      , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (1, 1, 1, 2, 3, 4)   , 1);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (2, 1)                  );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 1, 1)            , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 1, 1, 2)         , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 1, 1, 2, 3)      , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 1, 1, 2, 3, 4)   , 2);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (2, 2)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (2, 2, 1)               );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 2, 1, 2)         , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 2, 1, 2, 3)      , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (2, 2, 1, 2, 3, 4)   , 2);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 1)                  );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 1, 1)            , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 1, 1, 2)         , 1);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 1, 1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 1, 1, 2, 3, 4)   , 3);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 2)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 2, 1)               );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 2, 1, 2)         , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 2, 1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 2, 1, 2, 3, 4)   , 3);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 3)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 3, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (3, 3, 1, 2)            );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 3, 1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (3, 3, 1, 2, 3, 4)   , 3);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 2)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 2, 1)               );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 2, 1, 2)         , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 2, 1, 2, 3)      , 2);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 2, 1, 2, 3, 4)   , 4);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 2, 1, 2, 3, 4, 5), 4);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 3)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 3, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 3, 1, 2)            );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 3, 1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 3, 1, 2, 3, 4)   , 4);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 3, 1, 2, 3, 4, 5), 4);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 4)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 4, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 4, 1, 2)            );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (4, 4, 1, 2, 3)         );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 4, 1, 2, 3, 4)   , 4);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (4, 4, 1, 2, 3, 4, 5), 4);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 3)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 3, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 3, 1, 2)            );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 3, 1, 2, 3)      , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 3, 1, 2, 3, 4)   , 3);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 3, 1, 2, 3, 4, 5), 5);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 4)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 4, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 4, 1, 2)            );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 4, 1, 2, 3)         );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 4, 1, 2, 3, 4)   , 4);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 4, 1, 2, 3, 4, 5), 5);

      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 5)                  );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 5, 1)               );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 5, 1, 2)            );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 5, 1, 2, 3)         );
      REQUIRE_COMPILE_ERR (FHG_UTIL_OPTIONAL_VA_ARG (5, 5, 1, 2, 3, 4)      );
      BOOST_REQUIRE_EQUAL (FHG_UTIL_OPTIONAL_VA_ARG (5, 5, 1, 2, 3, 4, 5), 5);
    }
  }
}
