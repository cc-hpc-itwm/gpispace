#pragma once

#include <we/loader/IModule.hpp>

// see http://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
  #else
    #define DLL_PUBLIC
  #endif
#endif

#define WE_MOD_INITIALIZE_START(modname)\
  extern "C" \
  {\
     void DLL_PUBLIC we_mod_initialize(::we::loader::IModule *);   \
     void DLL_PUBLIC we_mod_initialize(::we::loader::IModule *mod)                \
     {\
        (void)(mod);                               \
        {volatile int _ = WE_GUARD_SYMBOL; (void)_;}

#define WE_REGISTER_FUN_AS(fun,as)             \
        mod->add_function(as, &fun)

#define WE_MOD_INITIALIZE_END(modname)\
    }\
  }
