// Copyright (C) 2011,2013-2015,2019,2021-2023,2025-2026
//               Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Requirement.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/schedule_data.hpp>

#include <functional>
#include <list>
#include <string>

namespace gspc::we::type
{
  inline std::function<double (std::string const&)> const
    null_transfer_cost
      = [] (std::string const&) { return 0.0; };

  using Preferences = std::list<Preference>;

  class Requirements_and_preferences
  {
  public:
    Requirements_and_preferences() = delete;

    Requirements_and_preferences
      ( std::list<Requirement> const& requirements
      , schedule_data const&
      , std::function<double (std::string const&)>
          transfer_cost
      , double estimated_computational_cost
      , unsigned long shared_memory_amount_required
      , Preferences
      );

    [[nodiscard]] unsigned long numWorkers() const;
    [[nodiscard]] std::optional<unsigned long>
      maximum_number_of_retries() const;
    [[nodiscard]] std::list<Requirement> const&
      requirements() const;
    [[nodiscard]]
      std::function<double (std::string const&)>
        transfer_cost() const;
    [[nodiscard]] double computational_cost() const;
    [[nodiscard]] unsigned long
      shared_memory_amount_required() const;
    [[nodiscard]] Preferences preferences() const;

  private:
    std::list<Requirement> _requirements;
    schedule_data _scheduleData;
    std::function<double (std::string const&)>
      _transfer_cost;
    double _estimated_computational_cost;
    unsigned long _shared_memory_amount_required;
    Preferences _preferences;
  };
}
