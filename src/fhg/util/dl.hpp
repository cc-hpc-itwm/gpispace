// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <errno.h>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    class scoped_dlhandle
    {
    public:
      scoped_dlhandle (std::string const& path)
        : _ (dl_safe ("dlopen", &dlopen, path.c_str(), RTLD_NOW | RTLD_DEEPBIND))
      {}
      ~scoped_dlhandle()
      {
        if (_)
        {
          dl_safe ("dlclose", &dlclose, _);
        }
      }

      scoped_dlhandle (scoped_dlhandle&& other)
        : _ (std::move (other._))
      {
        other._ = nullptr;
      }

      scoped_dlhandle (scoped_dlhandle const&) = delete;
      scoped_dlhandle& operator= (scoped_dlhandle const&) = delete;
      scoped_dlhandle& operator= (scoped_dlhandle&&) = delete;

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
}
