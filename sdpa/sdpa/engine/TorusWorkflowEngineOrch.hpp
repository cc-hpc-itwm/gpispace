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
#include <boost/numeric/ublas/storage.hpp>

using namespace boost::numeric::ublas;

class TorusWorkflowEngineOrch : public IWorkflowEngine {
  private:
    SDPA_DECLARE_LOGGER();

  public:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef std::string internal_id_type;

    //typedef std::pair<sdpa::job_id_t, result_type>
    typedef SynchronizedQueue<std::list<we_result_t> > ResQueue;

    TorusWorkflowEngineOrch( GenericDaemon* pIAgent = NULL, Function_t f = id_gen )
    : SDPA_INIT_LOGGER("OrchTorusWFE")
    {
      pIAgent_ = pIAgent;
      fct_id_gen_ = f;
      SDPA_LOG_INFO("Torus workflow engine created ...");
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
    bool failed( const id_type & activityId
               , const result_type & result
               , const int error_code
               , const std::string & reason
               )
    {
      SDPA_LOG_INFO("The activity " << activityId<<" failed!");
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
      SDPA_LOG_INFO("The activity " << activityId<<" finished!");

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

      size_t nBlockDim = token.block_1().size1();

      if(!m_nResults)
        m_product = matrix_t(m_nTorusDim*nBlockDim, m_nTorusDim*nBlockDim);

      m_nResults++;
      int nAllResults = m_nTorusDim*m_nTorusDim;

      int i = token.rankOwner() / m_nTorusDim;
      int j = token.rankOwner() % m_nTorusDim;

      project( m_product, range(i*nBlockDim,(i+1)*nBlockDim), range(j*nBlockDim,(j+1)*nBlockDim) ) = token.block_1();

      if( m_nResults == nAllResults )
      {
        SDPA_LOG_INFO("All partial results have been computed!");
        SDPA_LOG_INFO("------------------------------------------");
        SDPA_LOG_INFO("The product matrix is:");
        print(m_product);
        SDPA_LOG_INFO("------------------------------------------");
        SDPA_LOG_INFO("Expected matrix product is: ");
        print(m_expectedProduct);
        SDPA_LOG_INFO("------------------------------------------");

        std::stringstream sstr;
        boost::archive::text_oarchive ar(sstr);
        ar << (matrix_t const&)(m_product);

        SDPA_LOG_INFO("All partial results have been computed. Inform the orchestrator that the workflow "<<m_wfid<<" finished!");
        pIAgent_->finished(m_wfid, sstr.str());
      }
      else
      {
        SDPA_LOG_INFO("Only "<<m_nResults<<" partial results have been computed!");
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
      SDPA_LOG_INFO("The activity " << activityId<<" was cancelled!");
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
    SDPA_LOG_INFO("Submit new workflow, wfid = "<<wfid);
    lock_type lock(mtx_);

    matrix_t A, B;
    std::stringstream sstr(wf_desc);
    boost::archive::text_iarchive ar(sstr);
    ar >> A;
    ar >> B;

    m_product = matrix_t(A.size1(), A.size1());
    m_expectedProduct = matrix_t(A.size1(), A.size1());

    //ar >> m_nTorusDim;

    SDPA_LOG_ERROR("Got the matrices: ");
    print(A);
    print(B);

    m_expectedProduct = matrix_t(A.size1(), A.size2());
    axpy_prod(A, B, m_expectedProduct, true);  // C = A * B
    SDPA_LOG_INFO("Expected product is: ");
    print(m_expectedProduct);

    m_nResults = 0;
    m_wfid = wfid;

    id_type actId;
    size_t nBlockDim = A.size1()/m_nTorusDim;

    // split the initial matrix into PxP submatrices here
    for(size_t i=0;i<m_nTorusDim;i++ )
      for(size_t j=0;j<m_nTorusDim;j++)
      {
        int k = (i+j)%m_nTorusDim;

        matrix_t block_1 = project(A, range(i*nBlockDim, (i+1)*nBlockDim), range(k*nBlockDim, (k+1)*nBlockDim));
        SDPA_LOG_INFO("block_1: ");
        print(block_1);

        matrix_t block_2 = project(B, range(k*nBlockDim, (k+1)*nBlockDim), range(j*nBlockDim, (j+1)*nBlockDim));
        SDPA_LOG_INFO("block_2: ");
        print(block_2);

        try {
          // care master
          // the rank doesn't matter for the orchestrator
          Token yellowToken(YELLOW, pIAgent_->name(), -1, m_nTorusDim, block_1, block_2);
          yellowToken.activityId() = fct_id_gen_();

          unsigned int agentRank = i*m_nTorusDim+j;
          SDPA_LOG_INFO("The agent with the rank "<<i*m_nTorusDim+j<<" is "<<agentRank);

          std::ostringstream oss;
          oss<<"rank"<<agentRank;
          requirement_t req(oss.str(), true);
          requirement_list_t reqList;
          reqList.push_back(req);

          pIAgent_->submit(yellowToken.activityId(), yellowToken.encode(), reqList);
        }
        catch(boost::bad_function_call& ex)
        {
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
      SDPA_LOG_INFO("Called cancel workflow, wfid = "<<wfid);
      return true;
    }

  public:
    mutable GenericDaemon *pIAgent_;
    static size_t m_nTorusDim;

  private:
    mutex_type mtx_;
    Function_t fct_id_gen_;
    int m_nResults;

    std::string m_wfid;
    matrix_t m_product, m_expectedProduct;
};


#endif //TORUS_WORKFLOW_ENGINE_ORCH_HPP
