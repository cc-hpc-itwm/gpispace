#include <gspc/util/sig_err.hpp>

#if !HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST
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
    sighandler_t sig_err()
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return SIG_ERR;)
    }
  }
