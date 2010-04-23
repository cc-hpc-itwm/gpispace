/*
 * =====================================================================================
 *
 *       Filename:  messages.hpp
 *
 *    Description:  message definitions
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:24:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_APPS_NRE_WORKER_MESSAGES_HPP
#define SDPA_APPS_NRE_WORKER_MESSAGES_HPP 1

#include <ostream>
#include <string>
#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/IWorkflowEngine.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>

#include <sys/time.h>
#include <sys/resource.h>

namespace sdpa { namespace nre { namespace worker {
  class Message
  {
  public:
    Message()
      : id_("unset id")
    {}

    explicit
    Message(const std::string &msg_id)
      : id_(msg_id)
    {}

    virtual ~Message() {}

    virtual Message *execute(ExecutionContext *context) = 0;
    virtual void writeTo(std::ostream &) const = 0;

    // indicate a long-running execution request
    virtual bool would_block() const { return false; }

    const std::string &id() const { return id_; }
    std::string &id() { return id_; }
  private:
    std::string id_;
  };

  class Reply : public Message
  {
  public:
    Reply() {}

    explicit
    Reply(const std::string &a_id)
      : Message(a_id)
    {}

    virtual ~Reply() {}

    virtual Message *execute(ExecutionContext *)
    {
      return NULL;
    }
  };

  class Request : public Message
  {
  public:
    Request() {}

    explicit
    Request (const std::string &a_id)
      : Message(a_id)
    {}

    virtual ~Request() {}
  };

  class PingReply : public Reply
  {
  public:
    typedef struct rusage  usage_t;

    PingReply()
      : pid_(getpid())
    {
      getrusage(RUSAGE_SELF, &usage_);
    }

    explicit
    PingReply(const std::string &a_id)
      : Reply(a_id)
      , pid_(getpid())
    {
      getrusage(RUSAGE_SELF, &usage_);
    }

    virtual void writeTo(std::ostream &os) const
    {
      os << "PingReply: id=" << id() << " pid=" << pid() << " utime=" << usage().ru_utime.tv_sec;
    }

    pid_t &pid() { return pid_; }
    const pid_t &pid() const { return pid_; }

    const usage_t &usage() const { return usage_; }
    usage_t &usage() { return usage_; }
  private:
    pid_t pid_;
    usage_t usage_;
  };

  class PingRequest : public Request
  {
  public:
    PingRequest()
      : key_("")
    {}

    explicit PingRequest(const std::string &a_msg_id)
      : Request(a_msg_id)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "PingRequest: id=" << id();
    }

    virtual Reply *execute(ExecutionContext *)
    {
      return new PingReply(id());
    }
  private:
    std::string key_;
  };

  enum ExecutionState
  {
	  ACTIVITY_FINISHED
	, ACTIVITY_FAILED
	, ACTIVITY_CANCELLED
  };

  typedef std::pair<ExecutionState, result_type> execution_result_t;

  class ExecuteReply   : public Reply
  {
  public:

    ExecuteReply() {}

    explicit
    ExecuteReply(const execution_result_t& exec_result)
      : exec_result_(exec_result)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "ExecuteReply: result="<< exec_result_.second;
    }

    execution_result_t& result() { return exec_result_; }
    const execution_result_t& result() const { return exec_result_; }

  private:
    execution_result_t exec_result_; // defined in IWorkflowEngine
  };

  /*namespace detail {
	  // TODO implement for the real activity object type
	  template <typename Activity>
	  inline std::string get_module_name (const Activity & ) {
		  // boost::get<transition_t::mod_call_t> (act.transition().data()).m();
		  return "dummy";
	  }

	  template <typename Activity>
	  inline std::string get_function_name (const Activity & ) {
		  // boost::get<transition_t::mod_call_t> (act.transition().data()).f();
		  return "dummy";
	  }
  }*/

  class ExecuteRequest : public Request
  {
  public:
    ExecuteRequest()
    {}

    ExecuteRequest(const encoded_type &act)
      : activity_(act)
    {}

    virtual bool would_block() const { return true; }

    virtual void writeTo(std::ostream &os) const
    {
      os << "Execute(";
     // activity().writeTo(os, false);
      os<<activity_;
      os << ")";
    }

    virtual Reply *execute(ExecutionContext *ctxt)
    {
      typedef std::string activity_t;
      activity_t act ("dummy activity");

      Reply *reply(NULL);
      try
      {
    	  //LOG(INFO, "executing: " << activity());
    	  LOG (DEBUG, "received new activity ... ");
    	  LOG (DEBUG, "executing activity ... ");
    	  // encode activity again
    	  execution_result_t exec_res(std::make_pair(ACTIVITY_FINISHED, activity()));

    	  LOG (DEBUG, "creating a reply message ... ");
    	  reply = new ExecuteReply(exec_res);
      }
      catch (const std::exception &ex)
      {
    	  LOG(ERROR, "execution of activity failed: " << ex.what());
    	  execution_result_t exec_res(std::make_pair(ACTIVITY_FAILED, activity()));
    	  reply = new ExecuteReply(exec_res);
      }
      catch (...)
      {
    	  LOG(ERROR, "execution of activity failed: ");
    	  execution_result_t exec_res(std::make_pair(ACTIVITY_FAILED, activity()));
    	  reply = new ExecuteReply(exec_res);
      }

      assert(reply);
      reply->id() = id();
      LOG (DEBUG, "replying with id "<<reply->id());
      return reply;
    }

    encoded_type &activity() { return activity_; }
    const encoded_type &activity() const { return activity_; }
  private:
    encoded_type activity_;
  };


}}}

inline std::ostream &operator<<(std::ostream &os, const sdpa::nre::worker::Message &m)
{
  m.writeTo(os);
  return os;
}

#endif
