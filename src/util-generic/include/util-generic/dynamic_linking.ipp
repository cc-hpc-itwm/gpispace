// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
