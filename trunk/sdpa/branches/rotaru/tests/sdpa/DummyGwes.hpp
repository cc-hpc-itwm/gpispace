#ifndef DUMMY_WORKFLOW_HPP
#define DUMMY_WORKFLOW_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/logging.hpp"

#include <gwes/Gwes2Sdpa.h>
#include <sdpa/Sdpa2Gwes.hpp>

// for job_desc_t
#include <sdpa/types.hpp>

#include <sdpa/uuidgen.hpp>
#include <map>

#include <boost/config.hpp>
#include <iostream>

#include <boost/bimap.hpp>


using namespace sdpa;
using namespace gwes;

typedef boost::bimap< std::string, std::string > bimap_t;
typedef bimap_t::value_type id_pair;

/*namespace gwdl
  {
  class Place {
  Place() {}
  };
  }*/

class DummyWorkflow : public gwes::workflow_t
{
  public:
    DummyWorkflow(const sdpa::job_desc_t& /* desc */ ) { }

    const gwes::workflow_id_t &getID() const { return wf_id_; }
    void setID(const gwes::workflow_id_t &id) { wf_id_ = id; }

    std::string serialize() const { return "serialized workflow"; }
    void deserialize(const std::string &) {}
    gwdl::Place* getPlace(const std::string& /* id */) { return NULL; }

  private:
    gwes::workflow_id_t wf_id_;
};


class DummyActivity : public gwes::activity_t
{
  public:
    DummyActivity(  gwes::activity_id_t act_id_arg, gwes::workflow_id_t owner_wf_id_arg )
    {
      act_id_ = act_id_arg;
      owner_wf_id_ = owner_wf_id_arg;
    }

    void  setID( const activity_id_t &id_arg ) { act_id_ = id_arg; }
    const activity_id_t &getID() const { return act_id_; }

    virtual const gwdl::IWorkflow::workflow_id_t &getOwnerWorkflowID() const { return owner_wf_id_; }

    gwdl::IWorkflow::ptr_t transform2Workflow() const
    {
      gwdl::IWorkflow::ptr_t pWf( new DummyWorkflow( act_id_ ));
      return pWf;
    }

  private:
    gwes::activity_id_t act_id_;
    gwes::workflow_id_t owner_wf_id_;

};

class DummyGwes : public sdpa::Sdpa2Gwes {
  private:
    SDPA_DECLARE_LOGGER();
  public:
    DummyGwes() : SDPA_INIT_LOGGER("sdpa.tests.DummyGwes")
  {
    ptr_Gwes2SdpaHandler = NULL;
    SDPA_LOG_DEBUG("Dummy workflow engine created ...");
  }

    /**
     * Notify the GWES that an activity has been dispatched
     * (state transition from "pending" to "running").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submitActivity().
     */
    virtual void activityDispatched( const workflow_id_t & /* workflowId */
        , const activity_id_t & /* activityId */) throw (NoSuchWorkflow,NoSuchActivity)

    {
      SDPA_LOG_DEBUG("Called activityDispatched ...");
    }

    /**
     * Notify the GWES that an activity has failed
     * (state transition from "running" to "failed").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submitActivity().
     */
    virtual void activityFailed(const workflow_id_t &workflowId
                              , const activity_id_t &activityId
                              , const parameter_list_t & /* output */) throw (NoSuchWorkflow,NoSuchActivity)
    {
      SDPA_LOG_DEBUG("Called activityFailed ... wid:" << workflowId << " aid:" << activityId);

      if(ptr_Gwes2SdpaHandler)
      {
        // find the corresponding workflow
        workflow_id_t wf_id_orch = bimap_wf_act_ids_.right.at(activityId);
        ptr_Gwes2SdpaHandler->workflowFailed(wf_id_orch);
        bimap_wf_act_ids_.left.erase(wf_id_orch);
      }
      else
        SDPA_LOG_ERROR("SDPA has unregistered ...");
    }

    /**
     * Notify the GWES that an activity has finished
     * (state transition from running to finished).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submitActivity().
     */
    virtual void activityFinished(const workflow_id_t &workflowId
                                , const activity_id_t &activityId
                                , const parameter_list_t &/* output */) throw (NoSuchWorkflow,NoSuchActivity)

    {
      SDPA_LOG_DEBUG("Called activityFinished ... wid:" << workflowId << " aid:" << activityId);

      if(ptr_Gwes2SdpaHandler)
      {
        // find the corresponding workflow
        workflow_id_t wf_id_orch = bimap_wf_act_ids_.right.at(activityId);
        ptr_Gwes2SdpaHandler->workflowFinished(wf_id_orch);
        bimap_wf_act_ids_.left.erase(wf_id_orch);
      }
      else
        SDPA_LOG_ERROR("SDPA has unregistered ...");
    }

