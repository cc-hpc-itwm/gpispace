/*
 * =====================================================================================
 *
 *       Filename:  WorkerMapReduceWorkflowEngine.hpp
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
#ifndef WORKER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP
#define WORKER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP 1

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/mapreduce/WordCountMapper.hpp>
#include <sdpa/mapreduce/WordCountReducer.hpp>
#include <sdpa/mapreduce/Combiner.hpp>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

typedef boost::function<id_type()> Function_t;
//static std::string generateNewId() { return id_generator::instance().next(); }
static std::string id_gen2() { return id_generator::instance().next(); }

typedef std::list<std::string> list_values_t;

class WorkerMapReduceWorkflowEngine : public IWorkflowEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;
    typedef std::map<id_type, id_type> mapAct2WfT;
    typedef std::map<id_type, id_type> map_FileName2Node_t;
    typedef boost::tuple<id_type, encoded_type, requirement_list_t> Tuple;
    typedef SynchronizedQueue<std::list<Tuple> > TaskQueueT;
    typedef std::map<id_type, std::pair<WordCountMapper*, WordCountReducer*> > MapWf2MapReducerT;

    typedef boost::condition_variable_any condition_type;

    WorkerMapReduceWorkflowEngine( GenericDaemon* pIAgent = NULL, Function_t f = id_gen2 )
    : SDPA_INIT_LOGGER(pIAgent->name()+": MapReduceWE")
    {
      pIAgent_ = pIAgent;
      start();
      SDPA_LOG_DEBUG("MapReduce workflow engine created ...");
    }

    ~WorkerMapReduceWorkflowEngine()
    {
      stop();
    }

    virtual bool is_real() { return false; }

    void connect( GenericDaemon* pIAgent )
    {
      pIAgent_ = pIAgent;
    }

    /**
     * Notify the WE that an activity has failed
     * (state transition from "running" to "failed").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool failed(  const id_type & activityId,
                  const result_type& result,
                  const int error_code,
                  const std::string & reason )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" failed!");
      mapActId2WfId_.erase(activityId);

      return false;
    }

    /**
     * Notify the WE that an activity has forward
     * (state transition from running to forward).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool finished(const id_type& activityId, const result_type& strEncodedResult )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" finished!" );
      SDPA_LOG_DEBUG("Decode the result of the activity "<<activityId);


      return false;
    }

    /**
     * Notify the WE that an activity has been canceled
     * (state transition from * to terminated).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool cancelled(const id_type& activityId)
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" was cancelled!");
      return false;
    }

    // Attention: it is assumed tjat the workflow description is an
    // encoded string which represents the serialization of a
    // map of type [(filename, resident node)]
    void generateMapActivities(const id_type& wfid)
    {
      BOOST_FOREACH(WordCountMapper::MapOfTasksT::value_type& pairKeyTask, mapWf2MapReducer_[wfid].first->mapOfTasks())
      {
        std::ostringstream oss;
        oss<<"node="<<pairKeyTask.second.inValue();
        requirement_list_t reqList(1, requirement_t(oss.str(), true));

        id_type newActId = id_gen2();
        // no encoding, just send the string that denotes the file name
        mapActId2WfId_.insert(mapAct2WfT::value_type(newActId, wfid));

        queueTasks_.push(boost::make_tuple(newActId, encode(&pairKeyTask.second), reqList));
      }
    }

    // Attention: it is assumed that the workflow description is an
    // encoded string which represents the serialization of a
    // map of type [(filename, resident node)]
    void generateReduceActivities(const id_type& wfid)
    {
      BOOST_FOREACH(WordCountReducer::MapOfTasksT::value_type& pairKeyTask, mapWf2MapReducer_[wfid].second->mapOfTasks())
      {
        id_type newActId = id_gen2();
        // no encoding, just send the string that denotes the file name
        mapActId2WfId_.insert(mapAct2WfT::value_type(newActId, wfid));
        // no requirements for the reduce tasks

        queueTasks_.push(boost::make_tuple(newActId, encode(&pairKeyTask.second), requirement_list_t()));
      }
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


      // the mapper and the reducer should be derived from a
      // common class and should have a virtual method run
      Task* pTask = decode(wf_desc);

      WordCountMapper* pMapper(new WordCountMapper);
      WordCountReducer* pReducer(new WordCountReducer);

      // attention, the node value can be empty!!!
      // create a mapper and a reducer for this workflow

      if( WordCountMapTask* pMapTask = dynamic_cast<WordCountMapTask*>(pTask) )
      {
        //pMapper->addTask(pMapTask->inKey(), *pMapTask);
        //mapWf2MapReducer_.insert(MapWf2MapReducerT::value_type(wfid, std::pair<WordCountMapper*, WordCountReducer*>(pMapper, pReducer)));
        //generateMapActivities(wfid);

        // read the file
        pMapTask->run();
        Combiner<WordCountMapper, WordCountReducer>::shuffle(pMapTask, pReducer);

        pMapTask->clear();

        BOOST_FOREACH(WordCountReducer::MapOfTasksT::value_type& pairKeyTask, pReducer->mapOfTasks())
        {
          WordCountReducer::InKeyT key = pairKeyTask.first;
          WordCountReducer::TaskT& reduceTask = pairKeyTask.second;
          reduceTask.run();
          WordCountReducer::OutValueT value = reduceTask.listOutValues().front();
          pMapTask->emit(key, value);
        }

        std::string strEncodedResult(encode(pMapTask));
        pIAgent_->finished(wfid, strEncodedResult);
      }
      else if( dynamic_cast<WordCountReduceTask*>(pTask))
      {
        // if it's a simple reduce task just aggregate the results
        pTask->run();
        std::string strEncodedResult(encode(pTask));

        pIAgent_->finished(wfid, strEncodedResult);
      }
    }

    /**
     * Cancel a workflow asynchronously.
     * This method is to be invoked by the SDPA.
     * The WE will notifiy the SPDA about the
     * completion of the cancelling process by calling the
     * callback method Gwes2Sdpa::cancelled.
     */
    bool cancel( const id_type& wfid, const reason_type& reason )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("Called cancel workflow, wfid = "<<wfid);
      return true;
    }

    // thread related functions
    void start()
    {
      bStopRequested = false;
      if(!pIAgent_)
      {
        SDPA_LOG_ERROR("The Workfow engine cannot be started. Invalid communication handler. ");
        return;
      }

      m_thread = boost::thread(boost::bind(&WorkerMapReduceWorkflowEngine::run, this));
      SDPA_LOG_DEBUG("MasterAgentMapReduceWE thread started ...");
    }

    void stop()
    {
      bStopRequested = true;
      m_thread.interrupt();
      DLOG(TRACE, "MasterAgentMapReduceWE thread before join ...");
      m_thread.join();
      DLOG(TRACE, "MasterAgentMapReduceWE thread joined ...");
    }

    void run()
    {
      //lock_type lock(mtx_stop);
      while(!bStopRequested)
      {
        //wait until the last result comes from the workers !
        Tuple t= queueTasks_.pop_and_wait();
        pIAgent_->submit(boost::get<0>(t), boost::get<1>(t), boost::get<2>(t));
      }
    }

    bool fill_in_info ( const id_type & id, activity_information_t &) const
    {
      return false;
    }

    std::string encode(Task *pTask) const
    {
      std::ostringstream sstr;
      boost::archive::text_oarchive ar(sstr);

      ar.register_type(static_cast<WordCountMapper::TaskT*>(NULL));
      ar.register_type(static_cast<WordCountReducer::TaskT*>(NULL));

      ar << pTask;
      return sstr.str();
    }

    Task *decode(const std::string &s) const
    {
      std::istringstream sstr(s);
      boost::archive::text_iarchive ar(sstr);

      ar.register_type(static_cast<WordCountMapper::TaskT*>(NULL));
      ar.register_type(static_cast<WordCountReducer::TaskT*>(NULL));

      Task* pTask(NULL);
      ar >> pTask;
      return pTask;
    }

  public:
    mutable GenericDaemon *pIAgent_;

  private:
    mutex_type mtx_;
    condition_type condReductionFinished_;

    bool bStopRequested;
    boost::thread m_thread;

    mapAct2WfT mapActId2WfId_;
    TaskQueueT queueTasks_;

    MapWf2MapReducerT mapWf2MapReducer_;
};


#endif //MASTER_AGENT_MAPREDUCE_WORKFLOW_ENGINE_HPP
