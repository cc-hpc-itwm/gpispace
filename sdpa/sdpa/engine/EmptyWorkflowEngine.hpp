/*
 * =====================================================================================
 *
 *       Filename:  EMPTYGwes.hpp
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
#ifndef EMPTY_WORKFLOW_ENGINE_HPP
#define EMPTY_WORKFLOW_ENGINE_HPP 1

#include "sdpa/common.hpp"

// for job_desc_t
#include <sdpa/types.hpp>
#include <sdpa/events/id_generator.hpp>
#include <map>

#include <boost/config.hpp>
#include <iostream>

#include <map>
#include <boost/thread.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <boost/function.hpp>

using namespace sdpa;

typedef std::map<id_type, id_type> map_t;
typedef map_t::value_type id_pair;

typedef boost::function<id_type()> Function_t;
static std::string id_gen() { return id_generator::instance().next(); }

enum we_status { FINISHED, FAILED, CANCELLED };

class EmptyWorkflowEngine;

class we_result_t
{
public:
    we_result_t(const sdpa::job_id_t& jid, const result_type& res, const we_status& st  )
    {
      jobId = jid;
      status = st;
      result = res;

    }

    we_result_t(const we_result_t& res  )
    {
      jobId  = res.jobId;
      status = res.status;
      result = res.result;

    }

    we_result_t& operator=(const we_result_t& res  )
    {
      if( &res != this)
      {
              jobId  = res.jobId;
              status = res.status;
              result = res.result;
      }

      return *this;
    }

    friend class EmptyWorkflowEngine;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & boost::serialization::base_object<IWorkflowEngine>(*this);
      ar & jobId;
      ar & status;
      ar & result;
    }


   public:
    sdpa::job_id_t jobId;
    we_status status;
    result_type result;
};

class EmptyWorkflowEngine : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();
  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;

    //typedef std::pair<sdpa::job_id_t, result_type>
    typedef sdpa::daemon::SynchronizedQueue<std::list<we_result_t> > ResQueue;

    EmptyWorkflowEngine( sdpa::daemon::GenericDaemon* pGenericDaemon = NULL, Function_t f = id_gen )
        : SDPA_INIT_LOGGER(dynamic_cast<sdpa::daemon::GenericDaemon*>(pGenericDaemon)->name()+"::EmptyWE")
        , bStopRequested(false)
        {
        pGenericDaemon_ = pGenericDaemon;
        fct_id_gen_ = f;
        start();
        SDPA_LOG_DEBUG("Empty workflow engine created ...");
    }

    ~EmptyWorkflowEngine()
        {
       stop();
        }

    void connect(sdpa::daemon::GenericDaemon* pGenericDaemon )
    {
      pGenericDaemon_ = pGenericDaemon;
    }

    void set_id_generator ( Function_t f = id_gen )
    {
      fct_id_gen_ = f;
    }

    /**
     * Notify the GWES that an activity has failed
     * (state transition from "running" to "failed").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool failed( const id_type& activityId
               , const result_type & result
               , int error_code
               , std::string const & reason
               )
    {
      SDPA_LOG_DEBUG("The activity " << activityId<<" failed!");

      if(pGenericDaemon_)
      {
        // find the corresponding workflow_id
        const id_type workflowId = map_Act2Wf_Ids_[activityId];

        //pGenericDaemon_->failed(workflowId, result);
        we_result_t resP(workflowId, result, FAILED);
        qResults.push(resP);

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
    bool finished(const id_type& activityId, const result_type& result )
    {
      SDPA_LOG_DEBUG("The activity " << activityId<<" has finished!");

      if(pGenericDaemon_)
      {
        // find the corresponding workflow_id
        const id_type workflowId = map_Act2Wf_Ids_[activityId];

        //pGenericDaemon_->finished(workflowId, result);
        we_result_t resP(workflowId, result, FINISHED);
        qResults.push(resP);

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

      if(pGenericDaemon_)
      {
        // find the corresponding workflow_id
        const id_type workflowId = map_Act2Wf_Ids_[activityId];
        lock_type lock(mtx_);
        map_Act2Wf_Ids_.erase(activityId);

        // check if there are any activities left for that workflow
        bool bAllActFinished = true;
        for( map_t ::iterator it = map_Act2Wf_Ids_.begin(); it != map_Act2Wf_Ids_.end() && bAllActFinished; it++)
          if( it->second == workflowId )
            bAllActFinished = false;

        // if no activity left, declare the workflow cancelled
        if(bAllActFinished)
        {
          //pGenericDaemon_->cancelled(workflowId);
          result_type result;
          const we_result_t resP(workflowId, result, CANCELLED);
          qResults.push(resP);
        }

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
      if(pGenericDaemon_)
      {
        SDPA_LOG_DEBUG("Submit to the agent "<<pGenericDaemon_->name()<<" new activity \""<<act_id<<"\"");
        pGenericDaemon_->submit(act_id, wf_desc, empty_req_list());
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
      if(pGenericDaemon_)
      {
        SDPA_LOG_DEBUG("Cancel all the activities related to the workflow "<<wfid);

        for( map_t::iterator it = map_Act2Wf_Ids_.begin(); it != map_Act2Wf_Ids_.end(); it++ )
          if( it->second == wfid )
          {
            id_type activityId = it->first;
            pGenericDaemon_->cancel(activityId, reason);
          }
      }

      return true;
    }

    // thread related functions
    void start()
    {
      bStopRequested = false;

      if(!pGenericDaemon_)
      {
        SDPA_LOG_ERROR("The Workfow engine cannot be started. Invalid communication handler. ");
        return;
      }

      m_thread = boost::thread(boost::bind(&EmptyWorkflowEngine::run, this));

      SDPA_LOG_DEBUG("EmptyWE thread started ...");
    }

    void stop()
    {
      bStopRequested = true;
      m_thread.interrupt();
      DLOG(TRACE, "EmptyWE thread before join ...");
      if(m_thread.joinable())
    	  m_thread.join();

      DLOG(TRACE, "EmptyWE thread joined ...");
    }

    void run()
    {
      lock_type lock(mtx_stop);
      while(!bStopRequested)
      {
        //cond_stop.wait(lock);
        we_result_t we_result = qResults.pop_and_wait();

        switch(we_result.status)
        {
          case FINISHED:
              SDPA_LOG_INFO("Notify the agent "<<pGenericDaemon_->name()<<" that the job "<<we_result.jobId.str()<<" successfully finished!");
              pGenericDaemon_->finished(we_result.jobId, we_result.result);
              break;

          case FAILED:
              SDPA_LOG_INFO("Notify the agent "<<pGenericDaemon_->name()<<" that the job "<<we_result.jobId.str()<<" failed!");
              pGenericDaemon_->failed( we_result.jobId
                                     , we_result.result
                                     , fhg::error::UNEXPECTED_ERROR
                                     , "job failed, unknown reason"
                                     );
              break;

          case CANCELLED:
              SDPA_LOG_INFO("Notify the agent "<<pGenericDaemon_->name()<<" that the job "<<we_result.jobId.str()<<" was canceled!");
              pGenericDaemon_->cancelled(we_result.jobId);
              break;

          default:
              SDPA_LOG_ERROR("Invalid status for the the job "<<we_result.jobId.str()<<"!");
        }
      }
    }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & boost::serialization::base_object<IWorkflowEngine>(*this);
      ar & map_Act2Wf_Ids_;
      ar & qResults;
    }

  public:
    mutable sdpa::daemon::GenericDaemon *pGenericDaemon_;

  private:
    map_t map_Act2Wf_Ids_;
    mutex_type mtx_;
    Function_t fct_id_gen_;
    bool bStopRequested;
    boost::thread m_thread;

    ResQueue qResults;

    mutable mutex_type mtx_stop;
    boost::condition_variable_any cond_stop;
};


namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data(
    Archive & ar, const EmptyWorkflowEngine* t, const unsigned int
){
    // save data required to construct instance
    ar << t->pGenericDaemon_;
}

template<class Archive>
inline void load_construct_data(
    Archive & ar, EmptyWorkflowEngine* t, const unsigned int
){
    // retrieve data from archive required to construct new instance
    sdpa::daemon::GenericDaemon *pGenericDaemon;
    ar >> pGenericDaemon;

    // invoke inplace constructor to initialize instance of my_class
    ::new(t)EmptyWorkflowEngine(pGenericDaemon, id_gen);
}
}} // namespace ...


BOOST_SERIALIZATION_ASSUME_ABSTRACT(IWorkflowEngine)

#endif //EMPTY_WORKFLOW_ENGINE_HPP
