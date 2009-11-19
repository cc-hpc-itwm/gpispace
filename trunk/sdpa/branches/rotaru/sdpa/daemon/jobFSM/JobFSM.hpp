/*
 * =====================================================================================
 *
 *       Filename:  JobFSM.hpp
 *
 *    Description:  Job state machine/chart
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
#ifndef JOBFSM_HPP_
#define JOBFSM_HPP_

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <sdpa/daemon/jobFSM/BSC/JobFSM.hpp>
namespace jsm = sdpa::fsm::bsc;
#elif USE_SMC_SC
#   include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
namespace jsm = sdpa::fsm::smc;
#else
#   error "No state machine variant defined!"
#endif

using namespace jsm;

#endif /* JOBFSM_HPP_ */
