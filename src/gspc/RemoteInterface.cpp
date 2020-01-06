#include <gspc/RemoteInterface.hpp>

#include <iostream>

namespace gspc
{
  std::unordered_map< resource_manager::ID
                    , Forest<MaybeError<resource::ID>>
                    >
    const& RemoteInterface::resources() const
  {
    return _resources;
  }

  TreeResult<resource::ID> RemoteInterface::add
    ( resource_manager::ID resource_manager_id
    , Tree<resource::ID> resources
    )
  {
    return add (resource_manager_id, mreturn (resources));
  }
  TreeResult<resource::ID> RemoteInterface::add
    ( resource_manager::ID resource_manager_id
    , TreeResult<resource::ID> resources
    )
  {
    auto const result
      ( bind<resource::ID, resource::ID>
          ( resources
          , [] (resource::ID const& id) noexcept
            {
              //! todo: start processes or return exception_ptr
              return id;
            }
          )
      );

    _resources[resource_manager_id].emplace_back (result);

    return result;
  }

  RemoteInterface::~RemoteInterface()
  {
    if (!_resources.empty())
    {
      std::cerr << "RemoteInterface::~RemoteInterface: Not clean." << std::endl;

      while (true) { /* do nothing: black hole */ }
    }
  }
}
