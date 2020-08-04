#include <gspc/installation_sentinel.hpp>

#include <util-generic/executable_path.hpp>

namespace gspc
{
  namespace
  {
    template<int N>
      boost::filesystem::path nth_parent_path (boost::filesystem::path path)
    {
      for (int i (0); i < N; ++i)
      {
        path = path.parent_path();
      }
      return path;
    }
  }

  boost::filesystem::path installation_prefix()
  {
    return nth_parent_path<GPISPACE_INSTALLATION_SENTINEL_SUBDIR_COUNT>
      (fhg::util::executable_path (&installation_prefix));
  }
}
