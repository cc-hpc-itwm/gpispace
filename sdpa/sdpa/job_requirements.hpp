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
  job_requirements_t() = default;
  job_requirements_t ( const requirement_list_t& r_list
                     , const we::type::schedule_data& schedule_data
                     , std::function<double (std::string const&)> transfer_cost
                     )
    : _requirementList (r_list)
    , _scheduleData (schedule_data)
    , _transfer_cost (transfer_cost)
  {}

  void add (const we::type::requirement_t& req) {_requirementList.push_back(req); }

  unsigned long numWorkers() const { return _scheduleData.num_worker()?_scheduleData.num_worker().get():1; }
  const requirement_list_t& getReqList() const { return _requirementList; }
  bool empty() const { return _requirementList.empty(); }
  const std::function<double (std::string const&)> transfer_cost() const {return _transfer_cost;}
private:
  requirement_list_t _requirementList;
  we::type::schedule_data _scheduleData;
  std::function<double (std::string const&)> _transfer_cost = null_transfer_cost;
};

#endif

