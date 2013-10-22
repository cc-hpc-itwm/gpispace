// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef IWORKFLOWENGINE_HPP
#define IWORKFLOWENGINE_HPP

#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/layer.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

typedef we::mgmt::basic_layer::id_type id_type;
typedef we::mgmt::basic_layer::result_type result_type;
typedef we::mgmt::basic_layer::reason_type reason_type;
typedef we::mgmt::basic_layer::encoded_type encoded_type;

typedef we::type::requirement_t requirement_t;
typedef std::list<requirement_t> requirement_list_t;
typedef we::type::schedule_data schedule_data;

struct job_requirements_t
{
	typedef requirement_list_t::const_iterator const_iterator;

	job_requirements_t(const requirement_list_t& r_list, const schedule_data& schedule_data) :
		m_requirementList(r_list), m_scheduleData(schedule_data)
	{
	}

	void add(const requirement_t& req) {m_requirementList.push_back(req); }

	long numWorkers() { return m_scheduleData.num_worker()?m_scheduleData.num_worker().get():1; }
	const requirement_list_t& getReqList() const { return m_requirementList; }
private:
	requirement_list_t m_requirementList;
	schedule_data m_scheduleData;
};

#endif //IWORKFLOWENGINE_HPP
