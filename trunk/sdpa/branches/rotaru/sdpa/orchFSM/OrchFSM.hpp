#ifndef OrchFSM_H
#define OrchFSM_H

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <MgmtEvent.hpp>
#include <StartUpEvent.hpp>
#include <InterruptEvent.hpp>
#include <LifeSignEvent.hpp>
#include <DeleteJobEvent.hpp>
#include <RequestJobEvent.hpp>
#include <SubmitAckEvent.hpp>
#include <ConfigRequestEvent.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

using namespace std;

// OrchFSM states (forward declarations)
struct Down;
struct Up;

// The FSM
struct OrchFSM : sc::state_machine<OrchFSM, Down>
{
	void action_startup_ok(const StartUpEvent& e);
	void action_startup_nok(const StartUpEvent& e);

	void action_interrupt(const InterruptEvent& e);
	void action_lifesign(const LifeSignEvent& e);
	void action_delete_job(const DeleteJobEvent& e);
	void action_request_job(const RequestJobEvent& e);
	void action_submit_ack(const SubmitAckEvent& e);
	void action_config_request(const ConfigRequestEvent& e);

	void printStates();

private:
};

struct Down : sc::simple_state<Down, OrchFSM>
{
	typedef mpl::list< sc::custom_reaction<StartUpEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Down() { cout <<" enter state 'Down'" << endl; }
	~Down() { cout <<" leave state 'Down'" << endl; }

	sc::result react( const StartUpEvent& );
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

	Up() { cout << " enter state 'Up'" << endl; }
	~Up() { cout << " leave state 'Up'" << endl; }

	sc::result react( const InterruptEvent& );
	sc::result react( const LifeSignEvent& );
  	sc::result react( const DeleteJobEvent& );
    sc::result react( const RequestJobEvent& );
    sc::result react( const SubmitAckEvent& );
    sc::result react( const ConfigRequestEvent& );
    sc::result react( const sc::exception_thrown & );
};

#endif // OrchFSM_H
