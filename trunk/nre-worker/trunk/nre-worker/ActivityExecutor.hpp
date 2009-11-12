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

#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>
#include <sdpa/modules/ModuleLoader.hpp>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace sdpa { namespace nre { namespace worker {
  class Request;
  class Reply;

  class ActivityExecutor : public ExecutionContext
  {
  public:
    explicit
    ActivityExecutor(const std::string &my_location)
      : loader_(sdpa::modules::ModuleLoader::create())
      , location_(my_location)
      , socket_(NULL)
      , barrier_(2)
      , service_thread_(NULL)
      , execution_thread_(NULL)
    {}

    const std::string &location() const { return location_; }

    void start();
    void stop();

    // message context
    sdpa::modules::ModuleLoader &loader() { return *loader_; }

    void operator()();
  private:
    Request *decode(const std::string &);
    std::string encode(const Reply &);

    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv);
    void execution_thread();
    void trigger_shutdown();
    
    sdpa::modules::ModuleLoader::ptr_t loader_;
    std::string location_;

    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket *socket_;
    boost::barrier barrier_;
    boost::thread *service_thread_;

    boost::thread *execution_thread_;
    typedef std::pair<boost::asio::ip::udp::endpoint, Request *> request_t;
    typedef std::list<request_t> request_list_t;
    boost::recursive_mutex mtx_;
    boost::condition_variable_any request_avail_;
    request_list_t requests_;

    enum { max_length = ((2<<16 )-1) };
    char data_[max_length];
  };
}}}

#endif
