/*
 * =====================================================================================
 *
 *       Filename:  Aggreagtor.hpp
 *
 *    Description:  Contains the Aggregator class
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
#include <gwes/GWES.h>

namespace sdpa {
	namespace daemon {
		class Aggregator : public dsm::DaemonFSM {
			public:
			typedef sdpa::shared_ptr<Aggregator> ptr_t;
			SDPA_DECLARE_LOGGER();

			Aggregator( const std::string& name = "", const std::string& url = "",
						const std::string& masterName = "", const std::string& masterUrl = "");

			virtual ~Aggregator();

			static Aggregator::ptr_t create( const std::string& name, const std::string& url,
											 const std::string& masterName, const std::string& masterUrl );

			static void start(Aggregator::ptr_t ptrAgg);
			static void shutdown(Aggregator::ptr_t ptrAgg);

			void action_configure( const sdpa::events::StartUpEvent& );
			void action_config_ok(const sdpa::events::ConfigOkEvent&);

			void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
			void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

			void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
			void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

			const std::string& url() const {return url_;}
			const std::string& masterName() const { return masterName_; }
			const std::string& masterUrl() const { return masterUrl_; }

			template <class Archive>
			void serialize(Archive& ar, const unsigned int file_version )
			{
				ar & boost::serialization::base_object<DaemonFSM>(*this);
			}

			friend class boost::serialization::access;
			friend class sdpa::tests::WorkerSerializationTest;

			private:
			const std::string url_;
			const std::string masterName_;
			const std::string masterUrl_;
		};
	}
}
