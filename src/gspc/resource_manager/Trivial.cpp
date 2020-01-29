#include <gspc/resource_manager/Trivial.hpp>

namespace gspc
{
  namespace resource_manager
  {
    Trivial::Acquired
      Trivial::acquire ( InterruptionContext const& interruption_context
                       ,  resource::Class resource_class
                       )
    {
      return { WithPreferences::acquire (interruption_context, {resource_class})
             . requested
             };
    }
    void Trivial::release (Acquired const& acquired)
    {
      return WithPreferences::release ({acquired.requested, 0});
    }
  }
}
