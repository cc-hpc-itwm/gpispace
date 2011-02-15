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
 *     Updated by:  Tiberiu Rotaru
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_APPS_NRE_WORKER_MESSAGES_HPP
#define SDPA_APPS_NRE_WORKER_MESSAGES_HPP 1

#include <ostream>
#include <string>
#include <fhglog/fhglog.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/nre/nre-worker/ExecutionContext.hpp>
#include <sdpa/memory.hpp>

#include <sys/time.h>
#include <sys/resource.h>

namespace sdpa { namespace nre { namespace worker {
  class Message
  {
  public:
	  typedef sdpa::shared_ptr<Message> Ptr;
	  Message() : id_("unset id"){}

	  explicit Message(const std::string &msg_id): id_(msg_id){}
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
	  typedef sdpa::shared_ptr<Reply> Ptr;
	  Reply() {}

	  explicit Reply(const std::string &a_id) : Message(a_id) {}
	  virtual ~Reply() {}

	  virtual Message *execute(ExecutionContext *)
	  {
		  return NULL;
	  }
  };

  class Request : public Message
  {
  public:
	  typedef sdpa::shared_ptr<Request> Ptr;
	  Request() {}
	  explicit Request (const std::string &a_id): Message(a_id){}
	  virtual ~Request() {}
  };

  class PingReply : public Reply
  {
  public:
    typedef struct rusage  usage_t;
    typedef sdpa::shared_ptr<PingReply> Ptr;

    PingReply(): pid_(getpid())
    {
    	getrusage(RUSAGE_SELF, &usage_);
    }

    explicit PingReply(const std::string &a_id) : Reply(a_id), pid_(getpid())
    {
    	getrusage(RUSAGE_SELF, &usage_);
    }

    virtual ~PingReply() {}

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
	  typedef sdpa::shared_ptr<PingRequest> Ptr;
	  PingRequest() : key_("") {}
	  explicit PingRequest(const std::string &a_msg_id) : Request(a_msg_id) {}

          virtual ~PingRequest() {}

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

  struct pc_info_t
  {
    pc_info_t() : pid_(getpid()), rank_(0){}
    explicit pc_info_t(int rank) : pid_(getpid()), rank_(rank) {}

    int & rank() { return rank_; }
    const int & rank() const { return rank_; }

    pid_t & pid() { return pid_; }
    const pid_t & pid() const { return pid_; }
  private:
    pid_t pid_;
    int rank_;
  };

  class InfoReply : public Reply
  {
  public:
	  typedef sdpa::shared_ptr<InfoReply> Ptr;
	  InfoReply() { }
	  InfoReply(const std::string &a_id, ExecutionContext *ctxt) : Reply(a_id), info_(ctxt->getRank()){ }
          virtual ~InfoReply() {}

	  virtual void writeTo(std::ostream &os) const
	  {
		  os << "InfoReply: id=" << id() << " pid=" << info_.pid() << " rank=" << info_.rank();
	  }

	  pc_info_t & info() { return info_; }
	  const pc_info_t & info() const { return info_; }
  private:
	  pc_info_t info_;
  };

  class InfoRequest : public Request
  {
  public:
	  typedef sdpa::shared_ptr<InfoRequest> Ptr;
	  InfoRequest() : key_(""){}
	  explicit InfoRequest(const std::string &a_msg_id) : Request(a_msg_id) {}
          virtual ~InfoRequest() {}

	  virtual void writeTo(std::ostream &os) const
	  {
		  os << "InfoRequest: id=" << id();
	  }

	  virtual Reply *execute(ExecutionContext *pCtx)
	  {
		  return pCtx->reply(this);
		  //return new InfoReply(id(), ctxt);
	  }
  private:
	  std::string key_;
  };

  class ExecuteReply : public Reply
  {
  public:
	  typedef sdpa::shared_ptr<ExecuteReply> Ptr;
	  ExecuteReply() {}
	  explicit ExecuteReply(const execution_result_t& exec_result) : exec_result_(exec_result) {}
          virtual ~ExecuteReply() {}

	  virtual void writeTo(std::ostream &os) const
	  {
		  os << "ExecuteReply: result="<< exec_result_.second;
	  }

	  execution_result_t& result() { return exec_result_; }
	  const execution_result_t& result() const { return exec_result_; }

  private:
	  execution_result_t exec_result_; // defined in IWorkflowEngine
  };

  class ExecuteRequest : public Request
  {
  public:
	  typedef sdpa::shared_ptr<ExecuteRequest> Ptr;
	  ExecuteRequest() {}
	  ExecuteRequest(const encoded_type &act) : activity_(act) {}
          virtual ~ExecuteRequest() {}

	  virtual bool would_block() const { return true; }

	  virtual void writeTo(std::ostream &os) const
	  {
		  os << "Execute(";
		  // activity().writeTo(os, false);
		  os<<activity_;
		  os << ")";
	  }

	  virtual Reply *execute(ExecutionContext *pCtx)
	  {
		  return pCtx->reply(this);
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
