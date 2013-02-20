/*
 * =====================================================================================
 *
 *       Filename:  BasicEngine.hpp
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
#ifndef BASIC_WORKFLOW_ENGINE_HPP
#define BASIC_WORKFLOW_ENGINE_HPP 1

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

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
#include <map>

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

typedef boost::function<id_type()> Function_t;
typedef std::list<std::string> list_values_t;

class BasicEngine : public IWorkflowEngine
{
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;

    typedef boost::bimap<boost::bimaps::multiset_of<std::string>, boost::bimaps::multiset_of<std::string> > BimapAct2WfT;
    typedef BimapAct2WfT::value_type PairAct2WfT;

    typedef boost::tuple<id_type, encoded_type, requirement_list_t> Tuple;
    typedef SynchronizedQueue<std::list<Tuple> > TaskQueueT;

    typedef boost::condition_variable_any condition_type;

    BasicEngine( GenericDaemon* pIAgent = NULL, Function_t f = Function_t() )
    : SDPA_INIT_LOGGER(pIAgent->name()+": BasicEngine")
    {
      pIAgent_ = pIAgent;
      start();
      //SDPA_LOG_DEBUG("BasicEngine workflow engine created ...");
    }

    ~BasicEngine()
    {
      stop();
    }

    void connect( GenericDaemon* pIAgent )
    {
	    pIAgent_ = pIAgent;
    }

    virtual bool is_real() { return false; }

    void submit(const id_type&, const encoded_type & ) {}

    bool cancel( const id_type& wfid, const reason_type& reason )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("Called cancel workflow, wfid = "<<wfid);
      return true;
    }

    bool cancelled(const id_type& activityId)
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" was cancelled!");
      return false;
    }

    bool finished(const id_type& activityId, const result_type& strResult )
    {
      lock_type lock(mtx_);
      SDPA_LOG_INFO("The activity " << activityId<<" finished!" );

      // determine to which workflow the activity <activityId> belongs
      SDPA_LOG_INFO("Get the workflow id corresponding to the activity " <<activityId<<" ..." );
      id_type wfid = getWorkflowId(activityId);
      deleteActivity(activityId);

      if(wfid.empty())
      {
         SDPA_LOG_WARN("No workflow corresponding to the activity "<<activityId<<" was found!" );
         return false;
      }

      SDPA_LOG_INFO("Check if the workflow " <<wfid<<" is completed ..." );

      if(!workflowExist(wfid))
      {
        // store the output on some file and pass the file name as result
        SDPA_LOG_INFO( "Finished to compute all the map tasks! Tell the master that the workflow "<<wfid<<" finished!");
        pIAgent_->finished( wfid, "" );
      }

      SDPA_LOG_INFO("Delete the activity " <<activityId<<"!" );

      return false;
    }

    bool failed(  const id_type & activityId,
                  const result_type& result,
                  const int error_code,
                  const std::string& reason )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" failed!");
      deleteActivity(activityId);

      return false;
    }

    // id management
    void addActivity(const id_type& newActId, const id_type& wfid)
    {
      lock_type lock(mtx_id_);
      bimapActId2WfId_.insert(PairAct2WfT(newActId, wfid));
      //should throw an exception if the activity already exists
    }

    void deleteActivity(const id_type& newActId)
    {
      lock_type lock(mtx_id_);
      bimapActId2WfId_.left.erase(newActId);
    }

    id_type getWorkflowId(const id_type& actId)
    {
      lock_type lock(mtx_id_);

      BimapAct2WfT::left_iterator it = bimapActId2WfId_.left.find(actId);
      if( it != bimapActId2WfId_.left.end() )
        return it->second;

      return "";
    }

    list_values_t getWorkflowIdList(const id_type& actId)
    {
      lock_type lock(mtx_id_);

      list_values_t listValues;
      for( BimapAct2WfT::left_iterator it = bimapActId2WfId_.left.begin(); it != bimapActId2WfId_.left.end(); it++)
      if( it->first == actId )
        listValues.push_back(it->second);

      return listValues;
    }

    bool workflowExist(const id_type& wfid)
    {
      lock_type lock(mtx_id_);

      BimapAct2WfT::right_iterator it = bimapActId2WfId_.right.find(wfid);
      if( it != bimapActId2WfId_.right.end() )
        return true;

      return false;
    }

    template <typename T>
    void enqueueTask( const id_type& wfid, const T& task, const worker_id_t& strReq = "", const id_type& actId = "")
    {
      lock_type lock(mtx_id_);

    	requirement_list_t reqList;
    	id_type newActId;

    	if(!strReq.empty())
    	{
        reqList.push_back(requirement_t(strReq, true));
    	}

    	if(!actId.empty())
    	  newActId = actId;
    	else
    	  newActId = id_generator::instance().next();

      addActivity(newActId, wfid);

    	std::string taskEnc(task.encode());

    	queueTasks_.push(boost::make_tuple(newActId, taskEnc, reqList));
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

      m_thread = boost::thread(boost::bind(&BasicEngine::run, this));
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
      while(!bStopRequested)
      {
        //wait until the last result comes from the workers !
        Tuple t= queueTasks_.pop_and_wait();
        pIAgent_->submit(boost::get<0>(t), boost::get<1>(t), boost::get<2>(t));
      }
    }

  protected:
    mutable GenericDaemon *pIAgent_;
    TaskQueueT queueTasks_;

    mutex_type mtx_id_;
    mutex_type mtx_;
    condition_type condReductionFinished_;
    bool bStopRequested;

  private:

    boost::thread m_thread;
    BimapAct2WfT bimapActId2WfId_;
};


#endif //BASIC_WORKFLOW_ENGINE_HPP
