// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

namespace we
{
  namespace type
  {
    struct schedule_data
    {
    public:
      schedule_data() = default;
      schedule_data
        ( ::boost::optional<unsigned long> const& num_worker
        , ::boost::optional<unsigned long> const& maximum_number_of_retries
        );

      const ::boost::optional<unsigned long>& num_worker() const;
      ::boost::optional<unsigned long> const& maximum_number_of_retries() const;

    private:
      const ::boost::optional<unsigned long> _num_worker;
      const ::boost::optional<unsigned long> _max_num_retries;
    };
  }
}
