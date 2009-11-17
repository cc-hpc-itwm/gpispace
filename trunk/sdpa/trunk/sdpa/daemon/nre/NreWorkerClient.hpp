/*
 * =====================================================================================
 *
 *       Filename:  NreWorkerClient.hpp
 *
 *    Description:  client side for the nre-worker
 *
 *        Version:  1.0
 *        Created:  11/12/2009 11:56:59 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_NRE_WORKER_CLIENT_HPP
#define SDPA_DAEMON_NRE_WORKER_CLIENT_HPP 1

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#include <list>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fhglog/fhglog.hpp>

#include <sdpa/daemon/nre/messages.hpp>
#include <sdpa/daemon/nre/Codec.hpp>

namespace sdpa { namespace nre { namespace worker {
  using boost::asio::ip::udp;

  class NreWorkerClient
  {
  public:
    explicit
    NreWorkerClient(const std::string &nre_worker_location, unsigned short my_port=0)
      : nre_worker_location_(nre_worker_location)
      , my_reply_port_(my_port)
      , barrier_(2)
      , service_thread_(NULL)
      , socket_(NULL)
    { }

    ~NreWorkerClient() throw ()
    {
      try
      {
        stop();
      }
      catch (const std::exception &ex)
      {
        LOG(ERROR, "stopping of nre-pcd connection failed: " << ex.what());
      }
      catch (...)
      {
        LOG(ERROR, "stopping of nre-pcd connection failed (unknown reason)");
      }
    }

    const std::string &worker_location() const { return nre_worker_location_; }

    void start() throw (std::exception)
    {
      if (service_thread_)
      {
        DLOG(WARN, "still running, cannot start again!");
        return;
      }
      DLOG(INFO, "starting connection to nre-worker process at: " << worker_location());

      io_service_.reset();

      {
        std::string worker_host(worker_location());
        unsigned short worker_port(8000); // default port

        std::string::size_type sep_pos(worker_location().find(":"));
        if (sep_pos != std::string::npos)
        {
          worker_host = worker_location().substr(0, sep_pos);
          std::stringstream sstr(worker_location().substr(sep_pos+1));
          sstr >> worker_port;
          if (! sstr)
          {
            throw std::runtime_error("could not parse port-information from location: " + worker_location());
          }
        }
        udp::resolver resolver(io_service_);
        udp::resolver::query query(udp::v4(), worker_host.c_str(), "0");
        nre_worker_endpoint_ = *resolver.resolve(query);
        nre_worker_endpoint_.port(worker_port);
      }

      LOG(INFO, "connecting to nre-pcd process at: " << nre_worker_endpoint_);

      try
      {
        socket_ = new udp::socket(io_service_, udp::endpoint(udp::v4(), my_reply_port_));
      }
      catch (const std::exception &ex)
      {
        LOG(ERROR, "could not create my sending socket: " << ex.what());
        throw;
      }

      schedule_receive();

      service_thread_ = new boost::thread(boost::bind(&NreWorkerClient::service_thread, this));
      barrier_.wait();

      ping();

      LOG(INFO, "started connection to nre-pcd");
    }

    void stop() throw (std::exception)
    {
      if (service_thread_ == NULL)
      {
        return;
      }
      LOG(DEBUG, "stopping nre-pcd connection");
      service_thread_->interrupt(); 
      io_service_.stop();
      service_thread_->join();
      DLOG(DEBUG, "service thread finished");
      delete service_thread_; service_thread_ = NULL;

      if (socket_)
      {
        socket_->close();
        delete socket_; socket_ = NULL;
      }
      LOG(DEBUG, "connection to nre-pcd stopped.");
    }

    void cancel() throw (std::exception)
    {
      throw std::runtime_error("not implemented");
    }

    void ping(/* timeout */)
    {
      sdpa::shared_ptr<Message> msg = request(PingRequest("tag-1"));
      LOG(DEBUG, "got reply to ping: " << *msg);
    }

    sdpa::wf::Activity execute(const sdpa::wf::Activity &in_activity)
    {
      sdpa::shared_ptr<Message> msg = request(ExecuteRequest(in_activity));
      if (msg)
      {
        // check if it is a ExecuteReply
        ExecuteReply *exec_reply = dynamic_cast<ExecuteReply*>(msg.get());

        ping();

        if (exec_reply)
        {
          return exec_reply->result();
        }
        else
        {
          throw std::runtime_error("did not receive an ExecuteReply message!");
        }
      }
      else
      {
        throw std::runtime_error("did not get a response from worker!");
      }
    }
  private:
    sdpa::shared_ptr<Message> request(const Message &m)
    {
      // send
      {
        boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
        std::string encoded_message(codec_.encode(m));
        LOG(DEBUG, "sending " << encoded_message.size() << " bytes of data to " << nre_worker_endpoint_ << ": " << encoded_message);
        socket_->send_to(boost::asio::buffer(encoded_message), nre_worker_endpoint_);
      }

      // recv
      Message *msg(NULL);
      {
        boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
        while (incoming_messages_.empty())
        {
          const boost::system_time to(boost::get_system_time() + boost::posix_time::milliseconds(5000));
          if (! msg_avail_.timed_wait(lock, to))
          {
            if (! incoming_messages_.empty())
            {
              break;
            }
            else
            {
              throw std::runtime_error("did not receive a message (timeout)");
            }
          }
        }

        msg = incoming_messages_.front();
        incoming_messages_.pop_front();
      }

      return sdpa::shared_ptr<Message>(msg);
    }

    void schedule_receive()
    {
      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&NreWorkerClient::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv)
    {
      if (!error && bytes_recv > 0)
      {
        std::string tmp(data_, bytes_recv);
        DLOG(DEBUG, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << tmp);
        try
        {
          Message *msg(codec_.decode(tmp));
          boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
          incoming_messages_.push_back(msg);
          msg_avail_.notify_one();
        } catch (const std::exception &ex) {
          LOG(ERROR, "could not decode message: " << ex.what());
        } catch (...) {
          LOG(ERROR, "could not decode message due to an unknwon reason");
        }
      }
      else
      {
        LOG(ERROR, "error during receive: " << error);
      }

      schedule_receive();
    }

    void service_thread()
    {
      LOG(DEBUG, "thread started");
      barrier_.wait();
      io_service_.run();
    }

    std::string nre_worker_location_;
    unsigned short my_reply_port_;
    Codec codec_;

    // boost
    boost::asio::io_service io_service_;
    boost::barrier barrier_;
    boost::thread *service_thread_;
    udp::socket *socket_;
    udp::endpoint nre_worker_endpoint_;
    udp::endpoint sender_endpoint_;

    // asynchronous receive implementation
    boost::recursive_mutex msg_mtx_;
    boost::condition_variable_any msg_avail_;

    typedef std::list<Message*> message_list_t;
    message_list_t incoming_messages_;

    enum { max_length = ((2<<16) - 1) };
    char data_[max_length];
  };
}}}

#endif
