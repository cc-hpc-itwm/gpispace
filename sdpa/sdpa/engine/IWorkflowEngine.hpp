/*
 * =====================================================================================
 *
 *       Filename:  IWorkflowEngine.hpp
 *
 *    Description:  Redefines the interface to gwes
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef IWORKFLOWENGINE_HPP
#define IWORKFLOWENGINE_HPP 1

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/sdpa-config.hpp>

#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/type/requirement.hpp>
#include <we/mgmt/layer.hpp>
#include <we/loader/putget.hpp>

typedef we::mgmt::basic_layer::id_type id_type;
typedef we::mgmt::basic_layer::result_type result_type;
typedef we::mgmt::basic_layer::reason_type reason_type;
typedef we::mgmt::basic_layer::encoded_type encoded_type;

enum ExecutionState
 {
	  ACTIVITY_FINISHED
	, ACTIVITY_FAILED
	, ACTIVITY_CANCELLED
 };

typedef std::pair<ExecutionState, result_type> execution_result_t;

typedef we::type::requirement_t requirement_t;
typedef std::list<requirement_t> requirement_list_t;
typedef we::mgmt::basic_layer IWorkflowEngine;

#endif //IWORKFLOWENGINE_HPP
