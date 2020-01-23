#pragma once

#include <gspc/Resource.hpp>
#include <gspc/Forest.hpp>

namespace gspc
{
  namespace rtm
  {
    UniqueForest<Resource> host_topology
      ( std::size_t num_cores_per_socket
      , std::size_t num_sockets
      , std::size_t num_sockets_with_gpu
      , std::size_t num_loads
      , std::size_t num_stores
      );

    enum TaskType {LOAD, PROCESS, REDUCE, STORE};

    // number_of_nodes
    // working_directory

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

    // expected result:
    // for each shot: wd/shot/result
    // final: wd/final/result

    // intermediate:
    // for each intermediate: wd/{ids...}/result
    // helps to restart when reduction not finished

    struct Parameter
    {
      std::size_t number_of_shots {10000};

      struct
      {
        double load {0.0};
        double process {0.02};
        double reduce {0.01};
        double store {0.0};
      } probability_of_failure;
    };

    // \future sequence
  }
}
