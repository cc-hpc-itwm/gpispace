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

  class NrePcdIsDead : public std::runtime_error
  {
  public:
    NrePcdIsDead()
      : std::runtime_error("nre-pcd is probably dead")
    {}

    virtual ~NrePcdIsDead() throw () {}
  };

  class WalltimeExceeded : public std::runtime_error
  {
  public:
    WalltimeExceeded()
      : std::runtime_error("execution did not finish within the specified walltime")
    {}
    virtual ~WalltimeExceeded() throw () {}
  };

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
      , timer_timeout_(5)
      , timer_active_(false)
      , timer_(io_service_)
      , ping_interval_(10) // 10 seconds
      , ping_interval_timer_(io_service_)
      , not_responded_to_ping_(0)
      , ping_trials_(3)
      , num_waiting_receiver_(0)
      , started_(false)
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

    void set_ping_interval(unsigned long seconds)
    {
      ping_interval_ = seconds;
    }
    void set_ping_timeout(unsigned long seconds)
    {
      timer_timeout_ = seconds;
    }
    void set_ping_trials(std::size_t max_tries)
    {
      ping_trials_ = max_tries;
    }

    bool is_pcd_dead() const
    {
      return not_responded_to_ping_ >= ping_trials_;
    }

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
        LOG(FATAL, "could not create my sending socket: " << ex.what());
        throw;
      }

      schedule_receive();

      service_thread_ = new boost::thread(boost::bind(&NreWorkerClient::service_thread, this));
      barrier_.wait();

      ping (); // send a synchronous ping

      started_ = true;
      not_responded_to_ping_ = 0;
      LOG(INFO, "started connection to nre-pcd");

      start_ping_interval_timer();
    }

    void stop() throw (std::exception)
    {
      if (service_thread_ == NULL)
      {
        return;
      }
      LOG(TRACE, "stopping nre-pcd connection");
      service_thread_->interrupt(); 
      io_service_.stop();
      service_thread_->join();
      DLOG(TRACE, "service thread finished");
      delete service_thread_; service_thread_ = NULL;

      if (socket_)
      {
        socket_->close();
        delete socket_; socket_ = NULL;
      }
      started_ = false;
      LOG(INFO, "connection to nre-pcd stopped.");
    }

    void cancel() throw (std::exception)
    {
      throw std::runtime_error("not implemented");
    }

    sdpa::wf::Activity execute(const sdpa::wf::Activity &in_activity, unsigned long walltime = 0) throw (WalltimeExceeded, std::exception)
    {
      sdpa::shared_ptr<Message> msg = request(ExecuteRequest(in_activity), walltime);
      if (msg)
      {
        // check if it is a ExecuteReply
        ExecuteReply *exec_reply = dynamic_cast<ExecuteReply*>(msg.get());

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
    void ping() throw (NrePcdIsDead)
    {
      try
      {
        sdpa::shared_ptr<Message> msg = request(PingRequest("tag-1"), timer_timeout_);
        LOG(TRACE, "got reply to ping: " << *msg);
      }
      catch (...)
      {
        throw NrePcdIsDead();
      }
    }

    void send(const Message &m)
    {
      boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
      std::string encoded_message(codec_.encode(m));
      DLOG(TRACE, "sending " << encoded_message.size() << " bytes of data to " << nre_worker_endpoint_ << ": " << encoded_message);
      socket_->async_send_to(boost::asio::buffer(encoded_message)
                           , nre_worker_endpoint_
                           , boost::bind(&NreWorkerClient::handle_send_to, this
                           , boost::asio::placeholders::error
                           , boost::asio::placeholders::bytes_transferred));
    }

    template <class T> struct counter_mgr
    {
      counter_mgr(T *counter)
        : counter_(counter)
      { *counter_++; }

      ~counter_mgr() { *counter_--; }

      T *counter_;
    };
    typedef counter_mgr<unsigned int> waiter_mgr;

    Message *recv(unsigned long timeout)
    {

      Message *msg(NULL);
      boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);

      waiter_mgr waiter(&num_waiting_receiver_);

      while (incoming_messages_.empty())
      {
        bool notified_(false);

        if (timeout)
        {
          const boost::system_time to(boost::get_system_time() + boost::posix_time::seconds(timeout));
          notified_ = msg_avail_.timed_wait(lock, to);
        }
        else
        {
          msg_avail_.wait(lock);
        }

        if (! notified_)
        {
          if (! incoming_messages_.empty())
          {
            break;
          }
          else
          {
            if (is_pcd_dead())
            {
              throw NrePcdIsDead();
            }
            else
            {
              throw WalltimeExceeded();
            }
          }
        }
        DLOG(TRACE, "receiver woke up...");
        if (started_ && is_pcd_dead())
        {
          throw NrePcdIsDead();
        }
      }
      msg = incoming_messages_.front();
      incoming_messages_.pop_front();

      return msg;
    }

    sdpa::shared_ptr<Message> request(const Message &m, unsigned long timeout)
    {
      send(m);
      return sdpa::shared_ptr<Message>(recv(timeout));
    }

    void schedule_receive()
    {
      socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&NreWorkerClient::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void start_timeout_timer()
    {
      if (! timer_active_)
      {
        timer_.expires_from_now(boost::posix_time::seconds(timer_timeout_));
        timer_.async_wait(boost::bind(&NreWorkerClient::timer_timedout, this, boost::asio::placeholders::error));
        DLOG(TRACE, "started timeout timer (expires in: "<< timer_.expires_from_now() <<")");
        timer_active_ = true;
      }
      else
      {
        DLOG(TRACE, "timeout timer still active (expires in: "<< timer_.expires_from_now() <<")");
      }
    }

    void start_ping_interval_timer()
    {
      DLOG(TRACE, "starting ping timer ("<<ping_interval_<<"s)");
      // schedule next ping
      ping_interval_timer_.expires_from_now(boost::posix_time::seconds(ping_interval_));
      ping_interval_timer_.async_wait(boost::bind(&NreWorkerClient::send_ping, this, boost::asio::placeholders::error));
    }

    void send_ping(const boost::system::error_code &error)
    {
      if (! error)
      {
        DLOG(DEBUG, "checking if pcd is alive...");
        send (PingRequest("tag-1"));
        // start the timeout timer
        start_timeout_timer();
        start_ping_interval_timer();
      }
      {
        DLOG(TRACE, "ping timer cancelled");
      }
    }

    void ping_reply_received(const sdpa::shared_ptr<PingReply> &ping_reply)
    {
      not_responded_to_ping_ = 0;

      DLOG(DEBUG, "got ping reply from pid=" << ping_reply->pid());
      timer_.cancel();
      DLOG(TRACE, "timeout timer cancelled");
    }

    void timer_timedout(const boost::system::error_code &error)
    {
      if (! error)
      {
        ++not_responded_to_ping_;
        DLOG(WARN, "nre-pcd is not responding... (" << not_responded_to_ping_ << "/" << ping_trials_ << ")");

        // wake up any thread waiting for some execution
        if (is_pcd_dead())
        {
          LOG(ERROR, "nre-pcd is probably dead, waking blocked threads...");
          boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
          msg_avail_.notify_all();
        }
      }
      timer_active_ = false;
    }

    void handle_send_to(const boost::system::error_code &error
                      , size_t bytes_sent)
    {
      DLOG(TRACE, "sent " << bytes_sent << " bytes of data (error_code=" << error << ")...");
    }

    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv)
    {
      if (!error && bytes_recv > 0)
      {
        std::string tmp(data_, bytes_recv);
        DLOG(TRACE, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << tmp);
        try
        {
          Message *msg(codec_.decode(tmp));
          if (started_)
          {
            if (PingReply *pong = dynamic_cast<PingReply*>(msg))
            {
              // handle internal pings
              sdpa::shared_ptr<PingReply> ping_reply(pong);
              ping_reply_received(ping_reply);
            }
            else
            {
              boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
              incoming_messages_.push_back(msg);
              msg_avail_.notify_one();
            }
          }
          else
          {
            if (dynamic_cast<PingReply*>(msg))
            {
              boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
              // notify initial start() synchronous ping request
              incoming_messages_.push_back(msg);
              msg_avail_.notify_one();
            }
            else
            {
              LOG(WARN, "ignoring message (not completely started yet): " << *msg);
            }
          }
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

    unsigned long timer_timeout_;
    bool timer_active_;
    boost::asio::deadline_timer timer_;

    unsigned long ping_interval_;
    boost::asio::deadline_timer ping_interval_timer_;
    size_t not_responded_to_ping_;
    size_t ping_trials_;

    udp::endpoint nre_worker_endpoint_;
    udp::endpoint sender_endpoint_;

    // asynchronous receive implementation
    boost::recursive_mutex msg_mtx_;
    boost::condition_variable_any msg_avail_;
    typedef std::list<Message*> message_list_t;
    message_list_t incoming_messages_;
    unsigned int num_waiting_receiver_;

    bool started_;
    bool timeout_timer_active_;

    enum { max_length = ((2<<16) - 1) };
    char data_[max_length];
  };
}}}

#endif
