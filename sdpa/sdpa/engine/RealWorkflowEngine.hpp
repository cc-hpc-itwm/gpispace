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
#ifndef REAL_WORKFLOW_ENGINE_HPP
#define REAL_WORKFLOW_ENGINE_HPP 1

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>

#ifdef USE_REAL_WE
	typedef we::mgmt::layer RealWorkflowEngine;
#else
	typedef EmptyWorkflowEngine RealWorkflowEngine;
#endif

#endif //REAL_WORKFLOW_ENGINE_HPP
