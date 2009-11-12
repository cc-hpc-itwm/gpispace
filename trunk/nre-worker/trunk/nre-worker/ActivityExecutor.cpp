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

#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sdpa/daemon/nre/messages.hpp>
#include <sdpa/daemon/nre/Serialization.hpp>

#include "ActivityExecutor.hpp"

using boost::asio::ip::udp;

template <class Archive>
void init_archive(Archive & ar)
{
  ar.register_type(static_cast<sdpa::nre::worker::PingRequest*>(NULL));
  ar.register_type(static_cast<sdpa::nre::worker::PingReply*>(NULL));

  ar.register_type(static_cast<sdpa::nre::worker::ExecuteRequest*>(NULL));
  ar.register_type(static_cast<sdpa::nre::worker::ExecuteReply*>(NULL));
}

namespace sdpa { namespace nre { namespace worker {
  enum { max_length = ((2<<16) - 1) };

  Request *
  ActivityExecutor::decode(const std::string &bytes)
  {
    Request *rqst(NULL);
    std::stringstream sstr(bytes);
    boost::archive::text_iarchive ar(sstr);
    init_archive(ar);
    ar >> rqst;

    return rqst;
  }

  std::string
  ActivityExecutor::encode(const Reply &rply)
  {
    std::ostringstream sstr;
    boost::archive::text_oarchive ar(sstr);
    init_archive(ar);
    const Reply *rply_ptr = &rply;
    ar << rply_ptr;

    return sstr.str();
  }

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

    socket_->async_receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&ActivityExecutor::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

    LOG(INFO, "listening on " << real_endpoint);

    service_thread_ = new boost::thread(boost::ref(*this));
    execution_thread_ = new boost::thread(boost::bind(&ActivityExecutor::execution_thread, this));
    barrier_.wait();
  }

  void
  ActivityExecutor::stop()
  {
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      if (! service_thread_) return; // already stopped
      // FIXME: how to stop the execution thread?

      service_thread_->interrupt();
      io_service_.stop();
      service_thread_->join();
      delete service_thread_; service_thread_ = NULL;
    }

    if (execution_thread_)
    {
      execution_thread_->interrupt();
      execution_thread_->join();
      delete execution_thread_; execution_thread_ = NULL;
    }

    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      if (socket_)
      {
        delete socket_; socket_ = NULL;
      }
    }
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
      sdpa::shared_ptr<Request> rqst;
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
            LOG(DEBUG, "execution thread has been interrupted...");
            return;
          }
        }
        request_t r = requests_.front(); requests_.pop_front();
        rqst.reset(r.second);
        reply_to = r.first;
      } // release lock

      try
      {
        boost::this_thread::interruption_point();
      }
      catch (const boost::thread_interrupted &)
      {
        LOG(DEBUG, "execution thread has been interrupted...");
        return;
      }

      sdpa::shared_ptr<Reply> rply(rqst->execute(this));

      try
      {
        boost::this_thread::interruption_point();
      }
      catch (const boost::thread_interrupted &)
      {
        LOG(DEBUG, "execution thread has been interrupted...");
        return;
      }

      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        if (rply)
        {
          if (socket_)
          {
            socket_->send_to(boost::asio::buffer(encode(*rply)), reply_to);
          }
        }
        else
        {
          LOG(FATAL, "an execution request should always return a reply, shutting down");
          trigger_shutdown();
          return;
        }

        if (! socket_)
        {
          // i am late with terminating, everyone else is waiting for me
          return;
        }
      }
    }
  }

  void
  ActivityExecutor::trigger_shutdown()
  {
    LOG(ERROR, "implement me (send a TERM signal to myself or something like that)");
  }

  void
  ActivityExecutor::handle_receive_from(const boost::system::error_code &error
                                      , size_t bytes_recv)
  {
    if (!error && bytes_recv > 0)
    {
      data_[bytes_recv] = 0;
      std::string msg(data_, bytes_recv);

      DLOG(DEBUG, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << msg);

      // special commands in debug build
#ifndef NDEBUG
      DLOG(DEBUG, "accepting the following \"special\" commands: " << "QUIT");
      if (msg == "QUIT")
      {
        DLOG(INFO, "got QUIT request, returning from loop...");
        return;
      }
      else if (msg == "SEGV")
      {
        DLOG(INFO, "got SEGV request, obeying...");
        int *segv(0);
        *segv = 0;
        return;
      }
#endif

      try
      {
        Request *rqst = decode(msg);
        if (rqst->would_block())
        {
          LOG(DEBUG, "enqueing blocking request to execution thread...");
          boost::unique_lock<boost::recursive_mutex> lock(mtx_);
          requests_.push_back(std::make_pair(sender_endpoint_, rqst));
          request_avail_.notify_one();
        }
        else
        {
          LOG(DEBUG, "directly executing request...");
          sdpa::shared_ptr<Reply> rply(rqst->execute(this));
          delete rqst; rqst = NULL;

          if (rply)
          {
            socket_->send_to(boost::asio::buffer(encode(*rply)), sender_endpoint_);
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
}}}
