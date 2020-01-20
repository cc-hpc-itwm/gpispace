#pragma once

namespace gspc
{
  // \todo JS ID? We don't have a jobserver in our thing as it is
  // 100% user, so just let the user identify the context?
  struct UserContextID
  {
  };

  //! \todo Can we come up with more specific data? Should we instead
  //! just let it be WorkerID + enum?
  namespace task_state
  {
    struct Base {
      rts::WorkerID assigned_worker;
    };
    struct Pending : Base { std::string stall_reason; };
    struct GettingData : Base {};
    struct Running : Base {};
    struct PuttingData : Base {};
    struct Finalizing : Base {};

    //! \todo Exception-free handling of the inherent race?
    struct Unknown {};
  }
  using TaskState
    = boost::variant < task_state::Pending
                     , task_state::GettingData
                     , task_state::Running
                     , task_state::PuttingData
                     , task_state::Finalizing
                     >;
}
