/*
 * =====================================================================================
 *
 *       Filename:  ServiceThread.hpp
 *
 *    Description:  service thread around boost::asio::io_service
 *
 *        Version:  1.0
 *        Created:  12/10/2009 11:54:07 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_SERVICE_THREAD_HPP
#define SEDA_COMM_SERVICE_THREAD_HPP 1

#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace seda { namespace comm {
  class ServiceThread
  {
	public:
	  ServiceThread()
		: thread_(NULL)
	  {}

	  ~ServiceThread()
	  {
		try
		{
		  stop();
		}
		catch (const std::exception &ex)
		{
		  LOG(ERROR, "error during automatic stop of service-thread: " << ex.what());
		}
		catch (...)
		{
		  LOG(ERROR, "unknown error during automatic stop of service-thread!");
		}
	  }

	  void start()
	  {
		if (thread_) { return; }

		thread_ = new boost::thread(boost::bind(&ServiceThread::operator(), this));
	  }

	  void stop()
	  {
		if (! thread_) { return; }

		DLOG(TRACE, "interrupting service thread");
		thread_->interrupt();
		io_service_.stop();
		DLOG(TRACE, "joining service thread");
		thread_->join();
		DLOG(TRACE, "service thread finished");
		io_service_.reset();

		delete thread_; thread_ = NULL;
	  }

	  boost::asio::io_service &io_service() { return io_service_; }
	private:
	  void operator()()
	  {
		DLOG(TRACE, "service thread running");
		try
		{
		  io_service_.run();
		  io_service_.reset();
		}
		catch (const std::exception &ex)
		{
		  DLOG(TRACE, "interrupted");
		}
	  }

	private:
	  boost::thread *thread_;
	  boost::asio::io_service io_service_;
  };
}}

#endif
