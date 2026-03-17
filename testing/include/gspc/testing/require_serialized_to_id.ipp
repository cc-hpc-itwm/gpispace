#include <gspc/testing/printer/generic.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/test/test_tools.hpp>

#include <memory>
#include <sstream>




      namespace gspc::testing::detail
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




GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, gspc::testing::detail::as_pointer<T>, os, x)
{
  os << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (*x.impl);
}

#define GSPC_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID( ar_                      \
                                                    , def_init_                \
                                                    , init_                    \
                                                    , t_...                    \
                                                    )                          \
          do                                                                   \
          {                                                                    \
            std::stringstream ss;                                              \
            ::boost::archive::ar_ ## _oarchive oa {ss};                        \
            t_ const original init_;                                           \
            oa << original;                                                    \
            ::boost::archive::ar_ ## _iarchive ia {ss};                        \
            t_ deserialized def_init_;                                         \
            ia >> deserialized;                                                \
            BOOST_REQUIRE_EQUAL (deserialized, original);                      \
          }                                                                    \
          while (false)

#define GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID_IMPL(initializer_, type_...)     \
  GSPC_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                                 \
    ( binary                                                                   \
    ,                                                                          \
    , initializer_                                                             \
    , type_                                                                    \
    );                                                                         \
  GSPC_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                                 \
    ( text                                                                     \
    ,                                                                          \
    , initializer_                                                             \
    , type_                                                                    \
    );                                                                         \
  GSPC_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID (initializer_, type_)

#define GSPC_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID_IMPL( initializer_       \
                                                          , type_...           \
                                                          )                    \
  GSPC_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                                 \
    ( binary                                                                   \
    , (typename gspc::testing::detail::as_pointer<type_>::not_user_ctor{})     \
    , initializer_                                                             \
    , gspc::testing::detail::as_pointer<type_>                                 \
    );                                                                         \
  GSPC_TESTING_DETAIL_REQUIRE_SERIALIZED_TO_ID                                 \
    ( text                                                                     \
    , (typename gspc::testing::detail::as_pointer<type_>::not_user_ctor{})     \
    , initializer_                                                             \
    , gspc::testing::detail::as_pointer<type_>                                 \
    )
