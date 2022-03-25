// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    template<typename T>
      T* scoped_dlhandle::sym (std::string const& name) const
    try
    {
      union
      {
        void* _ptr;
        T* _data;
      } symbol;

      symbol._ptr = syscall::dlsym (_, name.c_str());

      return symbol._data;
    }
    catch (...)
    {
      std::throw_with_nested
        (std::runtime_error ("get symbol '" + name + "'"));
    }

#define FHG_UTIL_SCOPED_DLHANDLE_SYMBOL_IMPL(dlhandle_, symbol_)  \
    (dlhandle_).sym<decltype (symbol_)> (#symbol_)
  }
}
