#pragma once

#include <gspc/util/syscall.hpp>

#include <filesystem>
#include <string>
#include <vector>


  namespace gspc::util
  {
    //! A error-checking and scoped wrapper to the \c dlopen() family
    //! functions for runtime loading of shared objects.
    class scoped_dlhandle
    {
    public:
      //! Open the shared object at \a path, with \a flags passed to
      //! \c dlopen().
      scoped_dlhandle ( std::filesystem::path const& path
                      , int flags = RTLD_NOW | RTLD_DEEPBIND
                      );

      ~scoped_dlhandle();

      //! From the library loaded, retrieve the symbol identified by
      //! \a name, and cast it (unchecked!) to requested type \c T.
      template<typename T>
        T* sym (std::string const& name) const;

      scoped_dlhandle (scoped_dlhandle&&) noexcept;
      scoped_dlhandle (scoped_dlhandle const&) = delete;
      scoped_dlhandle& operator= (scoped_dlhandle const&) = delete;
      scoped_dlhandle& operator= (scoped_dlhandle&&) = delete;

    private:
      void* _;
    };

    //! Get the paths of all currently loaded libraries of this
    //! process.
    //! \note Not guaranteed to be absolute or canonical or existing:
    //! Libraries can be deleted after loading. Technically not
    //! required to have a name at all, so may contain empty paths.
    //! \note Not explicitly sorted by anything, neither by load order
    //! nor by path.
    std::vector<std::filesystem::path> currently_loaded_libraries();

    //! Given a \c scoped_dlhandle \a dlhandle_, call \c
    //! scoped_dlhandle::sym() with the name and type of the given \a
    //! symbol.
    //! \note The name is *not* mangled but taken verbatim,
    //! i.e. requires an 'extern "C"' specification on the symbol
    //! given.
#define FHG_UTIL_SCOPED_DLHANDLE_SYMBOL(dlhandle_, symbol_) \
    FHG_UTIL_SCOPED_DLHANDLE_SYMBOL_IMPL (dlhandle_, symbol_)
  }


#include <gspc/util/dynamic_linking.ipp>
