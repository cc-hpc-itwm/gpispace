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

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30 //or whatever you need
#define BOOST_MPL_LIMIT_MAP_SIZE 30 //or whatever you need

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_MSM
#   include <sdpa/daemon/jobFSM/BMSM/JobFSM.hpp>
namespace jsm = sdpa::fsm::bmsm;
#elif USE_BOOST_SC
#  include <boost/statechart/state_machine.hpp>
#  include <boost/statechart/simple_state.hpp>
#  include <boost/statechart/custom_reaction.hpp>
#  include <boost/statechart/transition.hpp>
#  include <boost/statechart/exception_translator.hpp>

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
