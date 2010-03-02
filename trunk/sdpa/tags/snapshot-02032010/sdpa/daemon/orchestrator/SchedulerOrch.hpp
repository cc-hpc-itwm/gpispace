/*
 * =====================================================================================
 *
 *       Filename:  SchedulerOrch.hpp
 *
 *    Description:  The orchestrator's scheduler
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

#ifndef SDPA_SchedulerOrch_HPP
#define SDPA_SchedulerOrch_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
	namespace daemon {
  class SchedulerOrch : public SchedulerImpl {

  public:
	 SchedulerOrch(sdpa::daemon::IComm* pCommHandler):
		 SchedulerImpl(pCommHandler),
		 SDPA_INIT_LOGGER("Scheduler " + pCommHandler->name())
	{

	}

	 virtual ~SchedulerOrch() {};

	 bool post_request( bool ) { return false; }
	 void send_life_sign() {}
	 void check_post_request() {}


  private:
	  SDPA_DECLARE_LOGGER();
  };
}}

#endif
