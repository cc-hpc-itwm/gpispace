// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later




      namespace gspc::test::parallel_reduce::module_call
      {
        template<typename Archive>
          void Task::serialize (Archive& ar, unsigned int)
        {
          ar & _lhs;
          ar & _rhs;
        }
      }
