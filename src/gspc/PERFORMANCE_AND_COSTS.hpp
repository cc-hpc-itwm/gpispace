
    //! implement as counting resources!?
    //! Note: would lead to diamonds:
    //!    +----> core ------+
    //!    |                 v
    //! socket -> core -> #memory
    //!    |                 ^
    //!    +----> core ------+
    //! -> support counting resources and diamonds in the
    //!    forest/resource_manager

    //! Alternative: Separate structure
    //! Structure:            Memory:
    //!    +----> core        core ------+
    //!    |                             v
    //! socket -> core        core -> #memory
    //!    |                             ^
    //!    +----> core        core ------+

// , local memory, global
    //! memory, all memories that are shared between different
    //! resources
    struct MemoryInformation
    {
      MemoryID memory_id;
      Bytes bytes;
      Alignment alignment;
    };

    //! Costs: To compare different tasks
    //! - memory transfer costs: constructor of CostAssignment
    //!   receives information about distribution -> no events required
    struct Costs
    {
      // assign costs to each possible (set of) execution resource(s)
      // -> for single_resource_class this becomes
      //    [(resource::ID, Cost)]
      // -> for multiple_resource_class this becomes
      //    [([resource::ID, Cost)] for all subsets of resources

      // Task -> std::unordered_map<resource::ID, Cost>
    }

    //! performance model: called by the scheduler after task
    //! extraction and before schedule allowed to reorder/remove maybe
    //! even add schedule_information

    //! static: no events delivered back
    struct StaticPerformanceModel
    {
      std::function<schedule_information (schedule_information)> optimize;
    };

    //! - dynamic model: overwrite static order produced by workflow engine
    //!   (application developer) using dynamic information, events

    //! events delivered to performance model by executing resource?
    //! or scheduler?
    struct DynamicPerformanceModel : public StaticPerformanceModel
    {
      using Event = boost::variant< TaskStarted
                                  , TaskFinished
                                  , Failed
                                  , Cancelled
                                  // fine grained, memory get/put?
                                  >;

      void event (task::ID, resource::ID, Event);
    };
