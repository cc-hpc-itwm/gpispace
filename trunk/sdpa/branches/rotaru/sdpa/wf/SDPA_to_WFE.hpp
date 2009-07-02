#ifndef SDPA_WF_WFE_TO_SDPA_HPP
#define SDPA_WF_WFE_TO_SDPA_HPP 1

#include <string>

namespace sdpa { namespace wf {

  class WFE_to_SDPA;
  class SDPA_to_WFE {
    public:
      typedef std::string workflow_t;

      typedef std::string workflow_id_t;
      typedef std::string activity_id_t;
      typedef std::string parameter_t;
      typedef std::list<parameter_t> parameter_list_t;

      // transition from pending to running
      virtual void activityDispatched(const workflow_id_t &wid, const activity_id_t &aid) = 0;

      // transition from running to failed     
      virtual void activityFailed(const workflow_id_t &wid, const activity_id_t &aid, const parameter_list_t &output) = 0;

      // transition from running to finished
      virtual void activityFinished(const workflow_id_t &wid, const activity_id_t &aid, const parameter_list_t &output) = 0;

      // transition from * to cancelled
      virtual void activityCancelled(const workflow_id_t &wid, const activity_id_t &aid) = 0; 

      virtual void registerHandler(WFE_to_SDPA *sdpa) = 0;

      // corresponds to WFE_to_SDPA::workflowFinished/Failed
      virtual void submitWorkflow(workflow_t &wf) = 0;

      virtual void cancelWorkflow(workflow_t &wf) = 0;
  };
}}
