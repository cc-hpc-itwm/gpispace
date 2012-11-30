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
#include <sdpa/mapreduce/Combiner.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

template <typename Mapper, typename Reducer>
class ReducerWorkflowEngine : public BasicEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef std::map<id_type, Reducer*> MapTag2ReducerT;
    typedef typename MapTag2ReducerT::value_type PairTagReducerT;

    ReducerWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : BasicEngine(pIAgent, f),
      SDPA_INIT_LOGGER(pIAgent->name()+": ReducerWE")
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

    Reducer* getReducer(const id_type& tag)
    {
      lock_type lock(mtx_);
      Reducer* pReducer;
      typename MapTag2ReducerT::iterator it = mapTag2Reducer_.find(tag);
      if( it == mapTag2Reducer_.end() )
      {
        // no entry for that tag exist
        // therefore, create a new reducer
        pReducer = new Reducer;
        PairTagReducerT pair(tag, pReducer);
        mapTag2Reducer_.insert(pair);
        pReducer->setId(id_generator::instance().next());
      }
      else
        pReducer = it->second;

      return pReducer;
    }

    void submit(const id_type& wfid, const encoded_type& wf_desc)
    {
      lock_type lock(mtx_);

      typename Mapper::TaskT fromMapperTask;

      try
      {
        fromMapperTask.decode(wf_desc);

        SDPA_LOG_INFO("Got new map map task tagged as: "<<fromMapperTask.inValue());

        id_type tag = fromMapperTask.inValue();
        Reducer* pReducer = getReducer(tag);

        Combiner<Mapper, Reducer>::shuffle(&fromMapperTask, pReducer);

        if( pReducer->isBoundReached( pIAgent_->numberOfMasterAgents() ))
        {
          // build a new map task, out of the reducer associated to this workflow
          typename Mapper::TaskT toCollMapTask(fromMapperTask.inKey(), fromMapperTask.inValue());
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
        Reducer* pReducer = mapTag2Reducer_[tag];
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
