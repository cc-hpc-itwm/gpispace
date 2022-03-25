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

#include <util-generic/serialization/exception.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <exception>
#include <future>

#define REQUIRE_ID(_type, ...)                                        \
  do                                                                  \
  {                                                                   \
    _type ex {__VA_ARGS__};                                           \
    fhg::util::testing::require_exception                             \
      ( [&]                                                           \
        {                                                             \
          std::rethrow_exception                                      \
            ( fhg::util::serialization::exception::deserialize        \
                ( fhg::util::serialization::exception::serialize      \
                    (std::make_exception_ptr (ex))                    \
                )                                                     \
            );                                                        \
        }                                                             \
      , ex                                                            \
      );                                                              \
  }                                                                   \
  while (0)


BOOST_AUTO_TEST_CASE (std_exceptions)
{
  REQUIRE_ID (std::bad_alloc);
  REQUIRE_ID (std::bad_cast);
  REQUIRE_ID (std::bad_exception);
  REQUIRE_ID (std::bad_function_call);
  REQUIRE_ID (std::bad_typeid);
  REQUIRE_ID (std::bad_weak_ptr);
  REQUIRE_ID (std::logic_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::domain_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::future_error, std::future_errc::future_already_retrieved);
  REQUIRE_ID (std::invalid_argument, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::length_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::out_of_range, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::runtime_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::overflow_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::range_error, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::system_error);
  //! \todo Bug in stdlibcxx of gcc 4.8:
  //! std::ios_base::failure does not inherit from std::system_error,
  //! thus the test fails with new versions of stdlibcxx after the
  //! fix. To avoid weird version detection, just ignore that case for now.
  // REQUIRE_ID (std::ios_base::failure, fhg::util::testing::random<std::string>{}());
  REQUIRE_ID (std::underflow_error, fhg::util::testing::random<std::string>{}());
}

BOOST_AUTO_TEST_CASE (issue_3)
{
  REQUIRE_ID (std::runtime_error, "issue\03");
}
BOOST_AUTO_TEST_CASE (just_zero)
{
  REQUIRE_ID (std::runtime_error, "\0");
}
BOOST_AUTO_TEST_CASE (two_zero)
{
  REQUIRE_ID (std::runtime_error, "\0\0");
}

#undef REQUIRE_ID

BOOST_AUTO_TEST_CASE (nested)
{
  std::exception_ptr ex;
  std::string const inner {fhg::util::testing::random<std::string>{}()};
  std::string const outer {fhg::util::testing::random<std::string>{}()};
  try
  {
    try
    {
      throw std::runtime_error (inner);
    }
    catch (...)
    {
      std::throw_with_nested (std::runtime_error (outer));
    }
  }
  catch (...)
  {
    ex = std::current_exception();
  }

  fhg::util::testing::require_exception
    ( [&]
      {
        std::rethrow_exception
          ( fhg::util::serialization::exception::deserialize
              (fhg::util::serialization::exception::serialize (ex))
          );
      }
    , fhg::util::testing::make_nested ( std::runtime_error (outer)
                                      , std::runtime_error (inner)
                                      )
    );
}

#define raw_exception_class_content(_type)                              \
    _type (int d)                                                       \
      : dummy (d)                                                       \
    {}                                                                  \
    virtual ~_type() = default;                                         \
    _type (_type const&) = default;                                     \
                                                                        \
    std::string what() const                                            \
    {                                                                   \
      return "dummy = " + std::to_string (dummy);                       \
    }                                                                   \
    int dummy

#define std_exception_class_content(_type)                              \
    _type (int d)                                                       \
      : dummy (d)                                                       \
      , _message ("dummy = " + std::to_string (dummy))                  \
    {}                                                                  \
    virtual ~_type() override = default;                                \
    _type (_type const&) = default;                                     \
                                                                        \
    virtual char const* what() const noexcept override                  \
    {                                                                   \
      return _message.c_str();                                          \
    }                                                                   \
    int dummy;                                                          \
    std::string _message

#define std_logic_error_exception_class_content(_type)                  \
    _type (int d)                                                       \
      : std::logic_error ("dummy = " + std::to_string (d))              \
      , dummy (d)                                                       \
    {}                                                                  \
    virtual ~_type() override = default;                                \
    _type (_type const&) = default;                                     \
    int dummy

