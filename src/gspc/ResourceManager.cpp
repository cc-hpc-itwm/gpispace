#include <gspc/ResourceManager.hpp>

#include <stdexcept>

namespace gspc
{
  TreeResult<resource::ID> ResourceManager::add (Tree<Resource>)
  {
    throw std::runtime_error ("NYI: RM::add");
  }
}
