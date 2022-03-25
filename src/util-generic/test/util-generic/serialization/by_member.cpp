// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/serialization/by_member.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace
      {
        std::size_t current_index = 0;

        template<int index>
          struct member
        {
          static std::size_t serialization_index;
          static std::size_t deserialization_index;

          template<typename Archive>
            void serialize (Archive&, unsigned int)
          {
            if (typename Archive::is_saving{})
            {
              serialization_index = current_index++;
            }
            else
            {
              deserialization_index = current_index++;
            }
          }
        };
        template<int index>
          std::size_t member<index>::serialization_index = 0;
        template<int index>
          std::size_t member<index>::deserialization_index = 0;

        struct order_checker : member<-1>, member<-2>
        {
          member<0> _0;
          member<1> _1;
          member<2> _2;
        };
      }
    }
  }
}

//! \note Order in serializer does not match order in class!
FHG_UTIL_SERIALIZATION_BY_MEMBER
  ( fhg::util::serialization::order_checker
  , fhg::util::serialization::base_class<fhg::util::serialization::member<-1>>()
  , &fhg::util::serialization::order_checker::_0
  , &fhg::util::serialization::order_checker::_2
  , &fhg::util::serialization::order_checker::_1
  , fhg::util::serialization::base_class<fhg::util::serialization::member<-2>>()
  )

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      BOOST_AUTO_TEST_CASE (serializes_given_bases_and_members_in_order)
      {
        current_index = 0;

        std::stringstream ss;
        ::boost::archive::binary_oarchive oa {ss};

        order_checker const original{};
        oa << original;

        ::boost::archive::binary_iarchive ia {ss};
        order_checker deserialized;
        ia >> deserialized;

        BOOST_REQUIRE_EQUAL
          (order_checker::member<-1>::serialization_index, 0);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<0>::serialization_index, 1);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<2>::serialization_index, 2);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<1>::serialization_index, 3);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<-2>::serialization_index, 4);

        BOOST_REQUIRE_EQUAL
          (order_checker::member<-1>::deserialization_index, 5);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<0>::deserialization_index, 6);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<2>::deserialization_index, 7);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<1>::deserialization_index, 8);
        BOOST_REQUIRE_EQUAL
          (order_checker::member<-2>::deserialization_index, 9);
      }

      //! \todo Having an unrelated class or member in
      //! base_or_members... shall fail compilation.

      //! \todo Having an unserializable class or member in
      //! base_or_members... shall fail compilation.

      //! \todo Codegen shall be the same as with manually written
      //! serialize overload listing given bases and members.
    }
  }
}
