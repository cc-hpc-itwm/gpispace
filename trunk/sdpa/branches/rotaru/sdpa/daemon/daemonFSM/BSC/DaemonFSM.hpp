/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.hpp
 *
 *    Description:  Daemon state chart (boost)
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
#ifndef DAEMON_FSM_BSC_HPP
#define DAEMON_FSM_BSC_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/memory.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

namespace sdpa { namespace fsm { namespace bsc {

// OrchFSM states (forward declarations)
struct Down;
struct Up;

// The FSM
struct DaemonFSM : public sdpa::daemon::GenericDaemon, public sc::state_machine<DaemonFSM, Down>
{
	typedef  sdpa::shared_ptr<DaemonFSM> ptr_t;
	typedef boost::recursive_mutex mutex_type;
	typedef boost::unique_lock<mutex_type> lock_type;

	DaemonFSM(	const std::string &name,
				seda::Stage* ptrToMasterStage,
				seda::Stage* ptrToSlaveStage,
				sdpa::Sdpa2Gwes*  pArgSdpa2Gwes);

	DaemonFSM(  const std::string &name,
				sdpa::Sdpa2Gwes*  pArgSdpa2Gwes,
				const std::string& toMasterStageName,
				const std::string& toSlaveStageName = std::string(""));

	DaemonFSM( const std::string &name = "", sdpa::Sdpa2Gwes*  pArgSdpa2Gwes = NULL);

	virtual ~DaemonFSM();

	virtual void handleDaemonEvent(const seda::IEvent::Ptr& pEvent);

	virtual void process_event( const boost::statechart::event_base & e) {
		 sc::state_machine<DaemonFSM, Down>::process_event(e);
	}

	template <class Archive>
	void serialize(Archive& ar, const unsigned int file_version )
	{
		ar & boost::serialization::base_object<GenericDaemon>(*this);
	}

	friend class boost::serialization::access;
	friend class sdpa::tests::WorkerSerializationTest;

	void print_states();
private:
	SDPA_DECLARE_LOGGER();
	mutex_type mtx_;
};

struct Down : sc::simple_state<Down, DaemonFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::StartUpEvent>,
					   sc::custom_reaction<sdpa::events::ErrorEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Down() : SDPA_INIT_LOGGER("sdpa.fsm.bsc.Down") { }
	~Down() { }

	sc::result react( const sdpa::events::StartUpEvent& );
	sc::result react( const sdpa::events::ErrorEvent& );
	sc::result react( const sc::exception_thrown & e);
	SDPA_DECLARE_LOGGER();
};

struct Configuring  : sc::simple_state<Configuring, DaemonFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::ConfigOkEvent>,
					   sc::custom_reaction<sdpa::events::ConfigNokEvent>,
					   sc::custom_reaction<sdpa::events::ErrorEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Configuring():  SDPA_INIT_LOGGER("sdpa.fsm.bsc.Configuring") { }
	~Configuring() { }

	sc::result react( const sdpa::events::ConfigOkEvent& );
	sc::result react( const sdpa::events::ConfigNokEvent& );
	sc::result react( const sdpa::events::ErrorEvent& );
	sc::result react( const sc::exception_thrown & e);
	SDPA_DECLARE_LOGGER();
};

struct Up : sc::simple_state<Up, DaemonFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::InterruptEvent>,
                   sc::custom_reaction<sdpa::events::LifeSignEvent>,
                   sc::custom_reaction<sdpa::events::DeleteJobEvent>,
                   sc::custom_reaction<sdpa::events::RequestJobEvent>,
                   sc::custom_reaction<sdpa::events::SubmitJobEvent>,
                   sc::custom_reaction<sdpa::events::ConfigRequestEvent>,
                   sc::custom_reaction<sdpa::events::WorkerRegistrationEvent>,
                   sc::custom_reaction<sdpa::events::ErrorEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Up() :  SDPA_INIT_LOGGER("sdpa.fsm.bsc.Up") { }
	~Up() { }

	sc::result react( const sdpa::events::InterruptEvent& );
	sc::result react( const sdpa::events::LifeSignEvent& );
  	sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sdpa::events::RequestJobEvent& );
    sc::result react( const sdpa::events::SubmitJobEvent& );
    sc::result react( const sdpa::events::ConfigRequestEvent& );
    sc::result react( const sdpa::events::WorkerRegistrationEvent& );
    sc::result react( const sdpa::events::ErrorEvent& );
    sc::result react( const sc::exception_thrown& );
    SDPA_DECLARE_LOGGER();
};

}}}

#endif // DaemonFSM_H
