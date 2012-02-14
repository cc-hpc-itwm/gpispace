/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.hpp
 *
 *    Description:  Daemon state machine/chart
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
#ifndef DAEMONFSM_HPP_
#define DAEMONFSM_HPP_

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_MSM
#  include "sdpa/daemon/daemonFSM/BMSM/DaemonFSM.hpp"
namespace dsm = sdpa::fsm::bmsm;
#elif USE_BOOST_SC
#  include <boost/statechart/state_machine.hpp>
#  include <boost/statechart/simple_state.hpp>
#  include <boost/statechart/custom_reaction.hpp>
#  include <boost/statechart/transition.hpp>
#  include <boost/statechart/exception_translator.hpp>
#  include "sdpa/daemon/daemonFSM/BSC/DaemonFSM.hpp"
namespace dsm = sdpa::fsm::bsc;
#elif USE_SMC_SC
#   include "sdpa/daemon/daemonFSM/SMC/DaemonFSM.hpp"
namespace dsm = sdpa::fsm::smc;
#else
#   error "No state machine variant defined!"
#endif

using namespace dsm;

#endif /* DAEMONFSM_HPP_ */
