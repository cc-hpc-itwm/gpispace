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
  ActivityExecutor::encode(Reply *rply)
  {
    std::ostringstream sstr;
    boost::archive::text_oarchive ar(sstr);
    init_archive(ar);
    ar << rply;

    return sstr.str();
  }

  void
  ActivityExecutor::start()
  {
    if (execution_thread_) return; // already started

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

    execution_thread_ = new boost::thread(boost::ref(*this));
    barrier_.wait();
  }

  void
  ActivityExecutor::stop()
  {
    if (! execution_thread_) return; // already stopped
    execution_thread_->interrupt();
    io_service_.stop();
    execution_thread_->join();
    delete execution_thread_; execution_thread_ = NULL;
    delete socket_; socket_ = NULL;
  }

  void ActivityExecutor::operator()()
  {
    barrier_.wait();
    io_service_.run();
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
      if (msg == "SEGV")
      {
        DLOG(INFO, "got SEGV request, returning from loop...");
        int *i = 0;
        *(i) = 0;
        return;
      }
#endif

      try
      {
        Request *rqst = decode(msg);
        Reply *rply = rqst->execute(this);
        delete rqst; rqst = NULL;

        if (rply)
        {
          socket_->send_to(boost::asio::buffer(encode(rply)), sender_endpoint_);
          delete rply; rply = NULL;
        }
        else
        {
          LOG(DEBUG, "nothing to reply, assuming shutdown...");
          return;
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
