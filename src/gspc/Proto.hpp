namespace gspc
{
  namespace interface
  {
    class ResourceManager;
    class Scheduler;
    class WorkflowEngine;
  }

  class PetriNetWorkflow;
  class PetriNetWorkflowEngine;
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class GreedyScheduler;
  class ReschedulingGreedyScheduler;
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    class Trivial;
    class Coallocation;
    class WithPreferences;
  }
}

namespace gspc
{
  class RuntimeSystem;

  namespace remote_interface
  {
    struct ID
    {
      std::uint64_t id;
    }

    using Hostname = std::string;

    namespace strategy
    {
      class ssh;
    }

    using Strategy = boost::variant<strategy::ssh>;
  }

  class RemoteInterface
  {
    RemoteInterface (remote_interface::ID);

    AnnotatedForest<Resource, MaybeError<resource::ID>>
      add (Forest<Resource>);

  private:
    remote_interface::ID _id;
  }

  namespace resource
  {
    struct ID
    {
      remote_interface::ID remote_interface;
      std::uint64_t id;
    };
  }

  // struct Resource;
  // struct Worker;
}

namespace gspc
{
  class RuntimeSystem
  {
    std::unordered_map < remote_interface::Hostname
                       , AnnotatedForest<Resource, MaybeError<resource::ID>>
                       >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , Forest<Resource>
          );
    // create rif if not yet running and provide _next_remote_interface_id++

    std::unordered_set<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , Forest<Resource> resources
                    )
    {
      bool failed {false};
      std::unordered_set<resource::ID> resource_ids;

      for (auto const& host_result : add (hostnames, strategy, resources))
      {
        auto const& hostname (host_result.first);

        for (auto const& resource_result : host_result.second)
        {
          if (is_success (resource_result.second))
          {
            resource_ids.emplace
              (boost::get<resource::ID> (resource_result.second));
          }
          else
          {
            failed = true;
          }
        }
      }

      if (failed)
      {
        remove (resource_ids);

        throw std::runtime_error ("failed to bootstrap");
      }

      return resource_ids;
    }

  private:
    remote_interface::ID _next_remote_interface_id {0};

    std::unordered_map< remote_interface::Hostname
                      , remote_interface::ConnectionAndPID
                      > _remote_interface_by_hostname;
    std::unordered_map< resource::ID
                      , remote_interface::Hostname
                      > _hostname_by_resource_id;

    //! \todo OPTIMIZE access to hostname via map<hostID, hostName>
  };
}

int main()
{
  gspc::resource_manager::Trivial resource_manager;

  gspc::RuntimeSystem runtime_system (resource_manager);

  auto const resource_ids
    ( runtime_system.add_or_throw
      ( {"hostname"}
      , remote_interface::strategy::ssh{}
      , Forest<Resource> {…}
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  gspc::petri_net::Workflow workflow;
  gspc::PetriNetWorkflowEngine workflow_engine (workflow);

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  return EXIT_SUCCESS;
}
