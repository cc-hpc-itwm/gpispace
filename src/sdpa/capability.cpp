#include <sdpa/capability.hpp>
#include <sdpa/id_generator.hpp>

namespace sdpa
{
  namespace
  {
    id_generator& GLOBAL_id_generator_cap()
    {
      static id_generator g ("cap");
      return g;
    }
  }

  Capability::Capability ( const std::string& name
                         , const std::string& owner
                         )
    : name_ (name)
    , depth_ (0)
    , owner_ (owner)
    , uuid_ (GLOBAL_id_generator_cap().next())
    {}
}
