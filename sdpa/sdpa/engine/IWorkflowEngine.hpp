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

enum ExecutionState
{
  ACTIVITY_FINISHED,
  ACTIVITY_FAILED,
	ACTIVITY_CANCELLED,
};

typedef std::pair<ExecutionState, result_type> execution_result_t;

typedef we::type::requirement_t requirement_t;
typedef std::list<requirement_t> requirement_list_t;

typedef we::mgmt::basic_layer IWorkflowEngine;
typedef we::mgmt::layer RealWorkflowEngine;

#endif //IWORKFLOWENGINE_HPP
