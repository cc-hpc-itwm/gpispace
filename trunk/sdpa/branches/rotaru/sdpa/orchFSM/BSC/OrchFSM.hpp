#ifndef OrchFSM_H
#define OrchFSM_H

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/ConfigNokEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/InterruptEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/SubmitAckEvent.hpp>

//#include <seda/Strategy.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

//using namespace std;
using namespace sdpa::events;

// OrchFSM states (forward declarations)
struct Down;
struct Up;

// The FSM
struct OrchFSM : /*public seda::Strategy,*/public sc::state_machine<OrchFSM, Down>
{
	void action_configure(const StartUpEvent& e);
	void action_config_ok(const ConfigOkEvent& e);
	void action_config_nok(const ConfigNokEvent& e);

	void action_interrupt(const InterruptEvent& e);
	void action_lifesign(const LifeSignEvent& e);
	void action_delete_job(const DeleteJobEvent& e);
	void action_request_job(const RequestJobEvent& e);
	void action_submit_ack(const SubmitAckEvent& e);
	void action_config_request(const ConfigRequestEvent& e);

	void printStates();


private:
	// Scheduler::ptr_t scheduler;
	// something like typedef std::map<string, Job> with synchronization
	// JobMap job_map; // public accessor functions for the scheduler
};

struct Down : sc::simple_state<Down, OrchFSM>
{
	typedef mpl::list< sc::custom_reaction<StartUpEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Down() { std::cout <<" enter state 'Down'" << std::endl; }
	~Down() { std::cout <<" leave state 'Down'" << std::endl; }

	sc::result react( const StartUpEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Configurring  : sc::simple_state<Configurring, OrchFSM>
{
	typedef mpl::list< sc::custom_reaction<ConfigOkEvent>,
					   sc::custom_reaction<ConfigNokEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Configurring() { std::cout <<" enter state 'Configurring'" << std::endl; }
	~Configurring() { std::cout <<" leave state 'Configurring'" << std::endl; }

	sc::result react( const ConfigOkEvent& );
	sc::result react( const ConfigNokEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Up : sc::simple_state<Up, OrchFSM>
{
typedef mpl::list< sc::custom_reaction<InterruptEvent>,
                   sc::custom_reaction<LifeSignEvent>,
                   sc::custom_reaction<DeleteJobEvent>,
                   sc::custom_reaction<RequestJobEvent>,
                   sc::custom_reaction<SubmitAckEvent>,
                   sc::custom_reaction<ConfigRequestEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Up() { std::cout << " enter state 'Up'" << std::endl; }
	~Up() { std::cout << " leave state 'Up'" << std::endl; }

	sc::result react( const InterruptEvent& );
	sc::result react( const LifeSignEvent& );
  	sc::result react( const DeleteJobEvent& );
    sc::result react( const RequestJobEvent& );
    sc::result react( const SubmitAckEvent& );
    sc::result react( const ConfigRequestEvent& );
    sc::result react( const sc::exception_thrown & );
};

#endif // OrchFSM_H
