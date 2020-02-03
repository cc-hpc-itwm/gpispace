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

      Children load_proxies;
      Children store_proxies;

      for (std::size_t load (0); load < num_loads; ++load)
      {
        load_proxies.emplace (topology.insert ({"load_proxy"}, {}));
      }
      for (std::size_t store (0); store < num_stores; ++store)
      {
        store_proxies.emplace (topology.insert ({"store_proxy"}, {}));
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
            topology.insert ({"gpu"}, {core, gpu_proxy}, gpu_proxy);
          }
        }

        for (auto const& core : cores)
        {
          for (auto const& load_proxy : load_proxies)
          {
            topology.insert ({"load"}, {core, load_proxy}, load_proxy);
          }
          for (auto const& store_proxy : store_proxies)
          {
            topology.insert ({"store"}, {core, store_proxy}, store_proxy);
          }
        }

        sockets.emplace (topology.insert ({"socket"}, cores));
      }

      topology.insert ({"node"}, sockets);

      return topology;
    }
  }
}
