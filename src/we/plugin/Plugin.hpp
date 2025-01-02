// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
        Plugin (::boost::filesystem::path, Context const&, PutToken);

        void before_eval (Context const&);
        void after_eval (Context const&);

      private:
        fhg::util::scoped_dlhandle _dlhandle;
        std::unique_ptr<Base> _;
      };
    }
  }
}