#define boost_serialization(_type)                                      \
    template<typename Ar>                                               \
    void serialize (Ar& ar, unsigned int)                               \
    {                                                                   \
      ar & dummy;                                                       \
      *this = _type (dummy);                                            \
    }                                                                   \
    _type& operator= (_type const&) = default

#define to_from_string(_type)                                           \
    static _type from_string (std::string const& serialized)            \
    {                                                                   \
      return _type (std::stoi (serialized));                            \
    }                                                                   \
    std::string to_string() const                                       \
    {                                                                   \
      return std::to_string (dummy);                                    \
    }                                                                   \
    _type() = delete

#define serialize_deserialize_with_archive(_type)                       \
    template<typename Archive> static _type deserialize (Archive& ar)   \
    {                                                                   \
      int d; ar >> d;                                                   \
      return _type (d);                                                 \
    }                                                                   \
    template<typename Archive> void serialize (Archive& ar) const       \
    {                                                                   \
      ar << dummy;                                                      \
    }                                                                   \
    _type() = delete

#define static_functions(_type)                                         \
    static ::boost::optional<std::string> from_exception_ptr              \
      (std::exception_ptr ex_ptr)                                       \
    {                                                                   \
      try                                                               \
      {                                                                 \
        std::rethrow_exception (ex_ptr);                                \
      }                                                                 \
      catch (_type const& ex)                                           \
      {                                                                 \
        return std::to_string (ex.dummy);                               \
      }                                                                 \
      catch (...)                                                       \
      {                                                                 \
        return ::boost::none;                                             \
      }                                                                 \
    }                                                                   \
    [[noreturn]] static void throw_with_nested (std::string serialized) \
    {                                                                   \
      std::throw_with_nested (user_defined_exception (std::stoi (serialized))); \
    }                                                                   \
    static std::exception_ptr to_exception_ptr (std::string serialized) \
    {                                                                   \
      return std::make_exception_ptr (_type (std::stoi (serialized)));  \
    }                                                                   \
    _type() = delete

namespace
{
  struct user_defined_exception
  {
    raw_exception_class_content (user_defined_exception);
    static_functions (user_defined_exception);
  };
  struct user_defined_std_logic_error : std::logic_error
  {
    std_logic_error_exception_class_content (user_defined_std_logic_error);
    static_functions (user_defined_std_logic_error);
  };
}

#define REQUIRE_ID(_type, _functions, ...)                              \
  do                                                                    \
  {                                                                     \
    _type ex {__VA_ARGS__};                                             \
                                                                        \
    fhg::util::testing::require_exception                               \
      ( [&]                                                             \
        {                                                               \
          std::rethrow_exception                                        \
            ( fhg::util::serialization::exception::deserialize          \
                ( fhg::util::serialization::exception::serialize        \
                    (std::make_exception_ptr (ex), _functions)          \
                , _functions                                            \
                )                                                       \
            );                                                          \
        }                                                               \
      , ex                                                              \
      );                                                                \
  }                                                                     \
  while (false)


