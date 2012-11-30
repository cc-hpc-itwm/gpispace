/*
 * =====================================================================================
 *
 *       Filename:  CollectorWorkflowEngine.hpp
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
#ifndef COLLECTOR_WORKFLOW_ENGINE_HPP
#define COLLECTOR_WORKFLOW_ENGINE_HPP 1

#include <sdpa/engine/BasicEngine.hpp>
#include <sdpa/mapreduce/Combiner.hpp>

#include <cstdio>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

template <typename Mapper, typename Reducer>
class CollectorWorkflowEngine : public BasicEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef std::map<id_type, Reducer*> MapTag2ReducerT;
    typedef typename MapTag2ReducerT::value_type PairTagReducerT;

    CollectorWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : BasicEngine(pIAgent, f),
      SDPA_INIT_LOGGER(pIAgent->name()+": CollectorWE"),
      output_filename("reducer.out")
    {
      SDPA_LOG_DEBUG("MapReduce workflow engine created ...");
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

      typename Mapper::TaskT fromRedMapTask;

      try
      {
        fromRedMapTask.decode(wf_desc);

        SDPA_LOG_INFO("Got new map map task tagged as: "<<fromRedMapTask.inValue());

        id_type tag(fromRedMapTask.inValue());
        Reducer* pReducer = getReducer(tag);

        Combiner<Mapper, Reducer>::shuffle(&fromRedMapTask, pReducer);

        if( pReducer->isBoundReached( pIAgent_->numberOfMasterAgents() ))
        {
          typename Mapper::TaskT mapTask;
          pReducer->collect(mapTask);

          mapTask.print(output_filename);
          remove(tag);
        }

        pIAgent_->finished( wfid, output_filename);
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
    std::string output_filename;
};


#endif //MASTER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP
