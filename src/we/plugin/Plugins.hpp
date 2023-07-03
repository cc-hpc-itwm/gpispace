// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/plugin/ID.hpp>
#include <we/plugin/Plugin.hpp>

#include <boost/filesystem/path.hpp>

#include <unordered_map>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Plugins
      {
        ID create (::boost::filesystem::path, Context const&, PutToken);
        void destroy (ID);

        void before_eval (ID, Context const&);
        void after_eval (ID, Context const&);

      private:
        ID _next_id = ID {0};
        std::unordered_map<ID, Plugin> _;

        decltype (_)::iterator at (ID);
      };
    }
  }
}
