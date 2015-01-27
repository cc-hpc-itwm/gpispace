// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_JOB_REQUIREMENTS_HPP
#define SDPA_JOB_REQUIREMENTS_HPP

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

typedef std::list<we::type::requirement_t> requirement_list_t;
const std::function<double (std::string const&)>
  null_transfer_cost = [](const std::string&) {return 0.0;};

class job_requirements_t
{
public:
  job_requirements_t() = delete;

  job_requirements_t ( const requirement_list_t& r_list
                     , const we::type::schedule_data& schedule_data
                     , std::function<double (std::string const&)> transfer_cost
                     , double estimated_computational_cost
                     )
    : _requirementList (r_list)
    , _scheduleData (schedule_data)
    , _transfer_cost (transfer_cost)
    , _estimated_computational_cost (estimated_computational_cost)
  {}

  unsigned long numWorkers() const {return _scheduleData.num_worker().get_value_or(1);}
  const requirement_list_t& getReqList() const {return _requirementList;}
  const std::function<double (std::string const&)> transfer_cost() const {return _transfer_cost;}
  double computational_cost() const {return _estimated_computational_cost;}
private:
  requirement_list_t _requirementList;
  we::type::schedule_data _scheduleData;
  std::function<double (std::string const&)> _transfer_cost;
  double _estimated_computational_cost;
};

#endif

