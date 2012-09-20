/*
 * =====================================================================================
 *
 *       Filename:  ReducerWorkflowEngine.hpp
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
#ifndef REDUCER_WORKFLOW_ENGINE_HPP
#define REDUCER_WORKFLOW_ENGINE_HPP 1

#include <sdpa/engine/BasicEngine.hpp>
#include <sdpa/mapreduce/WordCountMapper.hpp>
#include <sdpa/mapreduce/WordCountReducer.hpp>
#include <sdpa/mapreduce/Combiner.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

class ReducerWorkflowEngine : public BasicEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:

    ReducerWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : SDPA_INIT_LOGGER(pIAgent->name()+": ReducerWE"),
      BasicEngine(pIAgent, f),
      nCntEndSuite_(0)
    {
      SDPA_LOG_DEBUG("Reducer workflow engine created ...");
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

      WordCountMapper::TaskT mapTask;

      try
      {
          mapTask.decode(wf_desc);

          Combiner<WordCountMapper, WordCountReducer>::shuffle(&mapTask, reducer_);

          // accumulate and check if it's final
          nCntEndSuite_++;
          if( nCntEndSuite_ == pIAgent_->numberOfMasterAgents() )
          {
            // build a new map task, out of the reducer associated to this workflow
            WordCountMapper::TaskT mapTask;
            reducer_.collect(mapTask);
            //reducer_.print();

            enqueueTask(wfid, mapTask);

            // clean the reducer  !!!
            // delete pReducer;
            // deleteReducer(wfid);
          }
          else
             pIAgent_->finished( wfid, "");
      }
      catch(const std::exception& exc)
      {
        SDPA_LOG_ERROR("Error occurred when trying to decode the description of the workflow "<<wfid<<", submitted by "<<pIAgent_->name());

        SDPA_LOG_ERROR("Notify the agent "<<pIAgent_->name()<<" that the job "<<wfid<<" failed!");
        int errc = -1;
        std::string reason(exc.what());
        pIAgent_->failed( wfid, "", errc, reason );
      }
    }

  private:
    WordCountReducer reducer_;
    int nCntEndSuite_;
};


#endif //MASTER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP
