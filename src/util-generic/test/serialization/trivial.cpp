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

#include <util-generic/serialization/trivial.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_compiletime.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace
      {
        struct user_defined_struct_base
        {
          std::uintptr_t _original_object_address;

          int serialize_call_count;
          user_defined_struct_base (int x = 0)
            : _original_object_address (reinterpret_cast<std::uintptr_t> (this))
            , serialize_call_count (x)
          {}

          template<typename Archive>
            void serialize (Archive& ar, unsigned int)
          {
            ar & _original_object_address;

            if (typename Archive::is_saving{})
            {
              ++serialize_call_count;
            }
            ar & serialize_call_count;
            if (typename Archive::is_loading{})
            {
              ++serialize_call_count;
            }
          }

          bool operator== (user_defined_struct_base const& other) const
          {
            return _original_object_address == other._original_object_address;
          }

          friend std::ostream& operator<<
            (std::ostream& os, user_defined_struct_base const& x)
          {
            return os << x._original_object_address;
          }
        };

        struct non_trivial_user_defined_struct : user_defined_struct_base
        {
          using user_defined_struct_base::user_defined_struct_base;
        };
        struct trivial_user_defined_struct : user_defined_struct_base
        {
          using user_defined_struct_base::user_defined_struct_base;
        };
      }
    }
  }
}

FHG_UTIL_SERIALIZATION_TRIVIAL
  ( fhg::util::serialization::user_defined_struct_base
  , &fhg::util::serialization::user_defined_struct_base::serialize_call_count
  )

FHG_UTIL_SERIALIZATION_TRIVIAL
  ( fhg::util::serialization::trivial_user_defined_struct
  , fhg::util::serialization::base_class
      <fhg::util::serialization::user_defined_struct_base>()
  )

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      BOOST_AUTO_TEST_CASE (is_trivially_serializable_true_for_pods_or_tagged)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<>{}, true);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<int>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<float>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<std::size_t>{}, true);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<trivial_user_defined_struct>{}, true);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<int, float, std::size_t>{}, true);
      }

      BOOST_AUTO_TEST_CASE (is_trivially_is_false_as_soon_as_one_type_is_not)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<std::vector<std::string>>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<decltype (std::cout)>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<char[10]>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<int*>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<std::vector<std::string>, int>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<int, std::vector<std::string>>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<std::vector<std::string>, int*>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (is_trivially_serializable<non_trivial_user_defined_struct>{}, false);
      }

      BOOST_AUTO_TEST_CASE (trivially_serialized_structs_serialize_to_id)
      {
        FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
          ((fhg::util::testing::random<int>{}()), user_defined_struct_base);
        FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
          ((fhg::util::testing::random<int>{}()), trivial_user_defined_struct);
        FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
          ((fhg::util::testing::random<int>{}()), non_trivial_user_defined_struct);
      }

      //! \note Is testing boost.serialize, not something we do.
      BOOST_AUTO_TEST_CASE (serialize_is_only_called_for_non_trivial_types)
      {
        std::stringstream ss;
        {
          non_trivial_user_defined_struct x (0);
          trivial_user_defined_struct y (0);

          boost::archive::binary_oarchive oa (ss);
          oa & x;
          oa & y;

          BOOST_CHECK_EQUAL (x.serialize_call_count, 1);
          BOOST_CHECK_EQUAL (y.serialize_call_count, 0);
        }

        {
          non_trivial_user_defined_struct x (-1);
          trivial_user_defined_struct y (-1);

          boost::archive::binary_iarchive ia (ss);
          ia & x;
          ia & y;

          BOOST_CHECK_EQUAL (x.serialize_call_count, 2);
          BOOST_CHECK_EQUAL (y.serialize_call_count, 0);
        }
      }

      //! \todo FHG_UTIL_SERIALIZATION_TRIVIAL shall fail compilation
      //! if a member or base is not trivially serializable.
    }
  }
}
