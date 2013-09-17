// alexander.petry@itwm.fraunhofer.de

#ifndef WE_LOADER_MACROS_HPP
#define WE_LOADER_MACROS_HPP 1

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
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

#define WE_MOD_INITIALIZE_START(modname)\
  extern "C" \
  {\
     void DLL_PUBLIC we_mod_initialize(::we::loader::IModule *, unsigned int);   \
     void DLL_PUBLIC we_mod_initialize(::we::loader::IModule *mod, unsigned int)                \
     {\
        (void)(mod);                               \
        {volatile int _ = WE_GUARD_SYMBOL; (void)_;} \
        mod->name (#modname)

#define WE_REGISTER_FUN_START(fun)                \
  {                                               \
    ::we::loader::param_names_list_t params

#define WE_ADD_INP(p, typ)\
           params.push_back(p)
#define WE_ADD_OUT(p, typ)\
           params.push_back(p)

#define WE_REGISTER_FUN_END(fun)\
           mod->add_function(#fun, &fun, params);\
        }

#define WE_REGISTER_FUN(fun)                   \
        mod->add_function(#fun, &fun)
#define WE_REGISTER_FUN_AS(fun,as)             \
        mod->add_function(as, &fun)

#define WE_SET_STATE(s)\
        mod->state (s)

#define WE_MOD_INITIALIZE_END(modname)\
    }\
  }

#define WE_MOD_FINALIZE_START(modname)\
  extern "C"\
  {\
     void DLL_PUBLIC we_mod_finalize(::we::loader::IModule *);  \
     void DLL_PUBLIC we_mod_finalize(::we::loader::IModule *mod)\
     {\
        (void)(mod)

#define WE_GET_STATE()\
         mod->state()

#define WE_CLEAR_STATE()\
         mod->state(0)

#define WE_MOD_FINALIZE_END(modname)\
     }\
  }

#endif
