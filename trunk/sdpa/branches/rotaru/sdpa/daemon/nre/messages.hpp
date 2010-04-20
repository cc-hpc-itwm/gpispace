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
    ExecuteReply(const result_type & execution_result, const ExecutionState state)
      : result_(execution_result)
      , state_ (state)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "ExecuteReply: result="<< result();
    }

    result_type &result() { return result_; }
    const result_type &result() const { return result_; }

    const ExecutionState & state() const { return state_; }
    ExecutionState & state() { return state_; }
  private:
    result_type result_; // defined in IWorkflowEngine
    ExecutionState state_;
  };

  namespace detail {
	  // TODO implement for the real activity object type
	  template <typename Activity>
	  inline std::string get_module_name (const Activity & /*act*/) {
		  // boost::get<transition_t::mod_call_t> (act.transition().data()).m();
		  return "dummy";
	  }

	  template <typename Activity>
	  inline std::string get_function_name (const Activity & /*act*/) {
		  // boost::get<transition_t::mod_call_t> (act.transition().data()).f();
		  return "dummy";
	  }
  }

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
    	// decode activity -> activity object
    	//
      typedef std::string activity_t;
      activity_t act ("dummy activity");

      const std::string mod_name (detail::get_module_name (act));
      const std::string fun_name (detail::get_function_name (act) );

      Reply *reply(NULL);
      try
      {
        if (mod_name.empty() || fun_name.empty())
        {
          throw std::runtime_error("empty module or function: mod="+mod_name+" fun="+fun_name);
        }

        LOG(INFO, "executing: " << activity());

//        bool keep_going = activity().properties().get<bool>("keep_going", false);
        bool keep_going = true;

/*
        ctxt->loader().get(mod_name).call(fun_name
                                        , activity().parameters()
                                        , keep_going);
        activity().check_parameters(keep_going);
*/
        LOG(INFO, "execution of activity finished");

        // encode activity again
        reply = new ExecuteReply(activity(), ACTIVITY_FINISHED);
      }
      catch (const sdpa::modules::MissingFunctionArgument &mfa)
      {
        LOG(ERROR, "function " << mfa.module() << "." << mfa.function()
                               << " expected argument " << mfa.arguments());
//        activity().reason() = mfa.what();
        reply = new ExecuteReply(activity(), ACTIVITY_FAILED);
      }
      catch (const std::exception &ex)
      {
        LOG(ERROR, "execution of activity failed: " << ex.what());
//        activity().reason() = ex.what();
        reply = new ExecuteReply(activity(), ACTIVITY_FAILED);
      }
      catch (...)
      {
        LOG(ERROR, "execution of activity failed: ");
//        activity().reason() = "unknown reason";
        reply = new ExecuteReply(activity(), ACTIVITY_FAILED);
      }

      assert(reply);
      reply->id() = id();
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
