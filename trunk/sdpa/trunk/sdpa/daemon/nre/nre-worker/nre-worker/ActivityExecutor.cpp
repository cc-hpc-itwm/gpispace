/*
 * =====================================================================================
 *
 *       Filename:  UDPActivityExecutor.cpp
 *
 *    Description:  implementation
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:47:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <stdexcept>
#include <csignal>
#include <cstdio>

#include <boost/asio.hpp>

#include "ActivityExecutor.hpp"

#include <we/we.hpp>

#include <we/loader/module.hpp>
#include "context.hpp"

using boost::asio::ip::udp;

static
void sig_handler(int sig)
{
	std::cout << "Lethal signal (" << sig << ") received" << " - I will do my best to save the world!" << std::endl;
	exit(15);
}

namespace sdpa { namespace nre { namespace worker {
  enum { max_length = (2<<23) };

  void
  ActivityExecutor::start()
  {
	  boost::unique_lock<boost::recursive_mutex> lock(mtx_);

	  if (service_thread_) return; // already started

	  io_service_.reset();

	  LOG(DEBUG, "opening connection on: " << location());

	  std::string host(location());
	  unsigned short port(0);

    std::string::size_type sep_pos(location().find(":"));
    if (sep_pos != std::string::npos)
    {
      host = location().substr(0, sep_pos);
      std::stringstream sstr(location().substr(sep_pos+1));
      sstr >> port;
      if (! sstr)
      {
        throw std::runtime_error("could not parse port-information from location: " + location());
      }
    }
    udp::endpoint my_endpoint(boost::asio::ip::address::from_string(host), port);

    socket_ = new udp::socket(io_service_, my_endpoint);
    udp::endpoint real_endpoint = socket_->local_endpoint();

    boost::system::error_code ec;
    socket_->set_option (boost::asio::socket_base::reuse_address (true), ec);
    LOG_IF(WARN, ec, "could not set resuse address option: " << ec << ": " << ec.message());

    socket_->set_option (boost::asio::socket_base::send_buffer_size (max_length), ec);
    LOG_IF(WARN, ec, "could not set send-buffer-size to " << max_length << ": " << ec << ": " << ec.message());

    socket_->async_receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&ActivityExecutor::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

    LOG(INFO, "listening on " << real_endpoint);

    service_thread_ = new boost::thread(boost::ref(*this));
    execution_threads_.push_back(new boost::thread(boost::bind(&ActivityExecutor::execution_thread, this)));

    barrier_.wait();
  }

  bool
  ActivityExecutor::stop()
  {
    bool all_stopped(true);

    for (thread_list_t::iterator thrd(execution_threads_.begin()); thrd != execution_threads_.end(); ++thrd)
    {
      // FIXME: how to stop the execution thread (while it executes something)?
      const std::size_t max_trials(3);
      const unsigned int timeout(1); // 1 sec
      bool stopped(false);
      for (std::size_t trial(1); trial < max_trials+1; ++trial)
      {
        DLOG(TRACE, "trying to interrupt execution thread (trial " << trial << "/" << max_trials << ")");
        (*thrd)->interrupt();
        if ((*thrd)->timed_join(boost::posix_time::seconds(timeout)))
        {
          stopped = true; break;
        }
      }

      if (stopped)
      {
        LOG(TRACE, "thread stopped");
        delete (*thrd); (*thrd) = NULL;
      }
      else
      {
        LOG(WARN, "possible memory corruption created, could not terminate blocked thread cleanly: " << (*thrd)->get_id());
        all_stopped = false;
      }
    }
    execution_threads_.clear();

    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      if (! service_thread_) return true; // already stopped

      service_thread_->interrupt();
      io_service_.stop();
      service_thread_->join();
      delete service_thread_; service_thread_ = NULL;
    }

    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      if (socket_)
      {
        delete socket_; socket_ = NULL;
      }
    }
    return all_stopped;
  }

  void ActivityExecutor::operator()()
  {
    barrier_.wait();
    io_service_.run();
  }

  void ActivityExecutor::execution_thread()
  {
    for (;;)
    {
      sdpa::shared_ptr<Message> rqst;
      udp::endpoint reply_to;

      {
        // wait for a request
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        while (requests_.empty())
        {
          try
          {
            request_avail_.wait(lock);
          }
          catch (const boost::thread_interrupted &)
          {
            DLOG(TRACE, "execution thread has been interrupted...");
            return;
          }
        }
        request_t r = requests_.front(); requests_.pop_front();
        DLOG(DEBUG, "removed execution request from queue (" << requests_.size() << " waiting)");
        rqst.reset(r.second);
        reply_to = r.first;
      } // release lock

      try
      {
        boost::this_thread::interruption_point();
      }
      catch (const boost::thread_interrupted &)
      {
        DLOG(TRACE, "execution thread has been interrupted...");
        return;
      }

      sdpa::shared_ptr<Message> rply;

      try
      {
        rply = sdpa::shared_ptr<Message> (rqst->execute(this));
      } catch (std::exception const & ex)
      {
        LOG(ERROR, "could not handle request: " << ex.what());
        FHGLOG_FLUSH();
      }

      try
      {
        boost::this_thread::interruption_point();
      }
      catch (const boost::thread_interrupted &)
      {
        DLOG(TRACE, "execution thread has been interrupted...");
        return;
      }

      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        if (! socket_)
        {
          DLOG(TRACE, "I am late with terminating, everyone else is waiting for me...");
          return;
        }
        else
        {
          if (rply)
          {
            socket_->send_to(boost::asio::buffer(codec_.encode(*rply)), reply_to);
          }
          else
          {
            LOG(ERROR, "an execution request should always return a reply!");
          }
        }
      }

    } // end for(;;)
  }

  void
  ActivityExecutor::trigger_shutdown()
  {
    LOG(DEBUG, "committing suicide...");
    // i guess this will only work on Linux:
    kill(getpid(), SIGTERM);
    // the alternative would be
    //   raise(SIGTERM);
    // but (regardless what the manpage says) it has not the exact same effect (sighandler in main is not triggered) ;-(
  }

  void
  ActivityExecutor::handle_receive_from(const boost::system::error_code &error
                                      , size_t bytes_recv)
  {
    if (!error && bytes_recv > 0)
    {
      data_[bytes_recv] = 0;
      std::string msg(data_, bytes_recv);

      DLOG(TRACE, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << msg);

      // special commands in debug build
#ifndef NDEBUG
      DLOG(TRACE, "accepting the following \"special\" commands: " << "QUIT SEGV STATUS");
      if (msg == "QUIT")
      {
        DLOG(INFO, "got QUIT request, returning from loop...");
        trigger_shutdown();
        return;
      }
      else if (msg == "SEGV")
      {
        DLOG(INFO, "got SEGV request, obeying...");
        int *segv(0);
        *segv = 0;
        return;
      }
      else if (msg == "STATUS")
      {
        DLOG(INFO, "got STATUS request...");
        DLOG(INFO, "my modules: " << loader());

        goto cont;
      }
#endif

      try
      {
        Message *rqst_ptr = codec_.decode(msg);
        if (rqst_ptr->would_block())
        {
          boost::unique_lock<boost::recursive_mutex> lock(mtx_);
          requests_.push_back(std::make_pair(sender_endpoint_, rqst_ptr));
          DLOG(DEBUG, "enqueued blocking request to execution thread (" << requests_.size() << " waiting)");
          request_avail_.notify_one();
        }
        else
        {
		  sdpa::shared_ptr<Message> rqst(rqst_ptr);
          DLOG(DEBUG, "directly executing request...");
          sdpa::shared_ptr<Message> rply(rqst->execute(this));

          if (rply)
          {
            socket_->send_to(boost::asio::buffer(codec_.encode(*rply)), sender_endpoint_);
          }
          else
          {
            LOG(DEBUG, "nothing to reply, assuming shutdown...");
            trigger_shutdown();
            return;
          }
        }
      }
      catch (const std::exception &ex)
      {
        LOG(ERROR, "could not execute the desired request: " << ex.what());
      }
      catch (...) {
        LOG(ERROR, "could not execute the desired request (unknown reason)");
      }

#ifndef NDEBUG
cont:
#endif
      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&ActivityExecutor::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      LOG(ERROR, "error during receive: " << error);
      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&ActivityExecutor::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  unsigned int ActivityExecutor::run()
  {
     	signal(SIGSEGV, &sig_handler);
     	signal(SIGABRT, &sig_handler);

     	try
     	{
     		start();
     	}
     	catch (const std::exception &ex)
     	{
          LOG(ERROR, "could not start executor: " << ex.what());
          return 4;
     	}

     	LOG(DEBUG, "waiting for signals...");
     	sigset_t waitset;
     	int sig(0);
     	int result(0);

     	sigfillset(&waitset);
     	sigprocmask(SIG_BLOCK, &waitset, NULL);

     	bool signal_ignored = true;
     	while (signal_ignored)
     	{
          FHGLOG_FLUSH();

          result = sigwait(&waitset, &sig);
          if (result == 0)
          {
            LOG(DEBUG, "got signal: " << sig);

            switch (sig)
            {
            case SIGHUP:
              loader().unload_autoloaded();
              break;
            case SIGTERM: // fall through
            case SIGKILL:
            case SIGINT:
              signal_ignored = false;
              break;
            case SIGUSR1:
            case SIGUSR2:
              DLOG(TRACE, "flushing streams");
              FHGLOG_FLUSH();
              break;
            default:
              LOG(INFO, "ignoring signal: " << sig);
              break;
            }
          }
          else
          {
            LOG(ERROR, "error while waiting for signal: " << result);
          }
     	}

     	//fvm_pc.leave();

     	LOG(INFO, "terminating...");
     	if (!stop())
     	{
     		LOG(WARN, "executor did not stop correctly...");
     	}

     	return 0;
  }


Reply* ActivityExecutor::reply(ExecuteRequest* pMsgExecReq)
{
  	Reply *reply(NULL);

  	try
  	{
  		//LOG(INFO, "executing: " << activity());
  		DLOG (TRACE, "received new activity: "<<pMsgExecReq->activity());

  		we::activity_t act(we::util::text_codec::decode<we::activity_t>(pMsgExecReq->activity()));

		LOG (INFO, "executing: " << act.transition().name() << "(" << fhg::util::show(act.input().begin(), act.input().end()) << ")" );

  		struct exec_context ctxt( loader() );
  		act.execute(ctxt);

                LOG (INFO, "finished: " << fhg::util::show (act.output().begin(), act.output().end()));

  		execution_result_t exec_res(std::make_pair(ACTIVITY_FINISHED, we::util::text_codec::encode(act)));

  		DLOG (TRACE, "creating a reply message ... ");
  		reply = new ExecuteReply(exec_res);
  	}
  	catch (const std::exception &ex)
  	{
  		LOG(ERROR, "execution of activity failed: " << ex.what());
		execution_result_t exec_res(std::make_pair(ACTIVITY_FAILED, ex.what()));
  		reply = new ExecuteReply(exec_res);
  	}
  	catch (...)
  	{
  		LOG(ERROR, "execution of activity failed!");
		execution_result_t exec_res(std::make_pair(ACTIVITY_FAILED, "unknown exception"));
  		reply = new ExecuteReply(exec_res);
  	}

  	assert(reply);
  	reply->id() = pMsgExecReq->id();
  	DLOG (TRACE, "replying with id "<<reply->id());

  	return reply;
}

Reply* ActivityExecutor::reply(InfoRequest* pMsgInfoReq)
{
	return new InfoReply(pMsgInfoReq->id(), this);
}

}}}
