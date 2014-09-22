// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_JOB_REQUIREMENTS_HPP
#define SDPA_JOB_REQUIREMENTS_HPP

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

typedef std::list<we::type::requirement_t> requirement_list_t;

class job_requirements_t
{
public:
  job_requirements_t() = default;
  job_requirements_t ( const requirement_list_t& r_list
                     , const we::type::schedule_data& schedule_data
                     )
    : _requirementList (r_list)
    , _scheduleData (schedule_data)
  {}

  void add (const we::type::requirement_t& req) {_requirementList.push_back(req); }

  unsigned long numWorkers() const { return _scheduleData.num_worker()?_scheduleData.num_worker().get():1; }
  const requirement_list_t& getReqList() const { return _requirementList; }
  bool empty() const { return _requirementList.empty(); }

private:
  requirement_list_t _requirementList;
  we::type::schedule_data _scheduleData;
};

#endif

