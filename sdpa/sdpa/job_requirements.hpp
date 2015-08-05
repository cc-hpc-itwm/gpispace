// tiberiu.rotaru@itwm.fraunhofer.de

#pragma once

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

const std::function<double (std::string const&)>
  null_transfer_cost = [](const std::string&) {return 0.0;};

class job_requirements_t
{
public:
  job_requirements_t() = delete;

  job_requirements_t ( const std::list<we::type::requirement_t>& r_list
                     , const we::type::schedule_data& schedule_data
                     , std::function<double (std::string const&)> transfer_cost
                     , double estimated_computational_cost
                     , unsigned long shared_memory_amount_required
                     )
    : _requirementList (r_list)
    , _scheduleData (schedule_data)
    , _transfer_cost (transfer_cost)
    , _estimated_computational_cost (estimated_computational_cost)
    , _shared_memory_amount_required (shared_memory_amount_required)
  {}

  unsigned long numWorkers() const {return _scheduleData.num_worker().get_value_or(1);}
  const std::list<we::type::requirement_t>& getReqList() const {return _requirementList;}
  const std::function<double (std::string const&)> transfer_cost() const {return _transfer_cost;}
  double computational_cost() const {return _estimated_computational_cost;}
  unsigned long shared_memory_amount_required() const {return _shared_memory_amount_required;}
private:
  std::list<we::type::requirement_t> _requirementList;
  we::type::schedule_data _scheduleData;
  std::function<double (std::string const&)> _transfer_cost;
  double _estimated_computational_cost;
  unsigned long _shared_memory_amount_required;
};
