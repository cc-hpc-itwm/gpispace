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
#include <sdpa/engine/Token.hpp>

using boost::numeric::ublas::matrix;

class TorusWorkflowEngineAgent : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;

    //typedef std::pair<sdpa::job_id_t, result_type>
    typedef SynchronizedQueue<std::list<Token::ptr_t> > TokenQueue;

    TorusWorkflowEngineAgent( GenericDaemon* pIAgent = NULL, Function_t f = id_gen)
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

    void connect(GenericDaemon* pIAgent )
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
    bool failed(const id_type& activityId, const result_type&  result )
    {
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
    	SDPA_LOG_DEBUG("The activity " << activityId<<" forward!");
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
		SDPA_LOG_DEBUG("The activity " << activityId<<" was cancelled!");
		return false;
    }

    Token::ptr_t genBlueToken(Token& token)
    {
    	Token::ptr_t ptrTok(new Token(BLUE, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim, token.block_1()));
    	return ptrTok;
    }

    Token::ptr_t genRedToken(Token& token)
      {
      	Token::ptr_t ptrTok(new Token(RED, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim, token.block_2()));
      	return ptrTok;
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
		// GWES is supposed to parse the workflow and generate a suite of
		// sub-workflows or activities that are sent to SDPA
		// GWES assigns an unique workflow_id which will be used as a job_id
		// on SDPA side
		SDPA_LOG_DEBUG("Submit new workflow, wfid = "<<wfid);
		//lock_type lock(mtx_);

		Token token;
		token.decode(wf_desc);

		//int nTorusDim = token.size();

		if( token.color() == YELLOW )
		{
			SDPA_LOG_DEBUG("Got YELLOW token ...");
			//print(token.block_1());
			//print(token.block_2());

			m_orch = token.owner();
			m_wfid = wfid;

			// generate one blue and one red token, respectively
			Token::ptr_t pBlueToken = genBlueToken(token);
			Token::ptr_t pRedToken  = genRedToken(token);

			accumulate( pRedToken, pBlueToken, true ); // C += A * B

			propagate(RIGHT, pBlueToken);
			propagate(DOWN, pRedToken);

			return;
		}
		else
		{
			Token::ptr_t pNewToken(new Token(token));

			if(token.color() == RED)
			{
				SDPA_LOG_DEBUG("Got RED token ...: ");
				queueRedTokens.push(pNewToken);
			}
			else
				if(token.color() == BLUE)
				{
					SDPA_LOG_DEBUG("Got BLUE token ...: ");
					queueBlueTokens.push(pNewToken);
				}
				else
					std::cout<<"Invalid token color"<<std::endl;
		}
	}

	void accumulate(const Token::ptr_t& pRedToken, const Token::ptr_t& pBlueToken, bool bClear = false )
	{
		size_t nBlockDim = pRedToken->block_1().size1();
		if(bClear)
		{
			m_product = matrix_t(nBlockDim, nBlockDim);
		}

		axpy_prod( pBlueToken->block_1(), pRedToken->block_1(), m_product, bClear); // C += A * B
	}

	void propagate(const direction_t& dir, const Token::ptr_t& pToken)
	{
		if(dir == RIGHT)
			propagateRight(pToken);
		else
			if( dir == DOWN )
				propagateDown(pToken);
			else
				SDPA_LOG_FATAL("Invalid propagation direction!");
	}


	void propagateRight(const Token::ptr_t& pBlueToken)
	{
		int rank = pIAgent_->rank();

		int i = rank / m_nTorusDim;
		int j = rank % m_nTorusDim;

		int rankRight = i*m_nTorusDim+(j+1)%m_nTorusDim;

		id_type actIdRed, actIdBlue;
		try {
			actIdBlue = fct_id_gen_();
		}
		catch(boost::bad_function_call& ex) {
			SDPA_LOG_ERROR("Bad function call exception occurred!");
			return;
		}

		pIAgent_->forward(actIdBlue, pBlueToken->encode(), rankRight );
	}

	void propagateDown(const Token::ptr_t& pRedToken)
	{
		int rank = pIAgent_->rank();

		SDPA_LOG_INFO("The number of agents is: "<<m_nTorusDim*m_nTorusDim);

		int i  = rank/m_nTorusDim;
		int j  = rank%m_nTorusDim;

		int rankBottom = ((i+1)%m_nTorusDim)*m_nTorusDim+j;

		id_type actIdRed, actIdBlue;
		try {
			actIdRed  = fct_id_gen_();
		}
		catch(boost::bad_function_call& ex) {
			SDPA_LOG_ERROR("Bad function call exception occurred!");
			return;
		}

		pIAgent_->forward( actIdRed,  pRedToken->encode(),  rankBottom );
	}

	/**
	 * Cancel a workflow asynchronously.
	 * This method is to be invoked by the SDPA.
	 * The GWES will notifiy the SPDA about the
	 * completion of the cancelling process by calling the
	 * callback method Gwes2Sdpa::cancelled.
	 */
	bool cancel(const id_type& wfid, const reason_type& reason)
	{
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

				  if( pBlueToken->owner() != pIAgent_->name() )
					  propagate(RIGHT, pBlueToken);

				  pRedToken  = queueRedTokens.pop();
				  SDPA_LOG_DEBUG("Popped-up a RED token ...");

				  if( pRedToken->owner() != pIAgent_->name() )
					  propagate(DOWN, pRedToken);

				  if( pRedToken->owner() != pIAgent_->name() && pBlueToken->owner() != pIAgent_->name() ) // propagation finished
				  {
					  accumulate(pRedToken, pBlueToken );
					  continue;
				  }

				  // care master
				  if( pRedToken->owner() == pIAgent_->name() && pBlueToken->owner() == pIAgent_->name() )
				  {
					  Token yellowToken(YELLOW, pIAgent_->name(), pIAgent_->rank(), m_nTorusDim, m_product);

					  SDPA_LOG_DEBUG("Inform the orchestrator that the job "<<m_wfid<<" finished! Product: ");
					  print(yellowToken.block_1());

					  pIAgent_->finished(m_wfid, yellowToken.encode() );
					  m_product.clear();
				  }
    		  }
    		  else
    			  boost::this_thread::sleep(boost::posix_time::seconds(1));
    	  }
      }

      bool fill_in_info ( const id_type & id, activity_information_t &) const
      {
    	  return false;
      }

  public:
    mutable GenericDaemon *pIAgent_;
    static size_t m_nTorusDim;

  private:
    mutex_type mtx_;
    Function_t fct_id_gen_;
    
    bool bStopRequested;
    boost::thread m_thread;

    TokenQueue queueRedTokens;
    TokenQueue queueBlueTokens;

    std::string m_orch;
    id_type m_rightNghb;
    id_type m_bottomNghb;

    std::string m_wfid;
    matrix_t m_product;
};


#endif //TORUS_WORKFLOW_ENGINE_AGENT_HPP
