/*
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

/*
 * =====================================================================================
 *
 *       Filename:  UDPConnection.cpp
 *
 *    Description:  implementation of the udp-connection
 *
 *        Version:  1.0
 *        Created:  10/23/2009 01:16:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "UDPConnection.hpp"
#include <fhglog/fhglog.hpp>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "Serialization.hpp"

using boost::asio::ip::udp;

namespace seda { namespace comm {
  UDPConnection::UDPConnection(const Locator::ptr_t &a_locator
                             , const std::string &a_logical_name)
    : Connection()
    , locator_(a_locator)
    , logical_name_(a_logical_name)
    , host_()
    , port_()
    , io_service_()
    , sender_endpoint_()
    , socket_(NULL)
    , recv_waiting_(0)
    , service_thread_(NULL)
  {
    Locator::location_t my_loc(a_locator->lookup(name()));
    host_ = my_loc.host();
    port_ = my_loc.port();
  }

  UDPConnection::UDPConnection(const Locator::ptr_t &a_locator
                             , const std::string &a_logical_name
                             , const Locator::location_t::host_t &a_host
                             , const Locator::location_t::port_t &a_port)
    : Connection()
    , locator_(a_locator)
    , logical_name_(a_logical_name)
    , host_(a_host)
    , port_(a_port)
    , io_service_()
    , sender_endpoint_()
    , socket_(NULL)
    , recv_waiting_(0)
    , service_thread_(NULL)
  {}

  UDPConnection::~UDPConnection()
  {
    try
    {
      stop();
    }
    catch (const std::exception &ex)
    {
      LOG(ERROR, "exception during connection destructor: " << ex.what());
    }
    catch (...)
    {
      LOG(ERROR, "unknown exception during connection destructor");
    }
  }

  void UDPConnection::start()
  {
    if (service_thread_)
    {
      LOG(DEBUG, "still running, cannot start again");
      return;
    }

    io_service_.reset();

    DLOG(TRACE, "host = " << host() << " port = " << port());
    boost::system::error_code ec;
    udp::endpoint my_endpoint(boost::asio::ip::address::from_string(host(), ec), port());
    LOG_IF(ERROR, ec, "could not parse ip address: " << ec << ": " << ec.message());
    if (ec)
    {
      throw ec;
    }
    DLOG(TRACE, "starting UDPConnection(" << name() << ") on " << my_endpoint);

    socket_ = new udp::socket(io_service_);
    socket_->open (my_endpoint.protocol());

    socket_->set_option (boost::asio::socket_base::reuse_address (true), ec);
    LOG_IF(WARN, ec, "could not set resuse address option: " << ec << ": " << ec.message());

    socket_->set_option (boost::asio::socket_base::send_buffer_size (max_length), ec);
    LOG_IF(WARN, ec, "could not set send-buffer-size to " << max_length << ": " << ec << ": " << ec.message());

    socket_->bind (my_endpoint);
    udp::endpoint real_endpoint = socket_->local_endpoint();

    socket_->async_receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&UDPConnection::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    locator_->insert(name(), real_endpoint.address().to_string(), real_endpoint.port());

    DLOG(TRACE, "starting service thread");
    service_thread_ = new boost::thread(boost::ref(*this));
    LOG(INFO, "started UDPConnection(" << name() << ") on " << real_endpoint);
  }

  void UDPConnection::stop()
  {
    if (service_thread_)
    {
      DLOG(DEBUG, "stopping UDPConnection(" << name() << ")");
      locator_->remove(name());

      DLOG(TRACE, "interrupting service thread");
      service_thread_->interrupt();
      io_service_.stop();
      DLOG(TRACE, "joining service thread");
      service_thread_->join();
      DLOG(TRACE, "service thread finished");
      delete service_thread_;
      service_thread_ = NULL;

      if (socket_)
      {
        socket_->close();
        delete socket_;
        socket_ = NULL;
      }
      LOG(INFO, "stopped UDPConnection(" << name() << ")");
    }
  }

  void UDPConnection::handle_receive_from(const boost::system::error_code &error
                                        , size_t bytes_recv)
  {
    if (!error && bytes_recv > 0)
    {
      data_[bytes_recv] = 0;
      std::string tmp(data_, bytes_recv);

      DLOG(TRACE, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << data_);

      try
      {
        // handle compression

        std::stringstream compressed_sstr(tmp);
        std::stringstream decompressed_sstr;

        namespace io = boost::iostreams;

        io::filtering_streambuf<io::input>
          in;
        in.push (io::zlib_decompressor());
        in.push (compressed_sstr);
        io::copy (in, decompressed_sstr);

        boost::archive::text_iarchive ia(decompressed_sstr);



        seda::comm::SedaMessage msg;
        ia >> msg;

        {
          // update location
          locator_->insert(msg.from(), sender_endpoint_.address().to_string(), sender_endpoint_.port());
        }

        if (! has_listeners())
        {
          boost::unique_lock<boost::recursive_mutex> lock(mtx_);
          incoming_messages_.push_back(msg);
          recv_cond_.notify_one();
        }
        else
        {
          if (recv_waiting_)
          {
            boost::unique_lock<boost::recursive_mutex> lock(mtx_);
            incoming_messages_.push_back(msg);
            recv_cond_.notify_one();
          }
          else
          {
            notifyListener(msg);
          }
        }
      } catch (const std::exception &ex) {
        LOG(ERROR, "could not decode message: " << ex.what());
      } catch (...) {
        LOG(ERROR, "could not decode message due to an unknwon reason");
      }

      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&UDPConnection::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      LOG(ERROR, "error during receive: " << error);
      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&UDPConnection::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  void UDPConnection::send(const seda::comm::SedaMessage &m)
  {
#ifndef NDEBUG
    if (service_thread_ == NULL)
    {
      LOG(ERROR, "the connection has not been started!");
      throw std::runtime_error("Connection not started!");
    }
    /*
    DLOG_CHECK( service_thread_ == NULL
              , std::runtime_error
              , "the connection has not been started!"
              );
    */
