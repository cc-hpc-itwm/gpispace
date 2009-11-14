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
#include <sdpa/wf/Activity.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>

#include <sys/time.h>
#include <sys/resource.h>

namespace sdpa { namespace nre { namespace worker {
  class Message
  {
  public:
    Message()
    {}
    virtual ~Message() {}

    virtual Message *execute(ExecutionContext *context) = 0;
    virtual void writeTo(std::ostream &) const = 0;

    // indicate a long-running execution request
    virtual bool would_block() const { return false; }
  };

  class Reply : public Message
  {
  public:
    Reply() {}
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
    virtual ~Request() {}
  };

  class PingReply : public Reply
  {
  public:
    typedef struct rusage  usage_t;

    PingReply()
      : key_("")
      , pid_(0)
    {}

    PingReply(const std::string &a_key, const pid_t &my_pid)
      : key_(a_key)
      , pid_(my_pid)
    {
      getrusage(RUSAGE_SELF, &usage_);
    }

    virtual void writeTo(std::ostream &os) const
    {
      os << "PingReply: key=" << key() << " pid=" << pid() << " utime=" << usage().ru_utime.tv_sec;
    }

    std::string &key() { return key_; }
    const std::string &key() const { return key_; }

    pid_t &pid() { return pid_; }
    const pid_t &pid() const { return pid_; }

    const usage_t &usage() const { return usage_; }
    usage_t &usage() { return usage_; }
  private:
    std::string key_;
    pid_t pid_;
    usage_t usage_;
  };

  class PingRequest : public Request
  {
  public:
    PingRequest()
      : key_("")
    {}

    explicit PingRequest(const std::string &a_key)
      : key_(a_key)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "PingRequest: key="<< key();
    }

    std::string &key() { return key_; }
    const std::string &key() const { return key_; }

    virtual Reply *execute(ExecutionContext *)
    {
      return new PingReply(key(), getpid());
    }
  private:
    std::string key_;
  };

  class ExecuteReply   : public Reply
  {
  public:
    typedef sdpa::wf::Activity result_t;

    ExecuteReply() {}

    explicit
    ExecuteReply(const result_t & execution_result)
      : result_(execution_result)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "ExecuteReply: result="<< result();
    }

    result_t &result() { return result_; }
    const result_t &result() const { return result_; }
  private:
    result_t result_;
  };

  class ExecuteRequest : public Request
  {
  public:
    ExecuteRequest()
    {}

    explicit ExecuteRequest(const sdpa::wf::Activity &act)
      : activity_(act)
    {}

    virtual bool would_block() const { return true; }

    virtual void writeTo(std::ostream &os) const
    {
      os << "Execute(";
      activity().writeTo(os, false);
      os << ")";
    }

    virtual Reply *execute(ExecutionContext *ctxt)
    {
      const std::string mod_name(activity().method().module());
      const std::string fun_name(activity().method().name());

      try
      {
        LOG(INFO, "executing: " << activity());

        ctxt->loader().get(mod_name).call(fun_name, activity().parameters());
        activity().check_parameters(true /* relaxed */);

        LOG(INFO, "execution of activity finished");
        activity().state() = sdpa::wf::Activity::ACTIVITY_FINISHED;
        return new ExecuteReply(activity());
      }
      catch (const std::exception &ex)
      {
        LOG(ERROR, "execution of activity failed: " << ex.what());
        activity().state() = sdpa::wf::Activity::ACTIVITY_FAILED;
        activity().reason() = ex.what();
        return new ExecuteReply(activity());
      }
      catch (...)
      {
        LOG(ERROR, "execution of activity failed: ");
        activity().state() = sdpa::wf::Activity::ACTIVITY_FAILED;
        activity().reason() = ex.what();
        return new ExecuteReply(activity());
      }
    }

    sdpa::wf::Activity &activity() { return activity_; }
    const sdpa::wf::Activity &activity() const { return activity_; }
  private:
    sdpa::wf::Activity activity_;
  };

  class ModuleLoaded : public Reply
  {
  public:
    ModuleLoaded()
    {}

    ModuleLoaded(const std::string &path_to_module)
      : path_(path_to_module)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "ModuleLoaded: path="<< path();
    }

    const std::string &path() const { return path_; }
    std::string &path() { return path_; }
  private:
    std::string path_;
  };

  class ModuleNotLoaded : public Reply
  {
  public:
    ModuleNotLoaded()
    {}

    ModuleNotLoaded(const std::string &path_to_module, const std::string &reason_for_failure)
      : path_(path_to_module)
      , reason_(reason_for_failure)
    {}

    virtual void writeTo(std::ostream &os) const
    {
      os << "ModuleNotLoaded: path="<< path() << " reason=" << reason();
    }

    const std::string &path() const { return path_; }
    std::string &path() { return path_; }

    const std::string &reason() const { return reason_; }
    std::string &reason() { return reason_; }
  private:
    std::string path_;
    std::string reason_;
  };

  class LoadModuleRequest : public Request
  {
  public:
    LoadModuleRequest()
    {}

    explicit LoadModuleRequest(const std::string &path_to_module)
      : path_(path_to_module)
    { }

    virtual void writeTo(std::ostream &os) const
    {
      os << "LoadModule: path="<< path();
    }

    virtual Reply *execute(ExecutionContext *ctxt)
    {
      try
      {
        ctxt->loader().load(path());
        return new ModuleLoaded(path());
      }
      catch (const std::exception &ex)
      {
        LOG(WARN, "execution of activity failed: " << ex.what());
        return new ModuleNotLoaded(path(), ex.what());
      }
    }

    const std::string &path() const { return path_; }
    std::string &path() { return path_; }
  private:
    std::string path_;
  };
}}}

inline std::ostream &operator<<(std::ostream &os, const sdpa::nre::worker::Message &m)
{
  m.writeTo(os);
  return os;
}

#endif
