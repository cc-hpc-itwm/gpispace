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
//#include <boost/serialization/base_object.hpp>

#include <sdpa/daemon/NotificationService.hpp>

namespace sdpa {
	namespace daemon {
		class Aggregator : public dsm::DaemonFSM
		{
                  public:
                  typedef sdpa::shared_ptr<Aggregator > ptr_t;
                  SDPA_DECLARE_LOGGER();

                  Aggregator( const std::string& name = "",
                              const std::string& url = "",
                              const sdpa::master_list_t& arrMasterNames = std::vector<std::string>(),
                              unsigned int cap = MAX_CAPACITY,
                              const std::string& guiUrl = "" )
                  : DaemonFSM( name, arrMasterNames, cap, NULL ),
                    SDPA_INIT_LOGGER(name),
                    url_(url),
                    //masterName_(masterName),
                    m_guiService("SDPA", guiUrl)
                  {
                    SDPA_LOG_DEBUG("Aggregator's constructor called ...");
                    //ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new sdpa::daemon::SchedulerAgg(this));

                    // application gui service
                    if(!guiUrl.empty())
                    {
                      m_guiService.open ();
                      // attach gui observer
                      SDPA_LOG_INFO("Application GUI service at " << guiUrl << " attached...");
                    }
                  }

                  virtual ~Aggregator();

                  void action_configure( const sdpa::events::StartUpEvent& );
                  void action_config_ok( const sdpa::events::ConfigOkEvent& );

                  void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
                  void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

                  void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
                  void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

                  void cancelNotRunning (sdpa::job_id_t const & job);

                  const std::string url() const {return url_;}

                  template <class Archive>
                  void serialize(Archive& ar, const unsigned int)
                  {
                    ar & boost::serialization::base_object<DaemonFSM>(*this);
                    ar & url_; //boost::serialization::make_nvp("url_", url_);
                  }

                  virtual void backup( std::ostream& );
                  virtual void recover( std::istream& );

                  friend class boost::serialization::access;
                  //friend class sdpa::tests::WorkerSerializationTest;

                  void notifyAppGui(const result_type & result);

                  private:
                  Scheduler* create_scheduler()
                  {
                    DLOG(TRACE, "creating aggregator scheduler...");
                    return new SchedulerAgg(this);
                  }

                  std::string url_;
                  //std::string masterName_;

                  NotificationService m_guiService;
		};
	}
}

#include <sdpa/daemon/aggregator/Aggregator.cpp>

#endif
