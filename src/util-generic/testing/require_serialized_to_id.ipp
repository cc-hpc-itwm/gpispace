// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/printer/generic.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/test/test_tools.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        //! Forwards all implementations to T, but does serialization
        //! via pointer.
        template<typename T>
          struct as_pointer
        {
          std::unique_ptr<T> impl;

          struct not_user_ctor {};
          as_pointer (not_user_ctor)
            : impl (nullptr)
          {}

          template<typename... Args>
            as_pointer (Args&&... args)
              : impl (cxx14::make_unique<T> (std::forward<Args> (args)...))
          {}

          bool operator== (as_pointer<T> const& other) const
          {
            return *impl == *other.impl;
          }

          template<typename Archive>
            void serialize (Archive& ar, unsigned int)
          {
            ar & impl;
          }
        };
      }
    }
  }
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, fhg::util::testing::detail::as_pointer<T>, os, x)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (*x.impl);
}

#define FHG_UTIL_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID( ar_               \
                                                        , def_init_         \
                                                        , init_             \
                                                        , t_...             \
                                                        )                   \
          do                                                                \
          {                                                                 \
            std::stringstream ss;                                           \
            boost::archive::ar_ ## _oarchive oa {ss};                       \
            t_ const original init_;                                        \
            oa << original;                                                 \
            boost::archive::ar_ ## _iarchive ia {ss};                       \
            t_ deserialized def_init_;                                      \
            ia >> deserialized;                                             \
            BOOST_REQUIRE_EQUAL (deserialized, original);                   \
          }                                                                 \
          while (false)

#define FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID_IMPL(initializer_, type_...) \
  FHG_UTIL_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                             \
    ( binary                                                                   \
    ,                                                                          \
    , initializer_                                                             \
    , type_                                                                    \
    );                                                                         \
  FHG_UTIL_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                             \
    ( text                                                                     \
    ,                                                                          \
    , initializer_                                                             \
    , type_                                                                    \
    );                                                                         \
  FHG_UTIL_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID (initializer_, type_)

#define FHG_UTIL_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID_IMPL( initializer_   \
                                                              , type_...       \
                                                              )                \
  FHG_UTIL_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                             \
    ( binary                                                                   \
    , (typename fhg::util::testing::detail::as_pointer<type_>::not_user_ctor{})\
    , initializer_                                                             \
    , fhg::util::testing::detail::as_pointer<type_>                            \
    );                                                                         \
  FHG_UTIL_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                             \
    ( text                                                                     \
    , (typename fhg::util::testing::detail::as_pointer<type_>::not_user_ctor{})\
    , initializer_                                                             \
    , fhg::util::testing::detail::as_pointer<type_>                            \
    )
