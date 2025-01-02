// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/generic.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/test/test_tools.hpp>

#include <memory>
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
              : impl (std::make_unique<T> (std::forward<Args> (args)...))
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
            ::boost::archive::ar_ ## _oarchive oa {ss};                       \
            t_ const original init_;                                        \
            oa << original;                                                 \
            ::boost::archive::ar_ ## _iarchive ia {ss};                       \
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
