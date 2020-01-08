#include <gspc/util/Forest.hpp>
#include <gspc/MaybeError.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/variant.hpp>
#include <boost/range/adaptor/map.hpp>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace interface
  {
    class ResourceManager;
    class Scheduler;
    class WorkflowEngine;
  }

  class PetriNetWorkflow{};
  class PetriNetWorkflowEngine
  {
  public:
    PetriNetWorkflowEngine (PetriNetWorkflow);
  };
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class GreedyScheduler
  {
  public:
    template<typename WE, typename RM, typename RTS>
      GreedyScheduler (WE&, RM&, RTS const&);

    void wait();
  };
  class ReschedulingGreedyScheduler;
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    class Trivial{};
    class Coallocation;
    class WithPreferences;
  }
}

template<typename, typename A> using AnnotatedForest
  = std::list<std::pair<int, A>>;

namespace gspc
{
  class Resource
  {
  public:
    int _;
  };

  bool operator== (Resource const&, Resource const&);

  namespace remote_interface
  {
    struct ID
    {
      std::uint64_t id;
    };

    bool operator== (ID const&, ID const&);

    using Hostname = std::string;
    using ConnectionAndPID = int;

    namespace strategy
    {
      class ssh {};
    }

    using Strategy = boost::variant<strategy::ssh>;
  }

  namespace resource
  {
    struct ID
    {
      remote_interface::ID remote_interface;
      std::uint64_t id;
    };

    bool operator== (ID const&, ID const&);
  }

  class RemoteInterface
  {
    RemoteInterface (remote_interface::ID);

    AnnotatedForest<Resource, MaybeError<resource::ID>>
      add (util::Forest<Resource>);

  private:
    remote_interface::ID _id;
  };
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::remote_interface::ID
  , x
  , x.id
  );
FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::resource::ID
  , x
  , x.id
  , x.remote_interface
  );
FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::Resource
  , r
  , r._
  );

namespace gspc
{
  class RuntimeSystem;

  // struct Worker;
}

namespace gspc
{
  class RuntimeSystem
  {
  public:
    template<typename RM> RuntimeSystem (RM&);

    std::unordered_map < remote_interface::Hostname
                       , AnnotatedForest<Resource, MaybeError<resource::ID>>
                       >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , util::Forest<Resource>
          );
    // create rif if not yet running and provide _next_remote_interface_id++

    std::unordered_set<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , util::Forest<Resource> resources
                    )
    {
      bool failed {false};
      std::unordered_set<resource::ID> resource_ids;

      for ( auto const& host_result
          : add ( hostnames
                , strategy
                , std::move (resources)
                ) | boost::adaptors::map_values
          )
      {
        for (auto const& resource_result : host_result)
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

    void remove (std::unordered_set<resource::ID>);

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
try
{
  gspc::resource_manager::Trivial resource_manager;

  gspc::RuntimeSystem runtime_system (resource_manager);

  auto const resource_ids
    ( runtime_system.add_or_throw
      ( {"hostname"}
      , gspc::remote_interface::strategy::ssh{}
      , gspc::util::Forest<gspc::Resource> {}
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  gspc::PetriNetWorkflow workflow;
  gspc::PetriNetWorkflowEngine workflow_engine (workflow);

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
