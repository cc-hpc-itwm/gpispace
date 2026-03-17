#include <gspc/util/map_failed.hpp>

#if !HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST
# include <gspc/util/warning.hpp>
# define SUPPRESS_OLD_STYLE_CAST_WARNING(what...)       \
  DISABLE_WARNING_GCC ("-Wold-style-cast")              \
  what                                                  \
  RESTORE_WARNING_GCC ("-Wold-style-cast")
#else
# define SUPPRESS_OLD_STYLE_CAST_WARNING(what...)       \
  what
#endif


  namespace gspc::util
  {
    void* map_failed()
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return MAP_FAILED;)
    }
  }
