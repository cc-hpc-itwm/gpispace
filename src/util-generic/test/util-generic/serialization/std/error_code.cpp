// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/serialization/std/error_code.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (can_handle_standard_error_codes)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, std::error_code);

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({std::error_code (314, std::generic_category())}, std::error_code);

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({std::make_error_code (std::errc::inappropriate_io_control_operation)}, std::error_code);

  //! \todo Bug in stdlibcxx of gcc 4.8: std::ios_base::failure
  //! does not inherit from std::system_error as defined in c++11,
  //! thus this category is not yet defined.
  // FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
  //   ({std::make_error_code (std::io_errc::stream)}, std::error_code);

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({std::make_error_code (std::future_errc::broken_promise)}, std::error_code);
}

namespace
{
  struct : std::error_category
  {
    virtual const char* name() const noexcept override
    {
      return __PRETTY_FUNCTION__;
    }
    virtual std::string message (int) const override
    {
      throw std::logic_error ("will never be called");
    }
  } custom_error_category;
}

BOOST_AUTO_TEST_CASE (cant_handle_custom_error_categories)
{
  std::error_code const ec (0, custom_error_category);

  std::stringstream ss;
  ::boost::archive::binary_oarchive oa {ss};
  oa << ec;
  ::boost::archive::binary_iarchive ia {ss};
  std::error_code deserialized;

  fhg::util::testing::require_exception
    ( [&] { ia >> deserialized; }
    , std::logic_error ("unknown std::error_category")
    );
}
