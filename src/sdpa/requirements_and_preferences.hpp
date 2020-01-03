#pragma once

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/transition.hpp>

#include <functional>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

const std::function<double (std::string const&)>
  null_transfer_cost = [](const std::string&) {return 0.0;};

using Preferences = std::list<we::type::preference_t>;

class Requirements_and_preferences
{
public:
  Requirements_and_preferences() = delete;

  Requirements_and_preferences
      ( const std::list<we::type::requirement_t>& requirements
      , const we::type::schedule_data& schedule_data
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
  {
    if ( std::unordered_set<std::string>
           (std::begin (preferences), std::end (preferences)).size()
       != preferences.size()
       )
    {
      throw std::runtime_error ("the preferences must be distinct!");
    }

    _preferences.swap (preferences);
  }

  unsigned long numWorkers() const {return _scheduleData.num_worker().get_value_or(1);}
  const std::list<we::type::requirement_t>& requirements() const {return _requirements;}
  const std::function<double (std::string const&)> transfer_cost() const {return _transfer_cost;}
  double computational_cost() const {return _estimated_computational_cost;}
  unsigned long shared_memory_amount_required() const {return _shared_memory_amount_required;}
  Preferences preferences() const { return _preferences; }
private:
  std::list<we::type::requirement_t> _requirements;
  we::type::schedule_data _scheduleData;
  std::function<double (std::string const&)> _transfer_cost;
  double _estimated_computational_cost;
  unsigned long _shared_memory_amount_required;
  Preferences _preferences;
};
