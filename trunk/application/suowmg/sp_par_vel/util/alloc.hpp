
#include <string>

#include <fvm-pc/pc.hpp>
#include <stdexcept>

namespace sp_par_vel
{
  long alloc (const long & size, const std::string & descr)
  {
    const long handle (static_cast<long> (fvmGlobalAlloc (size)));

    if (handle == 0)
      {
        throw std::runtime_error ("BUMMER! " + descr + " == 0");
      }

    return handle;
  }
}
