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
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fhglog/fhglog.hpp>

#include <sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.hpp>
#include <sdpa/daemon/nre/nre-worker/messages.hpp>
#include <sdpa/daemon/nre/nre-worker/Codec.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

#include <sstream>
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>

#include <sdpa/daemon/nre/BasicWorkerClient.hpp>


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

  class NreWorkerClient : public BasicWorkerClient
  {
  public:
    explicit
    NreWorkerClient( const std::string &nre_worker_location
                   // TODO: fixme, this is ugly
                   , bool bLaunchNrePcd = false
                   , const std::string & fvmPCBinary = ""
                   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
                   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
                   )
      :  BasicWorkerClient(nre_worker_location, bLaunchNrePcd, fvmPCBinary, fvmPCSearchPath, fvmPCPreLoad )
      , nre_worker_location_(nre_worker_location)
      , my_reply_port_(0)
      , service_thread_(NULL)
      , socket_(NULL)
      , timer_timeout_(3)
      , timer_active_(false)
      , timer_(io_service_)
      , ping_interval_(1) // 1 seconds
      , ping_interval_timer_(io_service_)
      , not_responded_to_ping_(0)
      , ping_trials_(3)
      , num_waiting_receiver_(0)
      , started_(false)
      , nre_pcd_do_exec_(bLaunchNrePcd)
      , nre_pcd_binary_(fvmPCBinary)
      , nre_pcd_search_path_(fvmPCSearchPath)
      , nre_pcd_pre_load_(fvmPCPreLoad)
      , pidPcd(-1)
    { }

    ~NreWorkerClient()
    {
      DLOG(TRACE, "destructing NreWorkerClient");
		try {
			stop();
		}
		catch (const std::exception &ex) {
			LOG(ERROR, "stopping of nre-pcd connection failed: " << ex.what());
		}
		catch (...) {
			LOG(ERROR, "stopping of nre-pcd connection failed (unknown reason)");
		}
    }

    const std::string &worker_location() const { return nre_worker_location_; }
    void set_location(const std::string &str_loc){ nre_worker_location_ = str_loc; }

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

    unsigned int startNrePcd( ) throw (std::exception)
    {
    	unsigned int rc = 0;

       	// check here whether the nre-pcd is running or not

		if( nre_pcd_do_exec_ )
       	{
       		LOG(INFO, "Try to spawn nre-pcd with fork!");
       		pidPcd = fork();

			if (pidPcd == 0)  // child
			{
				// Code only executed by child process
				LOG(INFO, "After fork, I'm the child process ...");
				try {
					std::vector<std::string> cmdline;
					cmdline.push_back ( nre_pcd_binary_ );

					//LOG(INFO, std::string("nre_pcd_binary_ = ") + cmdline[0] );

					cmdline.push_back("-l");
					cmdline.push_back(worker_location().c_str());

					for ( std::vector<std::string>::const_iterator it (nre_pcd_search_path_.begin())
					  ; it != nre_pcd_search_path_.end()
					  ; ++it
					  )
					{
						cmdline.push_back("--append-search-path");
						cmdline.push_back(*it);
					}

					for ( std::vector<std::string>::const_iterator it (nre_pcd_pre_load_.begin())
					  ; it != nre_pcd_pre_load_.end()
					  ; ++it
					  )
					{
						cmdline.push_back("--load");
						cmdline.push_back(*it);
					}

					char ** av = new char*[cmdline.size()+1];
					av[cmdline.size()] = (char*)(NULL);

					std::size_t idx (0);
					for ( std::vector<std::string>::const_iterator it (cmdline.begin())
					  ; it != cmdline.end()
					  ; ++it, ++idx
					  )
					{
						//LOG(INFO, std::string("cmdline[")<<idx<<"]=" << cmdline[idx] );

						av[idx] = new char[it->size()+1];
						memcpy(av[idx], it->c_str(), it->size());
						av[idx][it->size()] = (char)0;
					}

					std::stringstream sstr_cmd;
					for(size_t k=0; k<idx; k++)
						sstr_cmd << av[idx];

					LOG(DEBUG, std::string("Try to launch the nre-pcd with the following the command line:\n") + sstr_cmd.str() );

					if ( execv( nre_pcd_binary_.c_str(), av) < 0 )
					{
						throw std::runtime_error( std::string("could not exec command line ") + sstr_cmd.str() );
					}
					// not reached
				}
				catch(const std::exception& ex)
				{
					LOG(ERROR, "Exception occurred when trying to spawn nre-pcd: "<<ex.what());
					exit(1);
				}
			}
			else if (pidPcd < 0)            // failed to fork
			{
				LOG(ERROR, "Failed to fork!");
                throw std::runtime_error ("fork failed: " + std::string(strerror(errno)));
			}
			else  // parent
			{
				// Code only executed by parent process
				boost::this_thread::sleep(boost::posix_time::seconds(2));
			}
       	}
        else
        {
          throw std::runtime_error ("automatic start of process-container disabled");
        }

	return rc;
    }


    void shutdownNrePcd()
    {
    	int c;
    	int nStatus;
		if (pidPcd <= 1)
		{
			LOG(ERROR, "cannot send TERM signal to child with pid: " << pidPcd);
			throw std::runtime_error ("pcd does not have a valid pid (<= 1)");
		}

		kill(pidPcd, SIGTERM);

		c = wait(&nStatus);
		if( WIFEXITED(nStatus) )
		{
			if( WEXITSTATUS(nStatus) != 0 )
			{
				std::cerr<<"nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus)<<endl;
				LOG(ERROR, "nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus));
			}
			else
				if( WIFSIGNALED(nStatus) )
				{
					std::cerr<<"nre-pcd exited due to a signal: " <<(int)WTERMSIG(nStatus)<<endl;
					LOG(ERROR, "nre-pcd exited due to a signal: "<<(int)WTERMSIG(nStatus));
				}
		}
    }


    unsigned int start( ) throw (std::exception)
    {
    	// start first the nre-pcd!!!! if bSpawnNrepcd is set
		if(service_thread_)
		{
			DLOG(WARN, "still running, cannot start again!");
			throw std::runtime_error( "Service still running, cannot start again!");
		}

		DLOG(INFO, "starting connection to nre-worker process at: " << worker_location());

		io_service_.reset();

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
                // FIXME: this is a possible segfault!
                udp::resolver::iterator endpoint_iter = resolver.resolve(query);
                if (endpoint_iter == udp::resolver::iterator())
                {
                  throw std::runtime_error ("could not resolve: " + worker_host);
                }
		nre_worker_endpoint_ = *resolver.resolve(query);
		nre_worker_endpoint_.port(worker_port);

		LOG(INFO, "connecting to nre-pcd process at: " << nre_worker_endpoint_);

		try {
			socket_ = new udp::socket(io_service_, udp::endpoint(udp::v4(), my_reply_port_));

			boost::system::error_code ec;
			socket_->set_option (boost::asio::socket_base::reuse_address (true), ec);
			LOG_IF(WARN, ec, "could not set resuse address option: " << ec << ": " << ec.message());

			socket_->set_option (boost::asio::socket_base::send_buffer_size (max_length), ec);
			LOG_IF(WARN, ec, "could not set send-buffer-size to " << max_length << ": " << ec << ": " << ec.message());
		}
		catch (const std::exception &ex) {
			LOG(FATAL, "could not create my sending socket: " << ex.what());
			throw;
		}

		schedule_receive();

		service_thread_ = new boost::thread(boost::bind(&NreWorkerClient::service_thread, this));

	    LOG(DEBUG, "Send a synchronous ping ... ");

            std::size_t trials (10);
            while (trials --> 0)
            {
              try
              {
                ping();
              }
              catch (std::exception const & ex)
              {
                if (trials)
                {
                  LOG(WARN, "no process container found, retrying...");
                  usleep (1 * 1000 * 1000);
                }
                else
                {
                  if (nre_pcd_do_exec_)
                  {
                    try
                    {
                      startNrePcd();
                    }
                    catch (std::exception const & ex)
                    {
                      LOG(ERROR, "no process container found and starting one failed: " << ex.what());
                      throw;
                    }
                  }
                  else
                  {
                    throw;
                  }
                }
              }
            }

		started_ = true;
		not_responded_to_ping_ = 0;
		LOG(INFO, "started connection to nre-pcd");

		pc_info_t pc_info ( request_pc_info () );

		start_ping_interval_timer();

		return (unsigned int)(pc_info.rank());
    }

    void stop()
    {
    	if (service_thread_ == NULL)
    		return;

    	if(nre_pcd_do_exec_)
    		shutdownNrePcd();

		LOG(TRACE, "stopping nre-pcd connection");
		io_service_.stop();
		service_thread_->interrupt();
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

    /* method: request
     * input parameters: message, timeout
     * return value: shared pointer to the reply Message
     */
	sdpa::shared_ptr<Message> request(const Message &m, unsigned long timeout)
	{
		send(m);
		return sdpa::shared_ptr<Message>(recv(timeout));
	}


    /* method: execute
     * input parameters: encoded activity received from WE, walltime
     * return value: result or error (of type std::string
     */
    execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (WalltimeExceeded, std::exception)
	{
         sdpa::shared_ptr<Message> msg = request(ExecuteRequest(in_activity), walltime);
         if (msg) {
        	 // check if it is a ExecuteReply
        	 if( sdpa::shared_ptr<ExecuteReply> pMsgExecReply = boost::dynamic_pointer_cast<ExecuteReply, Message>(msg) )
        		 return pMsgExecReply->result();
        	 else
        		 throw std::runtime_error("did not receive an ExecuteReply message!");
         }
         else {
        	 throw std::runtime_error("did not get a response from worker!");
         }

    	return std::make_pair(ACTIVITY_FINISHED, "empty result");
	}

  private:
	void ping() throw (NrePcdIsDead)
	{
		try {
			sdpa::shared_ptr<Message> msg = request(PingRequest("tag-1"), timer_timeout_);
			LOG(TRACE, "got reply to ping: " << *msg);
		}
		catch (...) {
			throw NrePcdIsDead();
		}
	}

	pc_info_t request_pc_info ()
    {
		try {
			sdpa::shared_ptr<Message> msg = request(InfoRequest("tag-1"), timer_timeout_);
			LOG(TRACE, "got reply to info request: " << *msg);
			return ((InfoReply*)(msg.get()))->info();
		}
		catch (const std::exception & ex) {
			LOG(ERROR, "could not request information from pc: " << ex.what());
			throw NrePcdIsDead();
		}

	}

    void send(const Message &m)
    {
      boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
      std::string encoded_message(codec_.encode(m));
      DLOG(TRACE, "sending " << encoded_message.size() << " bytes of data to " << nre_worker_endpoint_ << ": " << encoded_message);

      boost::system::error_code ec;
      std::size_t bytes_sent
        (socket_->send_to( boost::asio::buffer( encoded_message
                                              , encoded_message.size()
                                              )
                         , nre_worker_endpoint_
                         , 0
                         , ec
                         )
        );

      LOG_IF( ERROR
            , bytes_sent != encoded_message.size()
            , "not all data could be sent: " << bytes_sent << "/" << encoded_message.size()
            );

      if (ec.value() != boost::system::errc::success)
      {
        LOG( ERROR
           , "could not deliver message: " << ec << ": " << ec.message()
           );
        throw boost::system::system_error (ec);
      }
    }

    template <class T> struct counter_mgr
    {
      counter_mgr(T *counter)
        : counter_(counter)
      { *counter_ += 1; }

      ~counter_mgr() { *counter_ -= 1; }

      T *counter_;
    };

    typedef counter_mgr<unsigned int> waiter_mgr;

    Message *recv(unsigned long timeout)
    {
		Message *msg(NULL);
		boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);

		waiter_mgr waiter(&num_waiting_receiver_);

		while( incoming_messages_.empty() )
		{
			bool notified_(false);

			if( timeout )
			{
				const boost::system_time to(boost::get_system_time() + boost::posix_time::seconds(timeout));
				notified_ = msg_avail_.timed_wait(lock, to);
			}
			else
			  msg_avail_.wait(lock);

			if( !notified_ )
			{
			  if (! incoming_messages_.empty())
				break;
			  else
				if (is_pcd_dead())
				  throw NrePcdIsDead();
				else
				  throw WalltimeExceeded();
			}

			DLOG(TRACE, "receiver woke up...");
			if (started_ && is_pcd_dead())
			  throw NrePcdIsDead();
		}

		msg = incoming_messages_.front();
		incoming_messages_.pop_front();

		return msg;
    }

    void schedule_receive()
    {
    	socket_->async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&NreWorkerClient::handle_receive_from,
					  this,
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

		DLOG(TRACE, "ping timer cancelled");
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
			  LOG(ERROR, "The nre-pcd is probably dead, waking up other threads...");
			  boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
			  msg_avail_.notify_all();
			}
    	}
    	timer_active_ = false;
    }

    void handle_send_to(const boost::system::error_code &error, size_t bytes_sent)
    {
    	DLOG(TRACE, "sent " << bytes_sent << " bytes of data (error_code=" << error << ")...");
    }

    void handle_receive_from(const boost::system::error_code &error, size_t bytes_recv)
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
    				if (dynamic_cast<PingReply*>(msg))
    				{
    					boost::unique_lock<boost::recursive_mutex> lock(msg_mtx_);
    					// notify initial start() synchronous ping request
    					incoming_messages_.push_back(msg);
    					msg_avail_.notify_one();
    				}
    				else
    					LOG(WARN, "ignoring message (not completely started yet): " << *msg);

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

    	try {
    		io_service_.run();
    	}
    	catch (std::exception& e)
    	{
    		LOG(ERROR, "Exception occurred in service_thread: "<<e.what());
    	}

    }

    std::string nre_worker_location_;
    unsigned short my_reply_port_;
    Codec codec_;

    // boost
    boost::asio::io_service io_service_;
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

    enum { max_length = (2<<16) };
    char data_[max_length];

    bool nre_pcd_do_exec_;
    std::string nre_pcd_binary_;
    std::vector<std::string> nre_pcd_search_path_;
    std::vector<std::string> nre_pcd_pre_load_;

    pid_t pidPcd;
  };
}}}

#endif
