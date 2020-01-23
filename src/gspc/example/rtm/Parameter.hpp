#pragma once

#include <gspc/Resource.hpp>
#include <gspc/Forest.hpp>

namespace gspc
{
  namespace rtm
  {
    /* Topology (some sockets have a GPU, not necessary all)
       NODE -> SOCKET -> CORE <- GPU -\
                      -> CORE <- GPU --\ GPU
                      -> CORE <- GPU --/ SINGLETON
                      -> CORE <- CPU -/

            -> SOCKET -> CORE <- IO-LOAD --------------------> LOAD2-SINGLETON
                              <- IO-LOAD -> LOAD1-SINGLETON
                      -> CORE <- IO-LOAD --------------------> LOAD2-SINGLETON
                              <- IO-LOAD -> LOAD1-SINGLETON
                      -> ...
    */

    UniqueForest<Resource> host_topology
      ( std::size_t num_cores_per_socket
      , std::size_t num_sockets
      , std::size_t num_sockets_with_gpu
      , std::size_t num_loads
      , std::size_t num_stores
      );

    enum TaskType {LOAD, PROCESS, REDUCE, STORE};

    // policy: reschedule job 3 times to different resources
    // policy: stop using failing resource after 3 consecutive failures
    // how many nodes per process tasks

    // add/remove resources during execution
    // stop with hard cancel/restart

    // resource classes:
    // LOAD - i/o-load
    // STORE - i/o-store
    // REDUCE - Socket
    // PROCESS - {[GPU], [nodes]} // GPU: 1..6, node: 1..4

    using Shot = std::uint64_t;

    struct ProbabilityOfFailure
    {
      double load {0.0};
      double process {0.0};
      double reduce {0.0};
      double store {0.0};

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    struct Parameter
    {
      Shot number_of_shots {0};
      ProbabilityOfFailure probability_of_failure;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    // \future sequence
  }
}

namespace gspc
{
  namespace rtm
  {
    template<typename Archive>
      void ProbabilityOfFailure::serialize (Archive& ar, unsigned int)
    {
      ar & load;
      ar & process;
      ar & reduce;
      ar & store;
    }

    template<typename Archive>
      void Parameter::serialize (Archive& ar, unsigned int)
    {
      ar & number_of_shots;
      ar & probability_of_failure;
    }
  }
}
