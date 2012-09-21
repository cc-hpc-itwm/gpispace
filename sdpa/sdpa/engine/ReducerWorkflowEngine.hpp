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
    typedef std::map<id_type, WordCountReducer*> MapTag2ReducerT;
    typedef MapTag2ReducerT::value_type PairTagReducerT;

    ReducerWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : SDPA_INIT_LOGGER(pIAgent->name()+": ReducerWE"),
      BasicEngine(pIAgent, f)
    {
      SDPA_LOG_DEBUG("Reducer workflow engine created ...");
    }

    bool finished(const id_type& activityId, const result_type& strResult )
    {
      lock_type lock(mtx_);
      SDPA_LOG_INFO("The activity " << activityId<<" finished!" );

      // determine to which workflow the activity <activityId> belongs
      SDPA_LOG_INFO("Get the workflow id corresponding to the activity " <<activityId<<" ..." );
      list_values_t listWfIds = getWorkflowIdList(activityId);

      SDPA_LOG_INFO("Delete the activity " <<activityId<<"!" );
      deleteActivity(activityId);

      BOOST_FOREACH(const id_type& wfid, listWfIds)
      {
        // store the output on some file and pass the file name as result
        SDPA_LOG_INFO( "Finished to compute all the map tasks! Tell the master that the workflow "<<wfid<<" finished!");
        pIAgent_->finished( wfid, strResult);
      }

      return false;
    }

    void submit(const id_type& wfid, const encoded_type& wf_desc)
    {
      lock_type lock(mtx_);

      WordCountMapper::TaskT fromMapperTask;

      try
      {
          fromMapperTask.decode(wf_desc);

          SDPA_LOG_INFO("Got new map map task tagged as: "<<fromMapperTask.inValue());

          WordCountReducer* pReducer;
          id_type tag(fromMapperTask.inValue());
          MapTag2ReducerT::iterator it = mapTag2Reducer_.find(tag);
          if( it == mapTag2Reducer_.end() )
          {
            // no entry for that tag exist
            // therefore, create a new reducer
            pReducer = new WordCountReducer;
            mapTag2Reducer_.insert(PairTagReducerT(tag, pReducer));
            pReducer->setId(id_generator::instance().next());
          }
          else
            pReducer = it->second;

          Combiner<WordCountMapper, WordCountReducer>::shuffle(&fromMapperTask, pReducer);

          if( pReducer->reachedBound( pIAgent_->numberOfMasterAgents() ))
          {
            // build a new map task, out of the reducer associated to this workflow
            WordCountMapper::TaskT toCollMapTask(fromMapperTask.inKey(), fromMapperTask.inValue());
            pReducer->collect(toCollMapTask);
            //reducer_.print();

            enqueueTask(wfid, toCollMapTask, "", pReducer->id());
            remove(tag);
          }
          else
          {
            addActivity(pReducer->id(), wfid);
          }
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

    void remove(const id_type& tag)
    {
      try {
        WordCountReducer* pReducer = mapTag2Reducer_[tag];
        pReducer->clear();
        mapTag2Reducer_[tag]=NULL;
        delete pReducer;
        mapTag2Reducer_.erase(tag);
      }
      catch(std::exception& exc)
      {
        SDPA_LOG_ERROR("Exception occurred when trying to remove the tag "<<tag<<": "<<exc.what());
      }
    }

  private:
    MapTag2ReducerT mapTag2Reducer_;
};

#endif //MASTER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP
