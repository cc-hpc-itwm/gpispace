/*
 * =====================================================================================
 *
 *       Filename:  SchedulerAgg.hpp
 *
 *    Description:  The aggregator's scheduler
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
#ifndef SDPA_SCHEDULERAGG_HPP
#define SDPA_SCHEDULERAGG_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

#include <sdpa/mapreduce/WordCountMapper.hpp>
#include <sdpa/mapreduce/WordCountReducer.hpp>
#include <sdpa/mapreduce/Combiner.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
	namespace daemon {
  class AgentScheduler : public SchedulerImpl {

  public:
	 AgentScheduler(sdpa::daemon::IComm* pCommHandler = NULL,  bool use_request_model=true):
	   SchedulerImpl(pCommHandler, use_request_model),
	   SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name()+"::Scheduler":"Scheduler")
	{
	}

	 virtual ~AgentScheduler()
	 {
		 try
		 {
			 LOG(TRACE, "destructing SchedulerAgg");
			 stop();
		 }
		 catch (std::exception const & ex)
		 {
			 LOG(ERROR, "could not stop SchedulerAgg: " << ex.what());
		 }
	 }

	 friend class boost::serialization::access;

	 template <class Archive>
	 void serialize(Archive& ar, const unsigned int /* file_version */)
	 {
		 ar & boost::serialization::base_object<SchedulerImpl>(*this);
	 }

	void execute(const sdpa::job_id_t& jobId)
  {
    MLOG(TRACE, "executing activity: "<< jobId);
    const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);
    id_type act_id = pJob->id().str();

    execution_result_t result;
    encoded_type enc_act = pJob->description(); // assume that the NRE's workflow engine encodes the activity!!!

    if( !ptr_comm_handler_ )
    {
      // the comments indicate that the code is old (nre)
      // why does it still exist?
      // why don't we just have an assert(ptr_comm_handler_)?

      LOG(ERROR, "nre scheduler does not have a comm-handler!");
      result_type output_fail;
      ptr_comm_handler_->activityFailed ( ""
                                        , jobId
                                        , enc_act
                                        , fhg::error::UNEXPECTED_ERROR
                                        , "scheduler does not have a"
                                        " communication handler"
                                        );
      return;
    }

    try
    {
      pJob->Dispatch();
      //result = m_worker_.execute(enc_act, pJob->walltime());

      std::string strEncodedResult;

      try {
        /*Task* pTask = decode(enc_act);

        pTask->print();
        pTask->run();
        strEncodedResult = encode(pTask);*/

        // the mapper and the reducer should be derived from a
        // common class and should have a virtual method run
        Task* pTask = decode(enc_act);

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

          strEncodedResult = encode(pMapTask);
        }
        else if( dynamic_cast<WordCountReduceTask*>(pTask))
        {
          // if it's a simple reduce task just aggregate the results
          pTask->run();
          strEncodedResult = encode(pTask);
        }
      }
      catch(...)
      {
         boost::this_thread::sleep(boost::posix_time::milliseconds(2));
      }

      result = std::make_pair(ACTIVITY_FINISHED, strEncodedResult);
    }
    catch( const boost::thread_interrupted &)
    {
      std::string errmsg("could not execute activity: interrupted");
      SDPA_LOG_ERROR(errmsg);
      result = std::make_pair(ACTIVITY_FAILED, enc_act);
    }
    catch (const std::exception &ex)
    {
      std::string errmsg("could not execute activity: ");
      errmsg += std::string(ex.what());
      SDPA_LOG_ERROR(errmsg);
      result = std::make_pair(ACTIVITY_FAILED, enc_act);
    }

    // check the result state and invoke the NRE's callbacks
    if( result.first == ACTIVITY_FINISHED )
    {
      DLOG(TRACE, "activity finished: " << act_id);
      // notify the gui
      // and then, the workflow engine
      ptr_comm_handler_->activityFinished("", jobId, result.second);
    }
    else if( result.first == ACTIVITY_FAILED )
    {
      DLOG(TRACE, "activity failed: " << act_id);
      // notify the gui
      // and then, the workflow engine
      ptr_comm_handler_->activityFailed ( ""
                                        , jobId
                                        , result.second
                                        , fhg::error::UNEXPECTED_ERROR
                                        , "scheduler could not dispatch job"
                                        );
    }
    else if( result.first == ACTIVITY_CANCELLED )
    {
      DLOG(TRACE, "activity cancelled: " << act_id);

      // notify the gui
      // and then, the workflow engine
      ptr_comm_handler_->activityCancelled("", jobId);
    }
    else
    {
      SDPA_LOG_ERROR("Invalid status of the executed activity received from the worker!");
      ptr_comm_handler_->activityFailed ( ""
                                        , jobId
                                        , result.second
                                        , fhg::error::UNEXPECTED_ERROR
                                        , "invalid job state received"
                                        );
    }
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


  private:
	  SDPA_DECLARE_LOGGER();
  };
}}

#endif
