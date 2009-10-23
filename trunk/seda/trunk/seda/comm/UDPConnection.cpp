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
    , service_thread_(NULL)
    , recv_waiting_(0)
  {
    Locator::location_t my_loc(a_locator->lookup(name()));
    host_ = my_loc.first;
    port_ = my_loc.second;
  }

  UDPConnection::UDPConnection(const Locator::ptr_t &a_locator
                             , const std::string &a_logical_name
                             , const Locator::host_t &a_host
                             , const Locator::port_t &a_port)
    : Connection()
    , locator_(a_locator)
    , logical_name_(a_logical_name)
    , host_(a_host)
    , port_(a_port)
    , io_service_()
    , sender_endpoint_()
    , socket_(NULL)
    , service_thread_(NULL)
    , recv_waiting_(0)
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
      throw std::logic_error(name() + " still running, cannot start again");
    }

    io_service_.reset();

    DLOG(DEBUG, "host = " << host() << " port = " << port());
    udp::endpoint my_endpoint(boost::asio::ip::address::from_string(host()), port());
    LOG(INFO, "starting UDPConnection(" << name() << ") on " << my_endpoint);

    socket_ = new udp::socket(io_service_, my_endpoint);
    udp::endpoint real_endpoint = socket_->local_endpoint();

    LOG(INFO, "listening on " << real_endpoint);

    socket_->async_receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&UDPConnection::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    locator_->insert(name(), real_endpoint.address().to_string(), real_endpoint.port());

    DLOG(DEBUG, "starting service thread");
    service_thread_ = new boost::thread(boost::ref(*this));
  }

  void UDPConnection::stop()
  {
    LOG(INFO, "stopping UDPConnection(" << name() << ")");
    locator_->remove(name());

    if (service_thread_)
    {
      DLOG(DEBUG, "interrupting service thread");
      service_thread_->interrupt(); 
      io_service_.stop();
      DLOG(DEBUG, "joining service thread");
      service_thread_->join();
      DLOG(DEBUG, "service thread finished");
      delete service_thread_;
      service_thread_ = NULL;
    }

    if (socket_)
    {
      socket_->close();
      delete socket_;
      socket_ = NULL;
    }
  }

  void UDPConnection::handle_receive_from(const boost::system::error_code &error
                                        , size_t bytes_recv)
  {
    if (!error && bytes_recv > 0)
    {
      DLOG(DEBUG, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << data_);

      try
      {
        seda::comm::SedaMessage msg;
        std::string data(data_, bytes_recv);
        msg.decode(data);

        {
          // update location
          locator_->insert(msg.from(), sender_endpoint_.address().to_string(), sender_endpoint_.port());
        }
        
        if (listener_list_.empty())
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
    const Locator::location_t &loc(locator_->lookup(m.to()));

    udp::resolver resolver(io_service_);
    udp::resolver::query query(udp::v4(), loc.first, "0");
    udp::endpoint dst = *resolver.resolve(query);
    dst.port(loc.second);
    LOG(DEBUG, "sending " << m.str() << " to " << dst);

    boost::system::error_code ignored_error;
    const std::string msgdata(m.encode());
    socket_->send_to(boost::asio::buffer(msgdata), dst, 0, ignored_error);
    LOG(INFO, "error = " << ignored_error);
  }

  template <typename T> struct recv_waiting_mgr {
    public:
      recv_waiting_mgr(T *count)
        : cnt(count)
      {
        *cnt++;
      }

      ~recv_waiting_mgr()
      {
        *cnt--;
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
    LOG(INFO, "thread started");
    io_service_.run();
  }
}}
