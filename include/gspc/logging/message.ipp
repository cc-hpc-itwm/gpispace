// Copyright (C) 2018,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/serialization/std/chrono.hpp>


  namespace gspc::logging
  {
    template<typename Archive>
      void message::serialize (Archive& ar, unsigned int)
    {
      ar & _content;
      ar & _category;
      ar & _timestamp;
      ar & _hostname;
      ar & _process_id;
      ar & _thread_id;
    }
  }
