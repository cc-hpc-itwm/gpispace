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
#include <sdpa/mapreduce/MapTask.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

template <typename Mapper>
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
        id_type tag = wfid;
        worker_id_t destWorkerId(pairKeyTask.second);
        typename Mapper::TaskT newMapTask(pairKeyTask.first, tag);
        enqueueTask(wfid, newMapTask, "mapper");
      }
    }

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
         pIAgent_->failed( wfid, "", errc, reason );
      }
    }
};

#endif //MASTER_WORKFLOW_ENGINE_HPP
