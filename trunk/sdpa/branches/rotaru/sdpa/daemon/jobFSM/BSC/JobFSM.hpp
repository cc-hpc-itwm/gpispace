#ifndef JOB_FSM_BSC_HPP
#define JOB_FSM_BSC_HPP 1

#include <sdpa/daemon/JobImpl.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>
#include <boost/statechart/in_state_reaction.hpp>

#include <sdpa/logging.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

using namespace sdpa::daemon;

//using namespace std;

namespace sdpa { namespace fsm { namespace bsc {


// FSM states (forward declarations)
struct Pending;
struct Running;
struct Cancelling;
struct Cancelled;
struct Cancelled;
struct Failed;
struct Finished;


struct EvtBSCDispatch : sc::event< EvtBSCDispatch > {};


// The FSM
struct JobFSM : public sdpa::daemon::JobImpl, public sc::state_machine<JobFSM, Pending>
{
	JobFSM( const sdpa::job_id_t &id,
			const sdpa::job_desc_t &desc,
		    const sdpa::daemon::IComm* pHandler = NULL,
		    const sdpa::job_id_t &parent = Job::invalid_job_id())
			: JobImpl(id, desc, pHandler, parent), SDPA_INIT_LOGGER("sdpa.fsm.bsc.JobFSM")
	{
		initiate();
		SDPA_LOG_DEBUG("State machine created");
	}

	~JobFSM()  throw () {
		terminate();
		SDPA_LOG_DEBUG("State machine destroyed");
	}

	void print_states()
	{
		for( state_iterator it = state_begin(); it != state_end(); it++ )
			std::cout<<"State "<<typeid(*it).name()<<std::endl;
	}


	virtual void process_event( const boost::statechart::event_base & e) {
		 sc::state_machine<JobFSM, Pending>::process_event(e);
    }

	virtual void CancelJob(const sdpa::events::CancelJobEvent*);
	virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
	virtual void DeleteJob(const sdpa::events::DeleteJobEvent*);
	virtual void JobFailed(const sdpa::events::JobFailedEvent*);
	virtual void JobFinished(const sdpa::events::JobFinishedEvent*);
	virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*);
	virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
	virtual void Dispatch();

	virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& evt)
	{
		return JobImpl::action_query_job_status(evt);
	}

	sdpa::status_t getStatus()
	{
		sdpa::status_t status("");
		for( state_iterator it = state_begin(); it != state_end(); it++ )
		{
			status += std::string(typeid(*it).name());
		}

		return status;
	}

private:
	SDPA_DECLARE_LOGGER();
};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<EvtBSCDispatch>,
					   sc::custom_reaction< sdpa::events::CancelJobEvent>,
					   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { } //std::cout<<" enter state 'Pending'" << std::endl; }
	~Pending() { } //std::cout<<" leave state 'Pending'" << std::endl; }

	sc::result react( const EvtBSCDispatch & e);
	sc::result react( const sdpa::events::CancelJobEvent & e);
	//sc::result react( const sdpa::events::QueryJobStatusEvent & e);
	sc::result react( const sc::exception_thrown & e);
};

struct Running : sc::simple_state<Running, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
                   sc::custom_reaction<sdpa::events::CancelJobEvent>,
                   //sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Running() { } //std::cout<< " enter state 'Running'" << std::endl; }
	~Running() { } //std::cout<< " leave state 'Running'" << std::endl; }

	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobEvent& );
    //sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

// superstate wit 2 states Cncelling and Cancelled
struct Cancel : sc::simple_state<Cancel, JobFSM, Cancelling>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancel() { } //std::cout<< " enter state 'Cancelled'" << std::endl; }
	~Cancel() { } //std::cout<< " leave state 'Cancelled'" << std::endl; }

	//void action_query_status(const sdpa::events::QueryJobStatusEvent& e);
	void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Cancelling : sc::simple_state<Cancelling, Cancel>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
				   sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
				   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Cancelling() { } //std::cout<<" enter state 'Cancelling'" << std::endl; }
	~Cancelling() { } //std::cout<< " leave state 'Cancelling'" << std::endl; }

    sc::result react( const sdpa::events::CancelJobAckEvent& );
	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, Cancel>
{
typedef mpl::list< 	sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
                    sc::custom_reaction<sc::exception_thrown> > reactions;

	Cancelled() { } //std::cout<< " enter state 'Cancelled'" << std::endl; }
	~Cancelled() { } //std::cout<< " leave state 'Cancelled'" << std::endl; }

    //sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< 	sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction<sdpa::events::RetrieveJobResultsEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { } //std::cout<< " enter state 'Failed'" << std::endl; }
	~Failed() { } //std::cout<< " leave state 'Failed'" << std::endl; }

	//sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sdpa::events::RetrieveJobResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list<  sc::in_state_reaction< sdpa::events::QueryJobStatusEvent, JobFSM, &JobFSM::action_query_job_status >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction<sdpa::events::RetrieveJobResultsEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { } //std::cout<< " enter state 'Finished'" << std::endl; }
	~Finished() { } //std::cout<< " leave state 'Finished'" << std::endl; }

	//sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sdpa::events::RetrieveJobResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

}}}

#endif // JobFSM_H
