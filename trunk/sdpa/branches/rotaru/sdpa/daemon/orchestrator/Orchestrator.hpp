/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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
#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>

namespace sdpa {
namespace daemon {
	  class Orchestrator : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<Orchestrator> ptr_t;
		SDPA_DECLARE_LOGGER();

		Orchestrator( const std::string &name = "",  const std::string& url = "",
				      const std::string &workflow_directory = "", const bool bUseDummyWE = false);

		virtual ~Orchestrator();

		static Orchestrator::ptr_t create( const std::string& name,  const std::string& url,
				                           const std::string &workflow_directory, const bool bUseDummyWE = false );

		static void start(Orchestrator::ptr_t ptrOrch);
		static void shutdown(Orchestrator::ptr_t ptrOrch);

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );
		void action_interrupt( const sdpa::events::InterruptEvent& );

		void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent( const sdpa::events::JobFailedEvent* );

		void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt );
		void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt );

		void handleRetrieveResultsEvent( const sdpa::events::RetrieveJobResultsEvent* pEvt );

		const std::string& url() const {return url_;}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int file_version )
		{
			ar & boost::serialization::base_object<DaemonFSM>(*this);
		}

		virtual void backup( const std::string& strArchiveName );
		virtual void recover( const std::string& strArchiveName );

		friend class boost::serialization::access;
		friend class sdpa::tests::WorkerSerializationTest;

	  private:
		const std::string url_;
	  };
	}
}
