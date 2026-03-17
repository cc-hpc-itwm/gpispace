// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later


#include <gspc/util/serialization/std/optional.hpp>

  namespace gspc::logging
  {
    template<typename Archive>
      void serialize (Archive& ar, endpoint& ep, unsigned int const)
    {
      ar & ep.as_tcp;
      ar & ep.as_socket;
    }
  }
