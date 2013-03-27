#ifndef WE_LOADER_API_GUARD_HPP
#define WE_LOADER_API_GUARD_HPP

#include <boost/version.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#define WE_MAKE_SYMBOL_NAME(PRE,POST)                                          \
  PRE ## POST
#define WE_MAKE_SYMBOL(BASE,TAG)                                               \
  WE_MAKE_SYMBOL_NAME(BASE,TAG)
#define WE_GUARD_SYMBOL                                                        \
  WE_MAKE_SYMBOL(MODULE_DETECTED_INCOMPATIBLE_BOOST_VERSION_,BOOST_VERSION)

  extern const int WE_GUARD_SYMBOL;

#ifdef __cplusplus
}
#endif

#endif
