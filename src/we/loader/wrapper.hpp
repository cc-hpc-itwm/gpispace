#pragma once

#include <we/expr/eval/context.hpp>

#include <boost/version.hpp>

#include <map>
#include <string>

extern "C"
{
#define WE_MAKE_SYMBOL_NAME(PRE,POST)           \
  PRE ## POST
#define WE_MAKE_SYMBOL(BASE,TAG)                \
  WE_MAKE_SYMBOL_NAME (BASE,TAG)
#define WE_GUARD_SYMBOL                                                 \
  WE_MAKE_SYMBOL (MODULE_DETECTED_INCOMPATIBLE_BOOST_VERSION_, BOOST_VERSION)

  extern const int WE_GUARD_SYMBOL;
}

namespace drts { namespace worker { class context; } }

namespace we
{
  namespace loader
  {
    typedef void (*WrapperFunction) ( drts::worker::context*
                                    , expr::eval::context const&
                                    , expr::eval::context&
                                    , std::map<std::string, void*> const&
                                    );

    class IModule
    {
    public:
      virtual ~IModule() {}

      virtual void add_function (std::string const&, WrapperFunction) = 0;
    };
  }
}

//! \todo support gcc-frontend-incompatible compilers
#define DLL_PUBLIC __attribute__ ((visibility ("default")))

#define WE_MOD_INITIALIZE_START(modname)                           \
  extern "C"                                                       \
  {                                                                \
    void DLL_PUBLIC we_mod_initialize (::we::loader::IModule*);    \
    void DLL_PUBLIC we_mod_initialize (::we::loader::IModule* mod) \
    {                                                              \
      { volatile int _ = WE_GUARD_SYMBOL; (void) _; }              \
      (void) (mod)

#define WE_REGISTER_FUN_AS(fun,as)                                 \
        mod->add_function (as, &fun)

#define WE_MOD_INITIALIZE_END(modname)                             \
    }                                                              \
  }
