// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_DL_HPP
#define FHG_UTIL_DL_HPP

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <errno.h>
#include <string>

namespace dl
{
  class scoped_dlhandle
  {
  public:
    scoped_dlhandle (std::string const& path)
      : _ (dl_safe ("dlopen", &dlopen, path.c_str(), RTLD_NOW | RTLD_DEEPBIND))
    {}
    ~scoped_dlhandle()
    {
      dl_safe ("dlclose", &dlclose, _);
    }

    template<typename T> T* sym (std::string const& name) const
    {
      union
      {
        void* _ptr;
        T* _data;
      } sym;

      sym._ptr = dl_safe ("get symbol '" + name + "'", &dlsym, _, name.c_str());

      return sym._data;
    }

  private:
    void* _;

    template<typename Ret, typename... Args>
      static Ret dl_safe (std::string what, Ret (*fun)(Args...), Args... args)
    {
      dlerror();

      Ret ret (fun (args...));

      if (char* error = dlerror())
      {
        throw std::runtime_error ("'" + what + "': " + error);
      }

      return ret;
    }

    template<typename... Args>
      static void dl_safe (std::string what, void (*fun)(Args...), Args... args)
    {
      dlerror();

      fun (args...);

      if (char* error = dlerror())
      {
        throw std::runtime_error ("'" + what + "': " + error);
      }
    }
  };
}

#endif
