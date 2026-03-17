// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>




      namespace gspc::test::parallel_reduce::module_call
      {
        struct Task
        {
          unsigned long _lhs;
          unsigned long _rhs;

          Task (decltype (_lhs), decltype (_rhs));

          template<typename Archive> void serialize (Archive&, unsigned int);
          Task() = default;
        };

        std::ostream& operator<< (std::ostream&, Task const&);
        bool operator== (Task const&, Task const&);
      }




#include <test/parallel_reduce/module_call/Task.ipp>
