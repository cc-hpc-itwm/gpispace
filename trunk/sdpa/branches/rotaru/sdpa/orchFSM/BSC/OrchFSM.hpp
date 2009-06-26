#ifndef ORCHFSMBSC_HPP
#define ORCHFSMBSC_HPP 1

#include <sdpa/orchFSM/OrchFSMInterface.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>


namespace mpl = boost::mpl;
namespace sc = boost::statechart;


// OrchFSM states (forward declarations)
struct Down;
struct Up;

// The FSM
struct OrchFSM : public sdpa::fsm::OrchFSMInterface, public sc::state_machine<OrchFSM, Down>
{
	void printStates();
};

struct Down : sc::simple_state<Down, OrchFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::StartUpEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Down() { std::cout <<" enter state 'Down'" << std::endl; }
	~Down() { std::cout <<" leave state 'Down'" << std::endl; }

	sc::result react( const sdpa::events::StartUpEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Configurring  : sc::simple_state<Configurring, OrchFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::ConfigOkEvent>,
					   sc::custom_reaction<sdpa::events::ConfigNokEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Configurring() { std::cout <<" enter state 'Configurring'" << std::endl; }
	~Configurring() { std::cout <<" leave state 'Configurring'" << std::endl; }

	sc::result react( const sdpa::events::ConfigOkEvent& );
	sc::result react( const sdpa::events::ConfigNokEvent& );
	sc::result react( const sc::exception_thrown & e);
};

struct Up : sc::simple_state<Up, OrchFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::InterruptEvent>,
                   sc::custom_reaction<sdpa::events::LifeSignEvent>,
                   sc::custom_reaction<sdpa::events::DeleteJobEvent>,
                   sc::custom_reaction<sdpa::events::RequestJobEvent>,
                   sc::custom_reaction<sdpa::events::SubmitJobEvent>,
                   sc::custom_reaction<sdpa::events::SubmitJobAckEvent>,
                   sc::custom_reaction<sdpa::events::ConfigRequestEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Up() { std::cout << " enter state 'Up'" << std::endl; }
	~Up() { std::cout << " leave state 'Up'" << std::endl; }

	sc::result react( const sdpa::events::InterruptEvent& );
	sc::result react( const sdpa::events::LifeSignEvent& );
  	sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sdpa::events::RequestJobEvent& );
    sc::result react( const sdpa::events::SubmitJobEvent& );
    sc::result react( const sdpa::events::SubmitJobAckEvent& );
    sc::result react( const sdpa::events::ConfigRequestEvent& );
    sc::result react( const sc::exception_thrown & );
};

#endif // OrchFSM_H
