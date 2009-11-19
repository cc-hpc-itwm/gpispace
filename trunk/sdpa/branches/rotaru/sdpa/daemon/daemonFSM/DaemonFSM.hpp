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

#ifdef USE_BOOST_SC
#   include "sdpa/daemon/daemonFSM/BSC/DaemonFSM.hpp"
namespace dsm = sdpa::fsm::bsc;
#elif USE_SMC
#   include "sdpa/daemon/daemonFSM/SMC/DaemonFSM.hpp"
namespace dsm = sdpa::fsm::smc;
#else
#   error "No state machine variant defined!"
#endif

using namespace dsm;

#endif /* DAEMONFSM_HPP_ */
