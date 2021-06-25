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

#include <gspc/detail/dllexport.hpp>

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>

#include <drts/worker/context_fwd.hpp>

#include <map>
#include <string>

namespace we
{
  namespace loader
  {
    typedef void (*WrapperFunction)( drts::worker::context *
                                   , const expr::eval::context&
                                   , expr::eval::context&
                                   , std::map<std::string, void*> const&
                                   );

    class GSPC_DLLEXPORT IModule
    {
    public:
      virtual ~IModule() {}

      virtual void add_function (const std::string&, WrapperFunction) = 0;
    };
  }
}
