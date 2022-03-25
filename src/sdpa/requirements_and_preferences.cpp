// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/requirements_and_preferences.hpp>
#include <unordered_set>

Requirements_and_preferences::Requirements_and_preferences
  ( std::list<we::type::Requirement> const& requirements
  , we::type::schedule_data const& schedule_data
  , std::function<double (std::string const&)> transfer_cost
  , double estimated_computational_cost
  , unsigned long shared_memory_amount_required
  , Preferences preferences
  )
    : _requirements (requirements)
    , _scheduleData (schedule_data)
    , _transfer_cost (transfer_cost)
    , _estimated_computational_cost (estimated_computational_cost)
    , _shared_memory_amount_required (shared_memory_amount_required)
    , _preferences {preferences}
{
    if ( std::unordered_set<std::string>
           (std::begin (_preferences), std::end (_preferences)).size()
       != _preferences.size()
       )
    {
      throw std::runtime_error ("the preferences must be distinct!");
    }
  }

unsigned long Requirements_and_preferences::numWorkers() const
{
  return _scheduleData.num_worker().get_value_or (1);
}

::boost::optional<unsigned long>
  Requirements_and_preferences::maximum_number_of_retries() const
{
  return _scheduleData.maximum_number_of_retries();
}

std::list<we::type::Requirement> const&
  Requirements_and_preferences::requirements() const
{
  return _requirements;
}

std::function<double (std::string const&)>
  Requirements_and_preferences::transfer_cost() const
{
  return _transfer_cost;
}

double Requirements_and_preferences::computational_cost() const
{
  return _estimated_computational_cost;
}

unsigned long Requirements_and_preferences::shared_memory_amount_required() const
{
  return _shared_memory_amount_required;
}

Preferences Requirements_and_preferences::preferences() const
{
  return _preferences;
}
