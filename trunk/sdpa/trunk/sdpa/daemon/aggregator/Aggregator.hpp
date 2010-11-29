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
#ifndef SDPA_AGGREGATOR_HPP
#define SDPA_AGGREGATOR_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/aggregator/SchedulerAgg.hpp>
#include <boost/serialization/base_object.hpp>

namespace sdpa {
	namespace daemon {
		class Aggregator : public dsm::DaemonFSM {
			public:
			typedef sdpa::shared_ptr<Aggregator > ptr_t;
			SDPA_DECLARE_LOGGER();

			Aggregator( const std::string& name = "",
						const std::string& url = "",
						const std::string& masterName = "")
			: DaemonFSM( name, NULL ),
			  SDPA_INIT_LOGGER(name),
			  url_(url),
			  masterName_(masterName)
			{
				SDPA_LOG_DEBUG("Aggregator constructor called ...");
				//ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new sdpa::daemon::SchedulerAgg(this));
			}


			virtual ~Aggregator();

			static void start(ptr_t ptrAgg);
			static void shutdown(ptr_t ptrAgg);

			void action_configure( const sdpa::events::StartUpEvent& );
			void action_config_ok( const sdpa::events::ConfigOkEvent& );
			void action_interrupt( const sdpa::events::InterruptEvent& );

			void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
			void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

			void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
			void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

			const std::string& url() const {return url_;}
			const std::string& masterName() const { return masterName_; }
			//const std::string& masterUrl() const { return masterUrl_; }

			template <class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar & boost::serialization::base_object<DaemonFSM>(*this);
				ar & url_; //boost::serialization::make_nvp("url_", url_);
				ar & masterName_; //boost::serialization::make_nvp("url_", masterName_);
			}

			virtual void backup( const std::string& );
			virtual void recover( const std::string& );

			friend class boost::serialization::access;
			friend class sdpa::tests::WorkerSerializationTest;

			private:
			Scheduler* create_scheduler()
			{
				DLOG(TRACE, "creating aggregator scheduler...");
				return new SchedulerAgg(this);
			}

			std::string url_;
			std::string masterName_;
		};
	}
}

#include <sdpa/daemon/aggregator/Aggregator.cpp>

#endif
