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

#pragma once

#include <we/type/Requirement.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/Transition.hpp>

#include <functional>
#include <list>
#include <string>

const std::function<double (std::string const&)>
  null_transfer_cost = [](std::string const&) {return 0.0;};

using Preferences = std::list<we::type::Preference>;

class Requirements_and_preferences
{
public:
  Requirements_and_preferences() = delete;

  Requirements_and_preferences
      ( std::list<we::type::Requirement> const& requirements
      , we::type::schedule_data const& schedule_data
      , std::function<double (std::string const&)> transfer_cost
      , double estimated_computational_cost
      , unsigned long shared_memory_amount_required
      , Preferences preferences
      );

  unsigned long numWorkers() const;
  ::boost::optional<unsigned long> maximum_number_of_retries() const;
  std::list<we::type::Requirement> const& requirements() const;
  std::function<double (std::string const&)> transfer_cost() const;
  double computational_cost() const;
  unsigned long shared_memory_amount_required() const;
  Preferences preferences() const;

private:
  std::list<we::type::Requirement> _requirements;
  we::type::schedule_data _scheduleData;
  std::function<double (std::string const&)> _transfer_cost;
  double _estimated_computational_cost;
  unsigned long _shared_memory_amount_required;
  Preferences _preferences;
};
