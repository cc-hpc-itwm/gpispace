// Copyright (C) 2013-2015,2020-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>


  namespace gspc::we::type
  {
    struct schedule_data
    {
    public:
      schedule_data() = default;
      schedule_data
        ( std::optional<unsigned long> const& num_worker
        , std::optional<unsigned long> const& maximum_number_of_retries
        );

      const std::optional<unsigned long>& num_worker() const;
      std::optional<unsigned long> const& maximum_number_of_retries() const;

    private:
      const std::optional<unsigned long> _num_worker;
      const std::optional<unsigned long> _max_num_retries;
    };
  }
