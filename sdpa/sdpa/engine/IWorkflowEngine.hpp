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

	job_requirements_t() :  n_workers(1) {};
	job_requirements_t(const requirements_list_t& r_list, int m=1) :
		req_list(r_list), n_workers(m)
	{
	}

	void add(const requirement_t& req) {req_list.push_back(req); }

	requirements_list_t req_list;
	int n_workers;
};

typedef we::mgmt::basic_layer IWorkflowEngine;
typedef we::mgmt::layer RealWorkflowEngine;

#endif //IWORKFLOWENGINE_HPP
