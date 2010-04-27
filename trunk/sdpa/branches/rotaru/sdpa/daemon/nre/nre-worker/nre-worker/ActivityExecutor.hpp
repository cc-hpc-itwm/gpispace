/*
 * =====================================================================================
 *
 *       Filename:  ActivityExecutor.hpp
 *
 *    Description:  executes activities
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:01:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_APP_NRE_ACTIVITY_EXECUTOR_HPP
#define SDPA_APP_NRE_ACTIVITY_EXECUTOR_HPP 1

#include <iostream>
#include <list>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/memory.hpp>

#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>
#include <sdpa/daemon/nre/Codec.hpp>
#include <sdpa/modules/ModuleLoader.hpp>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace sdpa { namespace nre { namespace worker {
  class ActivityExecutor : public ExecutionContext
  {
  public:
    explicit
    ActivityExecutor(const std::string &my_location, int rank)
      : loader_(sdpa::modules::ModuleLoader::create())
      , location_(my_location)
      , rank_(rank)
      , socket_(NULL)
      , barrier_(2)
      , service_thread_(NULL)
    {}

    ~ActivityExecutor() throw()
    {
      try
      {
        stop();
      }
      catch (...)
      {
        LOG(ERROR, "error while stopping executor...");
      }
    }
    const std::string &location() const { return location_; }

    void start();
    bool stop();

    // message context
    sdpa::modules::ModuleLoader &loader() { return *loader_; }
    int getRank() const { return rank_; }

    void operator()();
  private:
    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv);
    void execution_thread();
    void trigger_shutdown();
    
    sdpa::modules::ModuleLoader::ptr_t loader_;
    std::string location_;
    int rank_;

    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket *socket_;
    boost::barrier barrier_;
    boost::thread *service_thread_;

    typedef std::list<boost::thread *> thread_list_t;
    thread_list_t execution_threads_;
    typedef std::pair<boost::asio::ip::udp::endpoint, Message *> request_t;
    typedef std::list<request_t> request_list_t;
    boost::recursive_mutex mtx_;
    boost::condition_variable_any request_avail_;
    request_list_t requests_;

    enum { max_length = ((2<<16 )-1) };
    char data_[max_length];

    sdpa::nre::worker::Codec codec_;
  };
}}}

#endif