#endif

    const Locator::location_t &loc(locator_->lookup(m.to()));

    udp::resolver resolver(io_service_);
    udp::resolver::query query(udp::v4(), loc.host(), "0");

    udp::resolver::iterator endpoint_iter = resolver.resolve(query);
    if (endpoint_iter == udp::resolver::iterator())
      throw std::runtime_error ("could not resolve host name: " + loc.host());

    udp::endpoint dst = *endpoint_iter;
    dst.port(loc.port());
    DLOG(TRACE, "sending " << m.str() << " to " << dst);

    std::stringstream decompressed_sstr;
    boost::archive::text_oarchive oa(decompressed_sstr);
    oa << m;

    // handle compression
    std::stringstream compressed_sstr;

    namespace io = boost::iostreams;
    io::filtering_streambuf<io::input> in;
    in.push (io::zlib_compressor());
    in.push (decompressed_sstr);
    io::copy (in, compressed_sstr);

    const std::string msg_to_send (compressed_sstr.str());

    DLOG(TRACE, "going to send " << msg_to_send.size() << " bytes of data: " << msg_to_send);
    boost::system::error_code ec;
    std::size_t bytes_sent
      (socket_->send_to( boost::asio::buffer(msg_to_send, msg_to_send.size())
		       , dst
		       , 0
		       , ec
		       )
      );

     LOG_IF(ERROR, bytes_sent != msg_to_send.size(), "not all data could be sent: " << bytes_sent << "/" << msg_to_send.size() << " max: " << max_length);
     if (ec.value() != boost::system::errc::success)
     {
       LOG(ERROR, "could not deliver message: " << ec << ": " << ec.message());
       throw boost::system::system_error (ec); // may not be reached depending on the FATAL behaviour
     }
  }

  void UDPConnection::handle_send_to(const boost::system::error_code &error
				    , size_t
				    )
  {
    LOG_IF(ERROR, error, "not all data could be sent: " << error << ": " << error.message());
  }

  template <typename T> struct recv_waiting_mgr {
    public:
      recv_waiting_mgr(T *count)
        : cnt(count)
      {
        *cnt += 1;
      }

      ~recv_waiting_mgr()
      {
        *cnt -= 1;
      }
    private:
      T *cnt;
  };

  bool UDPConnection::recv(SedaMessage &msg, const bool block) throw(boost::thread_interrupted)
  {
    boost::unique_lock<boost::recursive_mutex> lock(mtx_);

    recv_waiting_mgr<std::size_t> waiting_mgr(&recv_waiting_);

    if (block)
    {
      while (incoming_messages_.empty())
      {
        recv_cond_.wait(lock);
      }

      msg = incoming_messages_.front();
      incoming_messages_.pop_front();
      return true;
    }
    else
    {
      if (incoming_messages_.empty())
      {
        return false;
      }
      else
      {
        msg = incoming_messages_.front();
        incoming_messages_.pop_front();
        return true;
      }
    }
  }

  void UDPConnection::operator()()
  {
    DLOG(TRACE, "thread started");
    io_service_.run();
  }
}}
