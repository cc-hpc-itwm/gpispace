#ifndef _H_UFBMIG_UTIL_ALLOC
#define _H_UFBMIG_UTIL_ALLOC 1

#include <fvm-pc/pc.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace ufbmig
{
  namespace util
  {
    namespace memory
    {
      namespace global
      {
        inline long alloc (const std::size_t size, const std::string & descr)
        {
          const fvmAllocHandle_t handle (fvmGlobalAlloc (size, descr.c_str()));

          if (handle == 0)
            {
              std::ostringstream oss;

              oss << "alloc failed: " << descr << " (" << size << " bytes)";

              throw std::runtime_error (oss.str());
            }

          return static_cast<long> (handle);
        }

        inline void free (const long h)
        {
          const fvmAllocHandle_t handle (static_cast<fvmAllocHandle_t> (h));

          if (handle != 0)
            {
              fvmGlobalFree (handle);
            }

          return;
        }
      } // namespace global
    } // namespace memory
  } // namespace util
} // namespace ufbmig

#endif
