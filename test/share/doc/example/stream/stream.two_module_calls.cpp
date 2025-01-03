// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <doc/example/stream/test.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (share_example_stream_two_module_calls)
{
  share_example_stream_test::run
    ( "stream.two_module_calls"
    , [] (unsigned long size_slot)
    {
      std::ostringstream topology_description;

      topology_description << "process:2," << size_slot << " mark_free:2,1";

      return topology_description.str();
    }
    , std::chrono::milliseconds (25)
    , 14.0
    );
}