BOOST_AUTO_TEST_CASE (user_defined)
{
  fhg::util::serialization::exception::serialization_functions functions
    ( { { "ude"
        , { &user_defined_exception::from_exception_ptr
          , &user_defined_exception::to_exception_ptr
          , &user_defined_exception::throw_with_nested
          }
        }
      , { "udsrte"
        , { &user_defined_std_logic_error::from_exception_ptr
          , &user_defined_std_logic_error::to_exception_ptr
          , &user_defined_std_logic_error::throw_with_nested
          }
        }
      }
    );

  REQUIRE_ID (user_defined_exception, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (user_defined_std_logic_error, functions, fhg::util::testing::random<int>{}());
}

namespace
{
  struct with_boost_serialization
  {
    raw_exception_class_content (with_boost_serialization);
    boost_serialization (with_boost_serialization);

    with_boost_serialization() = default;
  };
  struct with_boost_serialization_std_exception : public std::exception
  {
    std_exception_class_content (with_boost_serialization_std_exception);
    boost_serialization (with_boost_serialization_std_exception);

    with_boost_serialization_std_exception() = default;
  };
  struct with_boost_serialization_std_logic_error : public std::logic_error
  {
    std_logic_error_exception_class_content
      (with_boost_serialization_std_logic_error);
    boost_serialization (with_boost_serialization_std_logic_error);

    with_boost_serialization_std_logic_error()
      : std::logic_error ("un-deserialized")
      , dummy (fhg::util::testing::random<int>{}())
    {}
  };

  struct with_to_from_string
  {
    raw_exception_class_content (with_to_from_string);
    to_from_string (with_to_from_string);
  };
  struct with_to_from_string_std_exception : public std::exception
  {
    std_exception_class_content (with_to_from_string_std_exception);
    to_from_string (with_to_from_string_std_exception);
  };
  struct with_to_from_string_std_logic_error : public std::logic_error
  {
    std_logic_error_exception_class_content
      (with_to_from_string_std_logic_error);
    to_from_string (with_to_from_string_std_logic_error);
  };

  struct with_static_functions
  {
    raw_exception_class_content (with_static_functions);
    static_functions (with_static_functions);
  };
  struct with_static_functions_std_exception : public std::exception
  {
    std_exception_class_content (with_static_functions_std_exception);
    static_functions (with_static_functions_std_exception);
  };
  struct with_static_functions_std_logic_error : public std::logic_error
  {
    std_logic_error_exception_class_content
      (with_static_functions_std_logic_error);
    static_functions (with_static_functions_std_logic_error);
  };

  struct with_serialize_deserialize_with_archive
  {
    raw_exception_class_content (with_serialize_deserialize_with_archive);
    serialize_deserialize_with_archive (with_serialize_deserialize_with_archive);
  };
  struct with_serialize_deserialize_with_archive_std_exception : public std::exception
  {
    std_exception_class_content (with_serialize_deserialize_with_archive_std_exception);
    serialize_deserialize_with_archive (with_serialize_deserialize_with_archive_std_exception);
  };
  struct with_serialize_deserialize_with_archive_std_logic_error : public std::logic_error
  {
    std_logic_error_exception_class_content
      (with_serialize_deserialize_with_archive_std_logic_error);
    serialize_deserialize_with_archive (with_serialize_deserialize_with_archive_std_logic_error);
  };
}

BOOST_AUTO_TEST_CASE (serialization_helper)
{
  fhg::util::serialization::exception::serialization_functions const functions
    ( { fhg::util::serialization::exception::make_functions<with_boost_serialization>()
      , fhg::util::serialization::exception::make_functions<with_boost_serialization_std_exception>()
      , fhg::util::serialization::exception::make_functions<with_boost_serialization_std_logic_error>()
      , fhg::util::serialization::exception::make_functions<with_static_functions>()
      , fhg::util::serialization::exception::make_functions<with_static_functions_std_exception>()
      , fhg::util::serialization::exception::make_functions<with_static_functions_std_logic_error>()
      , fhg::util::serialization::exception::make_functions<with_to_from_string>()
      , fhg::util::serialization::exception::make_functions<with_to_from_string_std_exception>()
      , fhg::util::serialization::exception::make_functions<with_to_from_string_std_logic_error>()
      , fhg::util::serialization::exception::make_functions<with_serialize_deserialize_with_archive>()
      , fhg::util::serialization::exception::make_functions<with_serialize_deserialize_with_archive_std_exception>()
      , fhg::util::serialization::exception::make_functions<with_serialize_deserialize_with_archive_std_logic_error>()
      }
    );

  REQUIRE_ID (with_boost_serialization, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_boost_serialization_std_exception, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_boost_serialization_std_logic_error, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_static_functions, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_static_functions_std_exception, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_static_functions_std_logic_error, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_to_from_string, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_to_from_string_std_exception, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_to_from_string_std_logic_error, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_serialize_deserialize_with_archive, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_serialize_deserialize_with_archive_std_exception, functions, fhg::util::testing::random<int>{}());
  REQUIRE_ID (with_serialize_deserialize_with_archive_std_logic_error, functions, fhg::util::testing::random<int>{}());
}
