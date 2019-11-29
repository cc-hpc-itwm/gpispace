#include <utility>

namespace sdpa
{
  namespace events
  {
    template<typename Event, typename... Args>
      std::string Codec::encode (Args&&... args) const
    {
      Event const e (std::forward<Args> (args)...);
      return encode (&e);
    }
  }
}
