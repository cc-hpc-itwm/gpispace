/*
 * =====================================================================================
 *
 *       Filename:  JobFSM.hpp
 *
 *    Description:  Job state chart (boost)
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
#include <boost/serialization/access.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

using namespace sdpa::daemon;

//using namespace std;

char const* const state_names[] = { 	"SDPA::Pending"
				  	  	  	  	  	  , "SDPA::Running"
				  	  	  	  	  	  , "SDPA::Finished"
				  	  	  	  	  	  , "SDPA::Failed"
				  	  	  	  	  	  , "SDPA::Cancelling"
				  	  	  	  	  	  , "SDPA::Cancelled"
                                  };

namespace sdpa { namespace fsm { namespace bsc {


// FSM states (forward declarations)
struct Pending;
struct Running;
struct Cancelling;
struct Cancelled;
struct Cancel;
struct Failed;
struct Finished;


struct EvtBSCDispatch : sc::event< EvtBSCDispatch > {};


// The FSM
struct JobFSM : public sdpa::daemon::JobImpl, public sc::state_machine<JobFSM, Pending>
{
	typedef sdpa::shared_ptr<JobFSM> Ptr;
	typedef boost::recursive_mutex mutex_type;
	typedef boost::unique_lock<mutex_type> lock_type;

	JobFSM( const sdpa::job_id_t id = JobId(""),
			const sdpa::job_desc_t desc = "",
		    const sdpa::daemon::IComm* pHandler = NULL,
		    const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
			: JobImpl(id, desc, pHandler, parent),
			  SDPA_INIT_LOGGER("sdpa.fsm.bsc.JobFSM")
	{
		SDPA_LOG_DEBUG("State machine created");
	}

	~JobFSM()  throw () {
		terminate();
		SDPA_LOG_DEBUG("State machine destroyed");
	}

	void start_fsm() { initiate(); }

	void print_states()
	{
		for( state_iterator it = state_begin(); it != state_end(); it++ )
			std::cout<<"State "<<getStatus()<<std::endl;
	}

	virtual void process_event( const boost::statechart::event_base & e) {
		 sc::state_machine<JobFSM, Pending>::process_event(e);
    }

	virtual void CancelJob(const sdpa::events::CancelJobEvent*);
	virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
	virtual void DeleteJob(const sdpa::events::DeleteJobEvent*);
	virtual void JobFailed(const sdpa::events::JobFailedEvent*);
	virtual void JobFinished(const sdpa::events::JobFinishedEvent*);
	virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IComm* );
	virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
	virtual void Dispatch();

	/*virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& evt)
	{
		return JobImpl::action_query_job_status(evt);
	}*/

	virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& evt)
	{
		return JobImpl::action_cancel_job_ack(evt);
	}

	virtual void action_retrieve_job_results( const sdpa::events::RetrieveJobResultsEvent& evt )
	{
		return JobImpl::action_retrieve_job_results(evt);
	}

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int) const
	{
		//lock_type lock(mtx_);
		//int stateId(m_fsmContext.getState().getId());

		// invoke serialization of the base class
		ar << boost::serialization::base_object<JobImpl>(*this);
		ar << stateId;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int)
	{
		lock_type lock(mtx_);
		int stateId;

		// invoke serialization of the base class
		ar >> boost::serialization::base_object<JobImpl>(*this);
		ar >> stateId;

		//LOG(TRACE, "setState("<<id()<<") to "<<m_fsmContext.valueOf(stateId).getName()<<"!!!");
		//m_fsmContext.setState(m_fsmContext.valueOf(stateId););
		//state.Entry(m_fsmContext);

	   try {
		   LOG(TRACE, "Current state is: "<<getStatus());
	   }
	   catch( const statemap::StateUndefinedException& ex )
	   {
		   LOG(TRACE, "Current state is: UNDEFINED");
	   }

	}

	template<class Archive>
	void serialize( Archive & ar, const unsigned int file_version )
	{
		boost::serialization::split_member(ar, *this, file_version);
	}

	/*sdpa::status_t getStatus()
	{
		sdpa::status_t status("");
		for( state_iterator it = state_begin(); it != state_end(); it++ )
		{
			status += std::string(typeid(*it).name());
		}

		return status;
	}*/

	sdpa::status_t getStatus()
	{
		sdpa::status_t status("");
		for( state_iterator it = state_begin(); it != state_end(); it++ )
		{
			status += std::string("SDPA:") + pLeafState->custom_dynamic_type_ptr< char >();
		}

		return status;
	}

private:
	SDPA_DECLARE_LOGGER();
	mutex_type mtx_;
};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<EvtBSCDispatch>,
					   sc::custom_reaction< sdpa::events::CancelJobEvent>,
					   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { }
	~Pending() { }

	sc::result react( const EvtBSCDispatch & e);
	sc::result react( const sdpa::events::CancelJobEvent & e);
	sc::result react( const sc::exception_thrown & e);
};

struct Running : sc::simple_state<Running, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
                   sc::custom_reaction<sdpa::events::CancelJobEvent>,
                   sc::transition<sdpa::events::CancelJobAckEvent, Cancelled>,
                   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Running() { }
	~Running() { }

	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobEvent& );
    sc::result react( const sc::exception_thrown & );
};

// superstate wit 2 states Cancelling and Cancelled
struct Cancel : sc::simple_state<Cancel, JobFSM, Cancelling>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancel() { }
	~Cancel() { }

	void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Cancelling : sc::simple_state<Cancelling, Cancel>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
				   sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
				   sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Cancelling() { }
	~Cancelling() { }

    sc::result react( const sdpa::events::CancelJobAckEvent& );
	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, Cancel>
{
typedef mpl::list< 	sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
					sc::in_state_reaction<sdpa::events::CancelJobAckEvent, JobFSM, &JobFSM::action_cancel_job_ack >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::in_state_reaction<sdpa::events::RetrieveJobResultsEvent, JobFSM, &JobFSM::action_retrieve_job_results >,
                    sc::custom_reaction<sc::exception_thrown> > reactions;

	Cancelled() { }
	~Cancelled() { }

    sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< 	sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction<sdpa::events::RetrieveJobResultsEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { }
	~Failed() { }

	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sdpa::events::RetrieveJobResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list<  sc::in_state_reaction< sdpa::events::QueryJobStatusEvent/*, JobFSM, &JobFSM::action_query_job_status*/ >,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction<sdpa::events::RetrieveJobResultsEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { }
	~Finished() { }

	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sdpa::events::RetrieveJobResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

}}}

Pending::custom_static_type_ptr( "Pending" );
Running::custom_static_type_ptr( "Running" );
Cancelling::custom_static_type_ptr( "Cancelling" );
Cancelled::custom_static_type_ptr( "Cancelled" );
Cancel::custom_static_type_ptr( "Cancel" )
Failed::custom_static_type_ptr( "Failed" );;
Finished::custom_static_type_ptr( "Finished" );;



#endif // JobFSM_H
