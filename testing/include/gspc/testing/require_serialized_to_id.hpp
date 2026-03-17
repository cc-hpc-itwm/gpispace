// Copyright (C) 2016,2018-2019,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once



    namespace gspc::testing
    {
      //! Ensure that `type_ x initializer_;` can be serialized and
      //! deserialized and operator== on those values returns
      //! true. Repeats for both, `binary_xarchive` and
      //! `text_xarchive` and also serializes via pointer.
      //! \see GSPC_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID()
#define GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID(initializer_, type_...) \
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID_IMPL (initializer_, type_)

      //! Ensure that `make_unique<type_> (initializer_);` can be
      //! serialized and deserialized and operator== on those values
      //! returns true. Repeats for both, `binary_xarchive` and
      //! `text_xarchive`. Does not check serialization without
      //! pointer.
#define GSPC_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID(initializer_, type_...) \
  GSPC_TESTING_REQUIRE_POINTER_SERIALIZED_TO_ID_IMPL (initializer_, type_)
    }



#include <gspc/testing/require_serialized_to_id.ipp>
