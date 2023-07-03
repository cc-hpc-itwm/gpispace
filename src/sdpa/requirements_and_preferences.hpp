// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/Requirement.hpp>
#include <we/type/Transition.hpp>
#include <we/type/schedule_data.hpp>

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
