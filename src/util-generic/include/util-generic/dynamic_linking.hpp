// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <util-generic/syscall.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    //! A error-checking and scoped wrapper to the \c dlopen() family
    //! functions for runtime loading of shared objects.
    class scoped_dlhandle
    {
    public:
      //! Open the shared object at \a path, with \a flags passed to
      //! \c dlopen().
      scoped_dlhandle ( boost::filesystem::path const& path
                      , int flags = RTLD_NOW | RTLD_DEEPBIND
                      );
      ~scoped_dlhandle();

      //! From the library loaded, retrieve the symbol identified by
      //! \a name, and cast it (unchecked!) to requested type \c T.
      template<typename T>
        T* sym (std::string const& name) const;

      scoped_dlhandle (scoped_dlhandle&&);
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
    std::vector<boost::filesystem::path> currently_loaded_libraries();

    //! Given a \c scoped_dlhandle \a dlhandle_, call \c
    //! scoped_dlhandle::sym() with the name and type of the given \a
    //! symbol.
    //! \note The name is *not* mangled but taken verbatim,
    //! i.e. requires an 'extern "C"' specification on the symbol
    //! given.
#define FHG_UTIL_SCOPED_DLHANDLE_SYMBOL(dlhandle_, symbol_) \
    FHG_UTIL_SCOPED_DLHANDLE_SYMBOL_IMPL (dlhandle_, symbol_)
  }
}

#include <util-generic/dynamic_linking.ipp>
