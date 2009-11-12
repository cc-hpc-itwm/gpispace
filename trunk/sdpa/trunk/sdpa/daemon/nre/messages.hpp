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

#include <string>
#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>

namespace sdpa { namespace nre { namespace worker {
  class Reply
  {
  public:
    Reply() {}
    virtual ~Reply() {}
  };

  class Request
  {
  public:
    virtual Reply *execute(ExecutionContext *context) = 0;

    // indicate a long-running execution request
    virtual bool would_block() const { return false; }
  };

  class PingReply : public Reply
  {
  public:
    PingReply()
      : key_("")
    {}

    explicit PingReply(const std::string &a_key)
      : key_(a_key)
    {}

    std::string &key() { return key_; }
    const std::string &key() const { return key_; }
  private:
    std::string key_;
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

    std::string &key() { return key_; }
    const std::string &key() const { return key_; }

    virtual Reply *execute(ExecutionContext *)
    {
      return new PingReply(key());
    }
  private:
    std::string key_;
  };

  class ExecuteReply   : public Reply
  {
  public:
    typedef sdpa::wf::Activity result_t;
    enum status_t
    {
      ST_FINISHED = 0
    , ST_FAILED = 1
    , ST_UNKNOWN = 2
    };

    ExecuteReply() {}

    explicit
    ExecuteReply(const result_t & execution_result, status_t exec_status)
      : result_(execution_result)
      , status_(exec_status)
    {}

    result_t &result() { return result_; }
    const result_t &result() const { return result_; }

    status_t &status() { return status_; }
    const status_t &status() const { return status_; }
  private:
    result_t result_;
    status_t status_;
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

    virtual Reply *execute(ExecutionContext *ctxt)
    {
      const std::string mod_name(activity().method().module());
      const std::string fun_name(activity().method().name());

      try
      {
        ctxt->loader().get(mod_name).call(fun_name, activity().parameters());
        LOG(INFO, "execution of activity finished");
        return new ExecuteReply(activity(), ExecuteReply::ST_FINISHED);
      }
      catch (const std::exception &ex)
      {
        LOG(WARN, "execution of activity failed: " << ex.what());
        return new ExecuteReply(activity(), ExecuteReply::ST_FAILED);
      }
      catch (...)
      {
        LOG(WARN, "execution of activity failed: ");
        return new ExecuteReply(activity(), ExecuteReply::ST_FAILED);
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

#endif