    /**
     * Notify the GWES that an activity has been canceled
     * (state transition from * to terminated).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submitActivity().
     */
    virtual void activityCanceled(const workflow_id_t &workflowId
                                , const activity_id_t &activityId) throw (NoSuchWorkflow,NoSuchActivity)
    {
      SDPA_LOG_DEBUG("Called activityCanceled ... wid:" << workflowId << " aid:" << activityId);

      /**
       * Notify the SDPA that a workflow has been canceled (state
       * transition from * to terminated.
       */
      //try
      {
        if(ptr_Gwes2SdpaHandler)
        {
          // find the corresponding workflow
          workflow_id_t wf_id_orch = bimap_wf_act_ids_.right.at(activityId);
          ptr_Gwes2SdpaHandler->workflowCanceled(wf_id_orch);
          bimap_wf_act_ids_.left.erase(wf_id_orch);
        }
        else
          SDPA_LOG_ERROR("SDPA has unregistered ...");
      }
      /*catch (NoSuchWorkflowException&)
        {
        SDPA_LOG_ERROR("NoSuchWorkflowException occured!");
        }*/
    }

    /**
     * Register a SDPA handler that implements the Gwes2Sdpa
     * interface. This handler is notified on each status
     * transitions of each workflow. This handler is also used
     * by the GWES to delegate the execution of activities or
     * sub workflows to the SDPA.
     * Currently you can only register ONE handler for a GWES.
     */
    virtual void registerHandler(Gwes2Sdpa *pSdpa)
    {
      ptr_Gwes2SdpaHandler = pSdpa;
      SDPA_LOG_DEBUG("Called registerHandler ...");
    }

    /**
     * UnRegister a SDPA handler that implements the Gwes2Sdpa
     * interface. This handler is notified on each status
     * transitions of each workflow. This handler is also used
     * by the GWES to delegate the execution of activities or
     * sub workflows to the SDPA.
     * Currently you can only register ONE handler for a GWES.
     */
    virtual void unregisterHandler(Gwes2Sdpa *pSdpa)
    {
      if( pSdpa == ptr_Gwes2SdpaHandler)
        ptr_Gwes2SdpaHandler = NULL;

      SDPA_LOG_DEBUG("SDPA has unregistered ...");
    }

    /*
     * initialize and start internal datastructures
     */
    virtual void start()
    {

    }

    /*
     * stop and destroy internal datastructures
     */
    virtual void stop()
    {

    }

    /**
     * Submit a workflow to the GWES.
     * This method is to be invoked by the SDPA.
     * The GWES will initiate and start the workflow
     * asynchronously and notifiy the SPDA about status transitions
     * using the callback methods of the Gwes2Sdpa handler.
     */
    virtual workflow_id_t submitWorkflow(workflow_t &workflow) throw (std::exception)
    {
      // GWES is supposed to parse the workflow and generate a suite of
      // sub-workflows or activities that are sent to SDPA
      // GWES assigns an unique workflow_id which will be used as a job_id
      // on SDPA side
      SDPA_LOG_DEBUG("Called submitWorkflow ...");

      // save the workflow_id
      workflow_id_t wf_id_orch = workflow.getID();
      SDPA_LOG_DEBUG("Generate activity ...");

      // generate unique id
      uuid uid;
      uuidgen gen;
      gen(uid);
      activity_id_t act_id = uid.str();

      // either you assign here an id or it be assigned by daemon
      DummyActivity dummyActivity(act_id, wf_id_orch);


      //bimap_wf_act_ids_[wf_id_orch] = act_id;
      bimap_wf_act_ids_.insert( id_pair(wf_id_orch, act_id) );


      if(ptr_Gwes2SdpaHandler)
      {
        SDPA_LOG_DEBUG("Gwes submits new activity ...");
        ptr_Gwes2SdpaHandler->submitActivity(dummyActivity);
      }
      else
        SDPA_LOG_ERROR("SDPA has unregistered ...");

      return workflow.getID();
    }

    /**
     * Cancel a workflow asynchronously.
     * This method is to be invoked by the SDPA.
     * The GWES will notifiy the SPDA about the
     * completion of the cancelling process by calling the
     * callback method Gwes2Sdpa::workflowCanceled.
     */
    virtual void cancelWorkflow(const workflow_id_t &workflowId) throw (std::exception)
    {
      SDPA_LOG_DEBUG("Called cancelWorkflow ...");

      if(ptr_Gwes2SdpaHandler)
      {
        SDPA_LOG_DEBUG("Gwes cancels the activities related to the workflow "<<workflowId);
        activity_id_t act_id = bimap_wf_act_ids_.left.at(workflowId);
        ptr_Gwes2SdpaHandler->cancelActivity(act_id);
      }
      else
        SDPA_LOG_ERROR("SDPA has unregistered ...");
    }

  private:
    mutable Gwes2Sdpa *ptr_Gwes2SdpaHandler;
    bimap_t bimap_wf_act_ids_;
    //use here a bidirectional map!
    /*workflow_id_t wf_id_orch;
      activity_id_t act_id;*/
};

#endif //DUMMY_WORKFLOW_HPP
