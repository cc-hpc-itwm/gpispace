// Copyright (C) 2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/requirements_and_preferences.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_set>

namespace gspc::we::type
{
  Requirements_and_preferences
    ::Requirements_and_preferences
      ( std::list<Requirement> const& requirements
      , schedule_data const& schedule_data
      , std::function<double (std::string const&)>
          transfer_cost
      , double estimated_computational_cost
      , unsigned long shared_memory_amount_required
      , Preferences preferences
      )
        : _requirements (requirements)
        , _scheduleData (schedule_data)
        , _transfer_cost (transfer_cost)
        , _estimated_computational_cost
            (estimated_computational_cost)
        , _shared_memory_amount_required
            (shared_memory_amount_required)
        , _preferences {preferences}
  {
    if ( std::unordered_set<std::string>
           ( std::begin (_preferences)
           , std::end (_preferences)
           ).size()
       != _preferences.size()
       )
    {
      throw std::runtime_error
        ("the preferences must be distinct!");
    }
  }

  unsigned long
    Requirements_and_preferences::numWorkers() const
  {
    return _scheduleData.num_worker().value_or (1);
  }

  std::optional<unsigned long>
    Requirements_and_preferences
      ::maximum_number_of_retries() const
  {
    return _scheduleData.maximum_number_of_retries();
  }

  std::list<Requirement> const&
    Requirements_and_preferences::requirements() const
  {
    return _requirements;
  }

  std::function<double (std::string const&)>
    Requirements_and_preferences::transfer_cost() const
  {
    return _transfer_cost;
  }

  double
    Requirements_and_preferences
      ::computational_cost() const
  {
    return _estimated_computational_cost;
  }

  unsigned long
    Requirements_and_preferences
      ::shared_memory_amount_required() const
  {
    return _shared_memory_amount_required;
  }

  Preferences
    Requirements_and_preferences::preferences() const
  {
    return _preferences;
  }
}
