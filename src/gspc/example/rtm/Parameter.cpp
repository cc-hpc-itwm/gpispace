#include <gspc/example/rtm/Parameter.hpp>

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
      )
    {
      UniqueForest<Resource> topology;

      using Children = std::unordered_set<std::uint64_t>;

      Children sockets;

      Children loads;
      Children stores;

      for (std::size_t load (0); load < num_loads; ++load)
      {
        loads.emplace (topology.insert ({"load_proxy"}, {}));
      }
      for (std::size_t store (0); store < num_stores; ++store)
      {
        stores.emplace (topology.insert ({"store_proxy"}, {}));
      }

      for (std::size_t socket (0); socket < num_sockets; ++socket)
      {
        Children cores;

        for (std::size_t core (0); core < num_cores_per_socket; ++core)
        {
          cores.emplace (topology.insert ({"core"}, {}));
        }

        if (socket < num_sockets_with_gpu)
        {
          auto const gpu_proxy (topology.insert ({"gpu_proxy"}, {}));

          for (auto const& core : cores)
          {
            topology.insert ({"gpu", "gpu_proxy"}, {core, gpu_proxy});
          }
        }

        for (auto const& core : cores)
        {
          for (auto const& load : loads)
          {
            topology.insert ({"load", "load_proxy"}, {core, load});
          }
          for (auto const& store : stores)
          {
            topology.insert ({"store", "store_proxy"}, {core, store});
          }
        }

        sockets.emplace (topology.insert ({"socket"}, cores));
      }

      topology.insert ({"node"}, sockets);

      return topology;
    }
  }
}
