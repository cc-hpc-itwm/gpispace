/*
 * =====================================================================================
 *
 *       Filename:  MapperWorkflowEngine.hpp
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
#ifndef MAPPER_WORKFLOW_ENGINE_HPP
#define MAPPER_WORKFLOW_ENGINE_HPP 1

#include <sdpa/engine/BasicEngine.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

template <typename Mapper>
class MapperWorkflowEngine : public BasicEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:

    MapperWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : BasicEngine(pIAgent, f),
      SDPA_INIT_LOGGER(pIAgent->name()+": MapperWE")
    {
      SDPA_LOG_DEBUG("Mapper workflow engine created ...");
    }

    // Attention: it is assumed that the workflow description is an
    // encoded string which represents the serialization of a
    // map of type [(filename, resident node)]
    void generateMapActivities(const id_type& wfid, typename Mapper::TaskT& mapTask)
    {
      mapTask.run(); // generate here list of [<word, count>] pairs !(count=1)

      // the map task contains now in outKeyValueMap_ a lot of pairs <word, count> ...
      // should partition the resulted list of pairs
      // and assign them to the predefined reducers
      // for any reducer define a bucket e.g. bucket_1 <-> agent_1 : letters [A..E]
      // bucḱet_2 <-> agent_2 : [F..M], etc

      sdpa::worker_id_list_t workerIdList;
      pIAgent_->scheduler()->getWorkerList(workerIdList);
      size_t nWorkers = workerIdList.size();

      // InKey -> the key is the workflow id
      // InValue -> the worker id where to be scheduled

      Mapper wcMapper(wfid); // the mapper is associated to the workflow wfid

      if(nWorkers)
      {
        wcMapper.partitionate(mapTask, workerIdList);

        // print here the mapper
        // wcMapper.print();

        // now, assign it effectively!
        BOOST_FOREACH(typename Mapper::MapOfTasksT::value_type& pairWorkerTask, wcMapper.mapOfTasks())
        {
          sdpa::worker_id_t workerId(pairWorkerTask.first);
          enqueueTask(wfid, pairWorkerTask.second, workerId);
        }
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

      typename Mapper::TaskT mapTask;
      try {
        mapTask.decode(wf_desc);

        SDPA_LOG_INFO("Generate a lot of new map activities!");
        generateMapActivities(wfid, mapTask);
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
};


#endif //MAPPER_WORKFLOW_ENGINE_HPP
