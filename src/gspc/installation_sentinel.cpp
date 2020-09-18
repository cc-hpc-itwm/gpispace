#include <gspc/installation_sentinel.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/getenv.hpp>

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
    auto const override_path
      (fhg::util::getenv ("GSPC_TESTING_OVERRIDE_INSTALLATION_PREFIX"));
    if (override_path)
    {
      return *override_path;
    }

    return nth_parent_path<GPISPACE_INSTALLATION_SENTINEL_SUBDIR_COUNT>
      (fhg::util::executable_path (&installation_prefix));
  }
}
