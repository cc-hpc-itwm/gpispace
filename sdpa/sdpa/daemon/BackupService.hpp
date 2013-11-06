/*
 * =====================================================================================
 *
 *       Filename:  BackupService.hpp
 *
 *    Description:  Defines scheduler class
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
#ifndef SDPA_BACKUP_SERVICE_HPP
#define SDPA_BACKUP_SERVICE_HPP 1

#include <fhg/assert.hpp>

#include <iostream>
#include <boost/thread.hpp>
#include <sdpa/daemon/IAgent.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sstream>

namespace bfs=boost::filesystem;

namespace sdpa {
namespace daemon
{
	class BackupService
	{
	public:
		BackupService(IAgent* pHandler)
			: SDPA_INIT_LOGGER(pHandler->name() + ": BackupService")
			, m_ptrDaemon_(pHandler)
			, m_bStopRequested(false)
			, m_backupFile("")
			, m_backup_interval(1000000)
			, m_bStarted(false)
		{
      fhg_assert (m_ptrDaemon_, "daemon pointer has to exist");
    }

		~BackupService(){ stop(); }

		// thread related functions
		void start()
		{
			SDPA_LOG_INFO( "Starting the backup service ... ");
			m_thread = boost::thread( boost::bind( &BackupService::backup2string, this ));
		}

		// thread related functions
		void start(const bfs::path& backupFile)
		{
			m_backupFile = backupFile;

			SDPA_LOG_INFO( "Starting the backup service ... ");
			m_thread = boost::thread( boost::bind( &BackupService::backup2file, this ));
		}

		void stop()
		{
			if(m_bStarted)
			{
				SDPA_LOG_INFO( "Stopping the backup service ... ");
				m_bStopRequested = true;
				if(m_thread.joinable())
					m_thread.join();
			}
		}

		void backup2string()
		{
      m_bStarted = true;
			while(!m_bStopRequested)
			{
				try
				{
					DMLOG(TRACE, "Backing up daemon: " << daemon()->name());

					std::ostringstream osstr;
					daemon()->backup(osstr);
					m_strBackupDaemon = osstr.str();

					boost::this_thread::sleep(boost::posix_time::microseconds(m_backup_interval));
				}
				catch(const std::exception& ex)
				{
					SDPA_LOG_ERROR( "Exception occurred when trying to backup the daemon "<<daemon()->name());
				}
			}

			DMLOG(TRACE, "backup service stopping, performing final backup of " << daemon()->name());

			std::ostringstream osstr;
			daemon()->backup(osstr);
			// last backup
			m_strBackupDaemon = osstr.str();
		}

		void backup2file()
		{
			m_bStarted = true;
			bfs::path tmpBakFile = m_backupFile.string()+".tmp";
			while(!m_bStopRequested)
			{
				try
				{
					DMLOG(TRACE, "Backing up daemon: " << daemon()->name());

					std::ofstream ofs(tmpBakFile.string().c_str());
					daemon()->backup(ofs);
					ofs.close();

					m_strBackupDaemon = tmpBakFile.string();
					boost::filesystem::rename(tmpBakFile, m_backupFile);

					boost::this_thread::sleep(boost::posix_time::microseconds(m_backup_interval));
				}
				catch( const std::exception& ex )
				{
					SDPA_LOG_ERROR( "Exception occurred when trying to backup the daemon "<<daemon()->name());
				}
			}

			SDPA_LOG_DEBUG( "While loop finished. Backup the daemon "<<daemon()->name() );

			std::ofstream ofs(tmpBakFile.string().c_str());
			daemon()->backup(ofs);
			ofs.close();
			// last backup
			m_strBackupDaemon = tmpBakFile.string();

			boost::filesystem::rename(tmpBakFile, m_backupFile);
		}

		sdpa::daemon::IAgent* daemon() { return m_ptrDaemon_; }

		void setBackupInterval(const sdpa::util::time_type& backup_interval) { m_backup_interval = backup_interval; }

		std::string getLastBackup() const { return m_strBackupDaemon; }

	private:
		SDPA_DECLARE_LOGGER();
		sdpa::daemon::IAgent* m_ptrDaemon_;
		bool m_bStopRequested;
		boost::thread m_thread;
		std::string m_strBackupDaemon;
		mutable bfs::path m_backupFile;
		sdpa::util::time_type m_backup_interval; // in microseconds
		bool m_bStarted;
  };

}}

#endif
