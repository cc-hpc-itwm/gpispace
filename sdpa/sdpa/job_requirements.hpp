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
  job_requirements_t(const requirement_list_t& r_list, const we::type::schedule_data& schedule_data)
    : m_requirementList(r_list), m_scheduleData(schedule_data)
  {}

  void add(const we::type::requirement_t& req) {m_requirementList.push_back(req); }

  unsigned long numWorkers() const { return m_scheduleData.num_worker()?m_scheduleData.num_worker().get():1; }
  const requirement_list_t& getReqList() const { return m_requirementList; }
  bool empty() const { return m_requirementList.empty(); }

private:
  requirement_list_t m_requirementList;
  we::type::schedule_data m_scheduleData;
};

#endif

