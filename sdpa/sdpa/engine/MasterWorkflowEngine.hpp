/*
 * =====================================================================================
 *
 *       Filename:  MasterWorkflowEngine.hpp
 *
 *    Description:
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
#ifndef MASTER_WORKFLOW_ENGINE_HPP
#define MASTER_WORKFLOW_ENGINE_HPP 1

#include <sdpa/engine/BasicEngine.hpp>
#include <sdpa/mapreduce/WordCountMapper.hpp>
#include <sdpa/mapreduce/WordCountReducer.hpp>
#include <sdpa/mapreduce/Combiner.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

class MasterWorkflowEngine : public BasicEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:

    typedef MapTask<std::string, std::string, std::string, std::string> UserMapTask;

    MasterWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : BasicEngine(pIAgent, f),
      SDPA_INIT_LOGGER(pIAgent->name()+": MasterWorkflowEngine")
    {
      SDPA_LOG_DEBUG("MasterWorkflowEngine workflow engine created ...");
    }

    void generateMapActivities(const id_type& wfid, const UserMapTask& mapTask)
    {
      // the mapper contain an arry of
      BOOST_FOREACH(const UserMapTask::OutKeyValPairT& pairKeyTask, mapTask.outKeyValueMap())
      {
        WordCountMapper::TaskT newMapTask(pairKeyTask.first, pairKeyTask.second);
        enqueueTask(wfid, newMapTask, pairKeyTask.second);
      }
    }

    bool finished(const id_type& activityId, const result_type& strResult )
    {
      lock_type lock(mtx_);
      SDPA_LOG_INFO("The activity " << activityId<<" finished!" );

      // determine to which workflow the activity <activityId> belongs
      SDPA_LOG_INFO("Get the workflow id corresponding to the activity " <<activityId<<" ..." );
      id_type wfid = getWorkflowId(activityId);

      SDPA_LOG_INFO("Delete the activity " <<activityId<<"!" );
      deleteActivity(activityId);

      if(wfid.empty())
      {
         SDPA_LOG_FATAL("No workflow corresponding to the activity "<<activityId<<" was found!" );
         return false;
      }

      SDPA_LOG_INFO("Check if the workflow " <<wfid<<" is completed ..." );

      if(!workflowExist(wfid))
      {
        // store the output on some file and pass the file name as result
        SDPA_LOG_INFO( "Finished to compute all the tasks! Tell the master that the workflow "<<wfid<<" finished!");
        pIAgent_->finished( wfid, "" );
      }

      return false;
    }

    /**
    * Submit a workflow to the WE.
    * This method is to be invoked by the SDPA.
    * The WE will initiate and start the workflow
    * asynchronously and notifiy the SPDA about status transitions
    * using the callback methods of the Gwes2Sdpa handler.
    */
    void submit(const id_type& wfid, const encoded_type& wf_desc)
    {
      lock_type lock(mtx_);
      // WE is supposed to parse the workflow and generate a suite of sub-workflows or activities that are sent to SDPA
      // WE assigns an unique workflow_id which will be used as a job_id on SDPA side
      SDPA_LOG_INFO("The agent submitted a new workflow, wfid = "<<wfid);

      // It is assumed that the workflow description
      // is an encoded map of files to nodes !

      UserMapTask userMapTask;

      try {
        userMapTask.decode(wf_desc);

        SDPA_LOG_INFO("Generate map activities!");

        // prepare and enqueue new map tasks generated out of map task
        // received from the user
        generateMapActivities(wfid, userMapTask); // generate orchestrator activities
      }
      catch(const std::exception& exc)
      {
         SDPA_LOG_ERROR("Error occurred when trying to decode the description of the workflow "<<wfid<<", submitted by "<<pIAgent_->name());

         //delete pMapper;
         SDPA_LOG_ERROR("Notify the agent "<<pIAgent_->name()<<" that the job "<<wfid<<" failed!");
         int errc = -1;
         std::string reason(exc.what());
         pIAgent_->failed( wfid, "" , errc, reason );
      }
    }
};

#endif //MASTER_WORKFLOW_ENGINE_HPP
