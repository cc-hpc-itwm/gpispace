#include <drts/Forest.hpp>
#include <drts/remote_interface/ID.hpp>
#include <drts/resource/ID.hpp>
#include <sdpa/daemon/resource_manager/ResourceManager.hpp>

#include <util-generic/join.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <queue>

namespace
{
  using Resources = gspc::Forest<gspc::resource::ID, gspc::resource::Class>;
  using Children = std::unordered_set<gspc::resource::ID>;

  Resources create_resource_forest
    ( unsigned int num_nodes
    , unsigned int num_sockets_per_node
    , unsigned int num_cores_per_socket
    , std::unordered_set<gspc::resource::ID>& resources_with_children
    )
  {
    Resources resources;
    gspc::remote_interface::ID next_remote_interface_id {0};

    for ( unsigned int i {0}
        ; i < num_nodes
        ; ++i, ++next_remote_interface_id
        )
    {
      gspc::resource::ID next_resource_id (next_remote_interface_id);

      Children sockets;
      for (unsigned int j {0}; j < num_sockets_per_node; ++j)
      {
        Children cores;
        for (unsigned int k {0}; k < num_cores_per_socket; ++k)
        {
          resources.insert (++next_resource_id, {"core"}, {});
          cores.emplace (next_resource_id);
        }

        resources.insert (++next_resource_id, {"socket"},  cores);
        resources_with_children.emplace (next_resource_id);
        sockets.emplace (next_resource_id);
      }

      resources.insert (++next_resource_id, {"node"}, sockets);
      resources_with_children.emplace (next_resource_id);
    }

    return resources;
  }

  void add_resources
    (gspc::ResourceManager& resource_manager, Resources const& resources)
  {
    resources.upward_combine_transform
      ( [&]
        ( gspc::Forest<gspc::resource::ID, gspc::resource::Class>::Node const& node
        , std::list<gspc::Forest<gspc::resource::ID, gspc::resource::Class>::Node const*> const& children
        )
        {
          Children children_ids;
          for (auto const& child : children)
          {
            children_ids.emplace (child->first);
          }

          resource_manager.add (node.first, node.second, children_ids);

          return node;
        }
      );
  }
}

BOOST_AUTO_TEST_CASE
  (acquiring_a_resource_blocks_all_descendants_and_ascendants)
{
  auto const num_nodes
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_sockets_per_node (2);
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_cores_per_socket (3);
    (fhg::util::testing::random<unsigned int>{} (16, 2));

  std::unordered_set<gspc::resource::ID> resources_with_children;

  Resources resources
    ( create_resource_forest
        ( num_nodes, num_sockets_per_node
        , num_cores_per_socket, resources_with_children
        )
    );

  gspc::ResourceManager resource_manager;

  add_resources (resource_manager, resources);

  unsigned int distance
    { fhg::util::testing::random<unsigned int>{}
       (resources_with_children.size() - 1, 0)
    };

  gspc::resource::ID resource_id
    (*std::next (resources_with_children.begin(), distance));

  BOOST_REQUIRE (resource_manager.acquire (resource_id));

  resources.down_up
    ( resource_id
    , [&] (Resources::Node const& x)
      {
        BOOST_REQUIRE (!resource_manager.acquire (x.first));
      }
    );

  BOOST_REQUIRE (!resource_manager.acquire (resource_id));
  resource_manager.release (resource_id);
  BOOST_REQUIRE (resource_manager.acquire (resource_id));

  resources.down_up
    ( resource_id
      , [&] (Resources::Node const& x)
        {
          BOOST_REQUIRE (!resource_manager.acquire (x.first));
        }
    );
}

BOOST_AUTO_TEST_CASE (an_already_acquired_resource_cannot_be_acquired)
{
  auto const num_nodes
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_sockets_per_node (2);
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_cores_per_socket (3);
    (fhg::util::testing::random<unsigned int>{} (16, 2));

  std::unordered_set<gspc::resource::ID> resources_with_children;

  Resources resources
    ( create_resource_forest
        ( num_nodes, num_sockets_per_node
        , num_cores_per_socket, resources_with_children
        )
    );

  gspc::ResourceManager resource_manager;

  add_resources (resource_manager, resources);

  unsigned int distance
    { fhg::util::testing::random<unsigned int>{}
       (resources_with_children.size() - 1, 0)
    };

  gspc::resource::ID resource_id
    (*std::next (resources_with_children.begin(), distance));

  BOOST_REQUIRE (resource_manager.acquire (resource_id));
  BOOST_REQUIRE (!resource_manager.acquire (resource_id));
}

BOOST_AUTO_TEST_CASE (a_released_resource_can_be_again_acquired)
{
  auto const num_nodes
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_sockets_per_node (2);
    (fhg::util::testing::random<unsigned int>{} (16, 2));
  auto const num_cores_per_socket (3);
    (fhg::util::testing::random<unsigned int>{} (16, 2));

  std::unordered_set<gspc::resource::ID> resources_with_children;

  Resources resources
    ( create_resource_forest
        ( num_nodes, num_sockets_per_node
        , num_cores_per_socket, resources_with_children
        )
    );

  gspc::ResourceManager resource_manager;

  add_resources (resource_manager, resources);

  unsigned int distance
    { fhg::util::testing::random<unsigned int>{}
        (resources_with_children.size() - 1, 0)
    };

  gspc::resource::ID resource_id
    (*std::next (resources_with_children.begin(), distance));

  BOOST_REQUIRE (resource_manager.acquire (resource_id));

  BOOST_REQUIRE (!resource_manager.acquire (resource_id));
  resource_manager.release (resource_id);
  BOOST_REQUIRE (resource_manager.acquire (resource_id));
}
