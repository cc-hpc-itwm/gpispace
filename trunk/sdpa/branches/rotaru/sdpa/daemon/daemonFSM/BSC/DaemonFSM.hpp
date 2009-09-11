#ifndef DAEMON_FSM_BSC_HPP
#define DAEMON_FSM_BSC_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

namespace sdpa { namespace fsm { namespace bsc {

// OrchFSM states (forward declarations)
struct Down;
struct Up;

// The FSM
struct DaemonFSM : public sdpa::daemon::GenericDaemon, public sc::state_machine<DaemonFSM, Down>
{
	DaemonFSM(const std::string &name, const std::string &outputStage, sdpa::wf::Sdpa2Gwes* ptrGwes = NULL)
		: SDPA_INIT_LOGGER("sdpa.fsm.bsc.DaemonFSM"),
		GenericDaemon(name, outputStage, ptrGwes)
		{

		};

	void print_states();
private:
	SDPA_DECLARE_LOGGER();
};

struct Down : sc::simple_state<Down, DaemonFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::StartUpEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Down() { } //std::cout <<" enter state 'Down'" << std::endl; }
	~Down() { } //std::cout <<" leave state 'Down'" << std::endl; }

	sc::result react( const sdpa::events::StartUpEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Configurring  : sc::simple_state<Configurring, DaemonFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::ConfigOkEvent>,
					   sc::custom_reaction<sdpa::events::ConfigNokEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Configurring() { } //std::cout <<" enter state 'Configuring'" << std::endl; }
	~Configurring() { } //std::cout <<" leave state 'Configuring'" << std::endl; }

	sc::result react( const sdpa::events::ConfigOkEvent& );
	sc::result react( const sdpa::events::ConfigNokEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Up : sc::simple_state<Up, DaemonFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::InterruptEvent>,
                   sc::custom_reaction<sdpa::events::LifeSignEvent>,
                   sc::custom_reaction<sdpa::events::DeleteJobEvent>,
                   sc::custom_reaction<sdpa::events::RequestJobEvent>,
                   sc::custom_reaction<sdpa::events::SubmitJobEvent>,
                   sc::custom_reaction<sdpa::events::SubmitJobAckEvent>,
                   sc::custom_reaction<sdpa::events::ConfigRequestEvent>,
                   sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
                   sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
                   sc::custom_reaction<sdpa::events::WorkerRegistrationEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Up() { } //std::cout << " enter state 'Up'" << std::endl; }
	~Up() { } //std::cout << " leave state 'Up'" << std::endl; }

	sc::result react( const sdpa::events::InterruptEvent& );
	sc::result react( const sdpa::events::LifeSignEvent& );
  	sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sdpa::events::RequestJobEvent& );
    sc::result react( const sdpa::events::SubmitJobEvent& );
    sc::result react( const sdpa::events::SubmitJobAckEvent& );
    sc::result react( const sdpa::events::ConfigRequestEvent& );
    sc::result react( const sdpa::events::JobFinishedEvent& );
    sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobAckEvent& );
    sc::result react( const sdpa::events::WorkerRegistrationEvent& );
    sc::result react( const sc::exception_thrown& );
};

}}}

#endif // DaemonFSM_H
