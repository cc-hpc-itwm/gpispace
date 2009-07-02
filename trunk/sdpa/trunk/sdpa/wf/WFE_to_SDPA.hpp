#ifndef SDPA_WF_WFE_TO_SDPA_HPP
#define SDPA_WF_WFE_TO_SDPA_HPP 1

#include <string>

namespace sdpa { namespace wf {
  class WFE_to_SDPA {
    public:
      typedef std::string workflow_t;
      typedef std::string activity_t;

      virtual void submitWorkflow(const workflow_t &workflow) = 0; // (sub-)workflow
      virtual void submitActivity(const activity_t &activity) = 0; // atomic workflow see Activity.hpp

      // parent job has to cancel all children
      virtual void cancelWorkflow(const workflow_t &workflow) = 0;
      virtual void cancelActivity(const activity_t &activity) = 0;

      virtual void workflowFinished(const workflow_t &workflow) = 0;
      virtual void workflowFailed(const workflow_t &workflow) = 0;
      virtual void workflowCancelled(const workflow_t &workflow) = 0;
  };
}}
