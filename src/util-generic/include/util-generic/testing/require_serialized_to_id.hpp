// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      //! Ensure that `type_ x initializer_;` can be serialized and
      //! deserialized and operator== on those values returns
      //! true. Repeats for both, `binary_xarchive` and
      //! `text_xarchive` and also serializes via pointer.
      //! \see FHG_UTIL_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID()
#define FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID(initializer_, type_...) \
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID_IMPL (initializer_, type_)

      //! Ensure that `make_unique<type_> (initializer_);` can be
      //! serialized and deserialized and operator== on those values
      //! returns true. Repeats for both, `binary_xarchive` and
      //! `text_xarchive`. Does not check serialization without
      //! pointer.
#define FHG_UTIL_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID(initializer_, type_...) \
  FHG_UTIL_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID_IMPL (initializer_, type_)
    }
  }
}

#include <util-generic/testing/require_serialized_to_id.ipp>
