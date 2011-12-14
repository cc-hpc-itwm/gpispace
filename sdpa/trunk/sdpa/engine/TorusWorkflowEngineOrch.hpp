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
#ifndef TORUS_WORKFLOW_ENGINE_ORCH_HPP
#define TORUS_WORKFLOW_ENGINE_ORCH_HPP 1
#include <sdpa/engine/Token.hpp>

namespace bnu = boost::numeric::ublas;

class TorusWorkflowEngineOrch : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;

    //typedef std::pair<sdpa::job_id_t, result_type>
    typedef SynchronizedQueue<std::list<we_result_t> > ResQueue;

    TorusWorkflowEngineOrch( GenericDaemon* pIAgent = NULL, Function_t f = id_gen, const std::string& left = "", const std::string& down = "" )
    	: SDPA_INIT_LOGGER("OrchTorusWFE")
        , m_product(3,3)
        , m_expectedProduct(3,3)
	{
    	pIAgent_ = pIAgent;
    	fct_id_gen_ = f;
    	SDPA_LOG_DEBUG("Torus workflow engine created ...");
    }

    ~TorusWorkflowEngineOrch()
   	{
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
     * Notify the GWES that an activity has finished
     * (state transition from running to finished).
     * This method is to be invoked by the SDPA.
     * This is a callback listener method to monitor activities submitted
     * to the SDPA using the method Gwes2Sdpa.submit().
    */
	bool finished(const id_type& activityId, const result_type& result )
    {
    	SDPA_LOG_DEBUG("The activity " << activityId<<" finished!");

    	// store the element i,j
    	// when the product matrix is calculated i.e. all
    	// PxP elements are calculated
    	// declare the workflow finished
    	// and notify the orchestrator

    	// update m_product
    	// encode it
    	lock_type lock(mtx_);

    	Token token;
    	token.decode(result);

    	m_nResults++;
    	int nAllRes = token.size()*token.size();

    	SDPA_LOG_DEBUG("Only "<<m_nResults<<" partial results have been computed!");
    	//print(token.block_1());

    	int i = token.rankOwner()/token.size();
    	int j = token.rankOwner()%token.size();

        m_product(i,j) = token.block_1()(0,0);

    	if(m_nResults == nAllRes )
    	{
    		int P = token.size();

    		SDPA_LOG_DEBUG("The product matrix is:");
    		print(m_product);

    		SDPA_LOG_DEBUG("Expected matrix product is: ");
    		print(m_expectedProduct);

    		std::stringstream sstr;
			boost::archive::text_oarchive ar(sstr);
			ar << m_product;

    		SDPA_LOG_DEBUG("All partial results have been computed. Inform the orchestrator that the workflow "<<m_wfid<<" finished!");
    		pIAgent_->finished(m_wfid, sstr.str());
    	}

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
    		lock_type lock(mtx_);

    		//matrix_t M = decode<matrix_t>(wf_desc);
    	 	matrix_t A, B;
			std::stringstream sstr(wf_desc);
			boost::archive::text_iarchive ar(sstr);
			ar >> A;
			ar >> B;

			int P;
			ar >> P;

    		SDPA_LOG_ERROR("Got the matrices: ");
    		print(A);
    		print(B);

    		m_expectedProduct = prod(A,B);

    		m_nResults = 0;
    		m_wfid = wfid;

    		id_type actId;


			// split the initial matrix into PxP submatrices here
			for(int i=0;i<P;i++ )
				for(int j=0;j<P;j++)
				{
					// create block[i,j]
					//matrix_t block_1 = project(A, bnu::range(i,i), bnu::range(j,j));    // the submatrix of A specified by the two index ranges r1 and r2
					//matrix_t block_2 = project(A, bnu::range(i,i), bnu::range(j,j));    // the submatrix of A specified by the two index ranges r1 and r2
					// block = project(M, s1, s2);    // the submatrix of A specified by the two index slices s1 and s2

					int k = (i+j)%P;

					matrix_t block_1(1,1);
					block_1<<=A(i,k);
					SDPA_LOG_INFO("block_1: ");
					print(block_1);

					matrix_t block_2(1,1);
					block_2<<=B(k,j);
					SDPA_LOG_INFO("block_2: ");
					print(block_2);

					try {
						actId  = fct_id_gen_();
						// care master
						// the rank doesn't matter for the orchestrator
						Token yellowToken(YELLOW, pIAgent_->name(), -1, P, block_1, block_2);

						requirement_list_t job_req_list;
						// A(i,k) and B(k,i) will be initially sent to the agent with the rank i*P+j
						
						unsigned int agentRank = i*P+j;
						//sdpa::worker_id_t agentId = pIAgent_->getWorkerId();
						SDPA_LOG_INFO("The agent with the rank "<<i*P+j<<" is "<<agentRank);

						pIAgent_->forward(actId, yellowToken.encode(), agentRank);
					}
					catch(boost::bad_function_call& ex) {
						SDPA_LOG_ERROR("Bad function call exception occurred!");
						return;
					}
				}
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

      bool fill_in_info ( const id_type & id, activity_information_t &) const
      {
    	  DLOG(TRACE, "fill_in_info (" << id << ")");
    	  return false;
      }


  public:
    mutable GenericDaemon *pIAgent_;

  private:
    mutex_type mtx_;
    Function_t fct_id_gen_;
    int m_nResults;

    std::string m_wfid;
    int* m_arrPartialRes;
    matrix_t m_product, m_expectedProduct;
};


#endif //TORUS_WORKFLOW_ENGINE_ORCH_HPP
