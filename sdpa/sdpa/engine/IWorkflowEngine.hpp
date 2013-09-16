// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef IWORKFLOWENGINE_HPP
#define IWORKFLOWENGINE_HPP

#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/layer.hpp>
#include <we/type/requirement.hpp>

typedef we::mgmt::basic_layer::id_type id_type;
typedef we::mgmt::basic_layer::result_type result_type;
typedef we::mgmt::basic_layer::reason_type reason_type;
typedef we::mgmt::basic_layer::encoded_type encoded_type;

typedef we::type::requirement_t requirement_t;
typedef std::list<requirement_t> requirements_list_t;

struct job_requirements_t
{
	typedef requirements_list_t::const_iterator const_iterator;

	job_requirements_t() :  m_nInstances_(1) {};
	job_requirements_t(const requirements_list_t& r_list, int m=1) :
		m_requirementsList(r_list), m_nInstances_(m)
	{
	}

	void add(const requirement_t& req) {m_requirementsList.push_back(req); }

	size_t nInstances() { return m_nInstances_; }
	void setNumberOfInstances(size_t n) { m_nInstances_ = n; }

	const requirements_list_t& getReqList() const { return m_requirementsList; }
private:
	requirements_list_t m_requirementsList;
	size_t m_nInstances_;
};

typedef we::mgmt::basic_layer IWorkflowEngine;
typedef we::mgmt::layer RealWorkflowEngine;

#endif //IWORKFLOWENGINE_HPP
