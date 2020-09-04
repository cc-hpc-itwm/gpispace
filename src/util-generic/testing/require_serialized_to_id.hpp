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
