/*
 * =====================================================================================
 *
 *       Filename:  DummyGwes.hpp
 *
 *    Description:  Simulate simple gwes behavior
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
#ifndef DUMMY_WORKFLOW_ENGINE_HPP
#define DUMMY_WORKFLOW_ENGINE_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/logging.hpp"

// for job_desc_t
#include <sdpa/types.hpp>
#include <sdpa/uuidgen.hpp>
#include <map>

#include <boost/config.hpp>
#include <iostream>

#include <boost/bimap.hpp>
#include <map>
#include <boost/thread.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <sdpa/daemon/IWorkflowEngine.hpp>
#include <boost/function.hpp>

using namespace sdpa;

typedef std::map<id_type, id_type> map_t;
typedef map_t::value_type id_pair;

/*struct f_id_gen {
	id_type operator()()
			{ std::string str; return str; }
};*/

static std::string id_gen() { JobId jobId; return jobId.str(); }

typedef boost::function<id_type()> Function_t;

class DummyWorkflowEngine : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();
  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    DummyWorkflowEngine( IDaemon* pIDaemon, Function_t f  ) : SDPA_INIT_LOGGER("sdpa.tests.DummyGwes")
	{
    	pIDaemon_ = pIDaemon;
    	fct_id_gen_ = f;
    	SDPA_LOG_DEBUG("Dummy workflow engine created ...");
    }

    void registerDaemon(IDaemon* pIDaemon )
    {
    	pIDaemon_ = pIDaemon;
    }

    /**
     * Notify the GWES that an activity has failed
     * (state transition from "running" to "failed").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool failed(const id_type& activityId, const result_type & result)
    {
    	SDPA_LOG_DEBUG("The activity " << activityId<<" failed!");

		if(pIDaemon_)
		{
			// find the corresponding workflow_id
			id_type workflowId = map_Act2Wf_Ids_[activityId];
			result_type wf_result;
			pIDaemon_->failed(workflowId, wf_result);

			lock_type lock(mtx_);
			map_Act2Wf_Ids_.erase(activityId);

			return true;
		}
		else
			return false;
    }

    /**
     * Notify the GWES that an activity has finished
     * (state transition from running to finished).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool finished(const id_type& activityId, const result_type & result)
    {
    	SDPA_LOG_DEBUG("The activity " << activityId<<" finished!");

		if(pIDaemon_)
		{
			// find the corresponding workflow_id
			id_type workflowId = map_Act2Wf_Ids_[activityId];

			result_type wf_result;
			pIDaemon_->finished(workflowId, wf_result);

			//delete the entry corresp to activityId;
			lock_type lock(mtx_);
			map_Act2Wf_Ids_.erase(activityId);

			return true;
		}
		else
			return false;
    }

    /**
     * Notify the GWES that an activity has been canceled
     * (state transition from * to terminated).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool cancelled(const id_type& activityId)
    {
		SDPA_LOG_DEBUG("The activity " << activityId<<" was cancelled!");

		/**
		* Notify the SDPA that a workflow has been canceled (state
		* transition from * to terminated.
		*/

		if(pIDaemon_)
		{
			// find the corresponding workflow_id
			id_type workflowId = map_Act2Wf_Ids_[activityId];
			lock_type lock(mtx_);
			map_Act2Wf_Ids_.erase(activityId);

			// check if there are any activities left for that workflow
			bool bAllActFinished = true;
			for( map_t ::iterator it = map_Act2Wf_Ids_.begin(); it != map_Act2Wf_Ids_.end() && bAllActFinished; it++)
				if( it->second == workflowId )
					bAllActFinished = false;

			// if no activity left, declare the workflow cancelled
			if(bAllActFinished)
				pIDaemon_->cancelled(workflowId);

			return true;
		}
		else
			return false;
    }


    /**
     * Submit a workflow to the GWES.
     * This method is to be invoked by the SDPA.
     * The GWES will initiate and start the workflow
     * asynchronously and notifiy the SPDA about status transitions
     * using the callback methods of the Gwes2Sdpa handler.
    */
    void submit(const id_type& wfid, const encoded_type& wf_desc)
    {
		// GWES is supposed to parse the workflow and generate a suite of
		// sub-workflows or activities that are sent to SDPA
		// GWES assigns an unique workflow_id which will be used as a job_id
		// on SDPA side
		SDPA_LOG_DEBUG("Submit new workflow, wfid = "<<wfid);

		//create several activities out of it

		// assign new id
		sdpa::JobId id;
		//id_type act_id = id.str();
		id_type act_id;
		try {
			act_id  = fct_id_gen_();
		}
		catch(boost::bad_function_call& ex) {
			SDPA_LOG_ERROR("Bad function call excecption occurred!");
		}

		// either you assign here an id or it be assigned by daemon
		lock_type lock(mtx_);
		map_Act2Wf_Ids_.insert(id_pair(act_id, wfid));

		// ship the same activity/workflow description
		encoded_type act_desc = wf_desc;
		if(pIDaemon_)
		{
			SDPA_LOG_DEBUG("Submit new activity ...");
			pIDaemon_->submit(act_id, act_desc);
		}
    }

    /**
     * Cancel a workflow asynchronously.
     * This method is to be invoked by the SDPA.
     * The GWES will notifiy the SPDA about the
     * completion of the cancelling process by calling the
     * callback method Gwes2Sdpa::cancelled.
     */
    bool cancel(const id_type& wfid, const reason_type& reason)
	{
		SDPA_LOG_DEBUG("Called cancel workflow, wfid = "<<wfid);

		lock_type lock(mtx_);
		if(pIDaemon_)
		{
			SDPA_LOG_DEBUG("Cancel all the activities related to the workflow "<<wfid);

			for( map_t::iterator it = map_Act2Wf_Ids_.begin(); it != map_Act2Wf_Ids_.end(); it++ )
				if( it->second == wfid )
				{
					id_type activityId = it->first;
					reason_type reason;
					pIDaemon_->cancel(activityId, reason);
				}
		}

		return true;
    }

    template <class Archive>
	void serialize(Archive& ar, const unsigned int file_version )
	{
    	ar & boost::serialization::base_object<IWorkflowEngine>(*this);
		ar & map_Act2Wf_Ids_;
	}

    void print() {
    	for( map_t::iterator it = map_Act2Wf_Ids_.begin(); it != map_Act2Wf_Ids_.end(); it++ )
    		std::cout<<it->second<<" -> "<<it->first<<std::endl;
    }

    virtual void print_statistics (std::ostream & s) const
    {

    }

  public:
    mutable IDaemon *pIDaemon_;

  private:
    map_t map_Act2Wf_Ids_;
    mutex_type mtx_;
    Function_t fct_id_gen_;
};


namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data(
    Archive & ar, const DummyWorkflowEngine* t, const unsigned int file_version
){
    // save data required to construct instance
    ar << t->pIDaemon_;
}

template<class Archive>
inline void load_construct_data(
    Archive & ar, DummyWorkflowEngine* t, const unsigned int file_version
){
    // retrieve data from archive required to construct new instance
	IDaemon *pIDaemon;
    ar >> pIDaemon;

    // invoke inplace constructor to initialize instance of my_class
    ::new(t)DummyWorkflowEngine(pIDaemon, id_gen);
}
}} // namespace ...



BOOST_SERIALIZATION_ASSUME_ABSTRACT(IWorkflowEngine)

#endif //DUMMY_WORKFLOW_ENGINE_HPP
