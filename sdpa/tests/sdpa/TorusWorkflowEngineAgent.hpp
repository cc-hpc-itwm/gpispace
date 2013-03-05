/*
 * =====================================================================================
 *
 *       Filename:  TORUSGwes.hpp
 *
 *    Description:  Simulate simple gwes behavior
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
#ifndef TORUS_WORKFLOW_ENGINE_AGENT_HPP
#define TORUS_WORKFLOW_ENGINE_AGENT_HPP 1
#include <TorusToken.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp> // we_result_t

using boost::numeric::ublas::matrix;

class TorusWorkflowEngineAgent : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;
    typedef std::map<id_type, id_type> map_act_ids_t;
    typedef std::map<id_type, Token::ptr_t> map_actId2tokenPtr_t;

    //typedef std::pair<sdpa::job_id_t, result_type>
    typedef sdpa::daemon::SynchronizedQueue<std::list<Token::ptr_t> > TokenQueue;

    TorusWorkflowEngineAgent( sdpa::daemon::GenericDaemon* pIAgent = NULL, Function_t f = id_gen)
    : SDPA_INIT_LOGGER(pIAgent->name()+": AgentTorusWFE")
    {
      pIAgent_ = pIAgent;
      fct_id_gen_ = f;
      start();
      SDPA_LOG_DEBUG("Torus workflow engine created ...");
      // de lucrat aici -> cu neighbors, care-i left, care-i down etc
    }

    ~TorusWorkflowEngineAgent()
    {
      stop();
    }

    virtual bool is_real() { return false; }

    void connect( sdpa::daemon::GenericDaemon* pIAgent )
    {
      pIAgent_ = pIAgent;
    }

    void set_id_generator ( Function_t f = id_gen )
    {
      fct_id_gen_ = f;
    }

    /**
     * Notify the GWES that an activity has failed
     * (state transition from "running" to "failed").
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool failed( const id_type & activityId
               , const result_type & result
               , const int error_code
               , const std::string & reason
               )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("The activity " << activityId<<" failed!");
      return false;
    }

    /**
     * Notify the GWES that an activity has forward
     * (state transition from running to forward).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
    bool finished(const id_type& activityId, const result_type& result )
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG( "The activity " << activityId<<" forward!" );
      pIAgent_->finished( m_mapActIds[activityId], "" );
      m_mapActIds.erase(activityId);

      return false;
    }

    /**
     * Notify the GWES that an activity has been canceled
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

    Token::ptr_t genBlueToken(const Token::ptr_t& pToken)
    {
      Token::ptr_t ptrTok(new Token(BLUE, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim, pToken->block_1()));
      queueBlueTokens.push(ptrTok);
      return ptrTok;
    }

    Token::ptr_t genRedToken(const Token::ptr_t& pToken)
    {
      Token::ptr_t ptrTok(new Token(RED, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim, pToken->block_2()));
      queueRedTokens.push(ptrTok);
      return ptrTok;
    }

    void split(const Token::ptr_t& pToken)
    {
      lock_type lock(mtx_);
      SDPA_LOG_DEBUG("Got YELLOW token ...");

      m_wfid = pToken->activityId();

      // generate one blue and one red token, respectively
      Token::ptr_t pBlueToken = genBlueToken(pToken);
      Token::ptr_t pRedToken  = genRedToken(pToken);

      Token::ptr_t pYellowToken(new Token(YELLOW, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim));
      pYellowToken->activityId() = pToken->activityId();
      mapActId2YellowTokenPtr[pToken->activityId()] = pYellowToken;

      return;
    }

    /**
    * Submit a workflow to the GWES.
    * This method is to be invoked by the SDPA.
    * The GWES will initiate and start the workflow
    * asynchronously and notifiy the SPDA about status transitions
    * using the callback methods of the Gwes2Sdpa handler.
    */
    void submit(const id_type& wfid, const encoded_type& wf_desc)
    {
      lock_type lock(mtx_);
      // GWES is supposed to parse the workflow and generate a suite of
      // sub-workflows or activities that are sent to SDPA
      // GWES assigns an unique workflow_id which will be used as a job_id
      // on SDPA side
      SDPA_LOG_DEBUG("Submit new workflow, wfid = "<<wfid);
      //lock_type lock(mtx_);

      Token::ptr_t pToken(new Token());
      pToken->decode(wf_desc);
      pToken->activityId() = wfid;

      if(pToken->color() == RED)
      {
        SDPA_LOG_DEBUG("Got RED token ...: ");
        queueRedTokens.push(pToken);
      }
      else
        if(pToken->color() == BLUE)
        {
          SDPA_LOG_DEBUG("Got BLUE token ...: ");
          queueBlueTokens.push(pToken);
        }
        else
          if( pToken->color() == YELLOW )
            split(pToken);
          else
            std::cout<<"Invalid token color"<<std::endl;
    }

    void accumulate( const Token::ptr_t& pRedToken, const Token::ptr_t& pBlueToken, const Token::ptr_t& pYellowToken, bool bClear = false )
    {
      size_t nBlockDim = pRedToken->block_1().size1();
      if(bClear)
      {
        pYellowToken->block_1() = matrix_t(nBlockDim, nBlockDim);
      }

      axpy_prod( pBlueToken->block_1(), pRedToken->block_1(), pYellowToken->block_1(), bClear); // C += A * B
    }

    void propagate( const direction_t& dir, const Token::ptr_t& pToken )
    {
      if(dir == RIGHT)
        propagateRight(pToken);
      else
        if( dir == DOWN )
          propagateDown(pToken);
        else
          SDPA_LOG_FATAL("Invalid propagation direction!");
    }

    void propagateRight( const Token::ptr_t& pBlueToken )
    {
      lock_type lock(mtx_);
      int rank = pIAgent_->rank();

      int i = rank / m_nTorusDim;
      int j = rank % m_nTorusDim;

      int rankRight = i*m_nTorusDim+(j+1)%m_nTorusDim;

      try {
        id_type newActId = fct_id_gen_();
        m_mapActIds[newActId] = pBlueToken->activityId();
        pBlueToken->activityId() = newActId;
        pBlueToken->incVisitedNodes();
      }
      catch(boost::bad_function_call& ex)
      {
        SDPA_LOG_ERROR("Bad function call exception occurred!");
        return;
      }

      std::ostringstream oss;
      oss<<"rank"<<rankRight;
      requirement_t req(oss.str(), true);
      requirement_list_t reqList;
      reqList.push_back(req);

      pIAgent_->submit(pBlueToken->activityId(), pBlueToken->encode(), reqList);
    }

    void propagateDown( const Token::ptr_t& pRedToken )
    {
      lock_type lock(mtx_);
      int rank = pIAgent_->rank();

      SDPA_LOG_INFO("The number of agents is: "<<m_nTorusDim*m_nTorusDim);

      int i  = rank/m_nTorusDim;
      int j  = rank%m_nTorusDim;

      int rankBottom = ((i+1)%m_nTorusDim)*m_nTorusDim+j;

      try {
        id_type newActId = fct_id_gen_();
        m_mapActIds[newActId] = pRedToken->activityId();
        pRedToken->activityId() = newActId;
        pRedToken->incVisitedNodes();
      }
      catch(boost::bad_function_call& ex)
      {
        SDPA_LOG_ERROR("Bad function call exception occurred!");
        return;
      }

      std::ostringstream oss;
      oss<<"rank"<<rankBottom;
      requirement_t req(oss.str(), true);
      requirement_list_t reqList;
      reqList.push_back(req);

      pIAgent_->submit(pRedToken->activityId(), pRedToken->encode(), reqList);
    }

    /**
     * Cancel a workflow asynchronously.
     * This method is to be invoked by the SDPA.
     * The GWES will notifiy the SPDA about the
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

      m_thread = boost::thread(boost::bind(&TorusWorkflowEngineAgent::run, this));
      SDPA_LOG_DEBUG("EmptyWE thread started ...");
    }

    void stop()
    {
      bStopRequested = true;
      m_thread.interrupt();
      DLOG(TRACE, "EmptyWE thread before join ...");
      m_thread.join();
      DLOG(TRACE, "EmptyWE thread joined ...");
    }

    void run()
    {
      //lock_type lock(mtx_stop);
      while(!bStopRequested)
      {
        //cond_stop.wait(lock);
        Token::ptr_t pBlueToken, pRedToken;

        // consume always 2 tokens
        if(!queueBlueTokens.empty() && !queueRedTokens.empty() )
        {
          pBlueToken = queueBlueTokens.pop();
          SDPA_LOG_DEBUG("Popped-up a BLUE token ...");

          if( pBlueToken->nVisitedNodes() < m_nTorusDim )
            propagate(RIGHT, pBlueToken);

          pRedToken  = queueRedTokens.pop();
          SDPA_LOG_DEBUG("Popped-up a RED token ...");

          if( pRedToken->nVisitedNodes() < m_nTorusDim )
            propagate(DOWN, pRedToken);

          Token::ptr_t pYellowToken = mapActId2YellowTokenPtr[m_wfid];

          if( pRedToken->nVisitedNodes() == 1 && pBlueToken->nVisitedNodes() == 1 )
          {
            accumulate( pRedToken, pBlueToken, pYellowToken, true ); // C += A * B
            continue;
          }
          else if( pRedToken->owner() != pIAgent_->name() && pBlueToken->owner() != pIAgent_->name() ) // propagation finished
          {
            accumulate( pRedToken, pBlueToken, pYellowToken );
            continue;
          }

          // care master
          if( pRedToken->nVisitedNodes() == m_nTorusDim && pBlueToken->nVisitedNodes() == m_nTorusDim )
          {
            SDPA_LOG_INFO("The number of visited nodes is blue:" <<pBlueToken->nVisitedNodes()<<", red:"<<pRedToken->nVisitedNodes());

            SDPA_LOG_DEBUG("Inform the orchestrator that the job "<<m_wfid<<" finished! Product: ");
            print(pYellowToken->block_1());

            pIAgent_->finished( pBlueToken->activityId(), "");
            pIAgent_->finished( pRedToken->activityId(), "" );

            pIAgent_->finished( m_wfid, pYellowToken->encode() );

            mapActId2YellowTokenPtr.erase(m_wfid);
          }
        }
        else
          boost::this_thread::sleep(boost::posix_time::seconds(1));
      }
    }

  public:
    mutable sdpa::daemon::GenericDaemon *pIAgent_;
    static size_t m_nTorusDim;

  private:
    mutex_type mtx_;
    Function_t fct_id_gen_;

    bool bStopRequested;
    boost::thread m_thread;

    TokenQueue queueRedTokens;
    TokenQueue queueBlueTokens;
    map_actId2tokenPtr_t mapActId2YellowTokenPtr;

    id_type m_rightNghb;
    id_type m_bottomNghb;

    std::string m_wfid;

    map_act_ids_t m_mapActIds;
};


#endif //TORUS_WORKFLOW_ENGINE_AGENT_HPP
