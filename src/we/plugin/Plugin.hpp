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

#include <we/plugin/Base.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Plugin
      {
        Plugin (boost::filesystem::path, Context const&, PutToken);

        void before_eval (Context const&);
        void after_eval (Context const&);

      private:
        fhg::util::scoped_dlhandle _dlhandle;
        std::unique_ptr<Base> _;
      };
    }
  }
}
