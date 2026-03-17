// Copyright (C) 2014-2015,2021-2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/drts/client.fwd.hpp>
#include <gspc/drts/drts.fwd.hpp>
#include <gspc/drts/pimpl.hpp>

#include <string>

namespace gspc
{
  class GSPC_EXPORT information_to_reattach
  {
  public:
    information_to_reattach (scoped_runtime_system const&);
    information_to_reattach (std::string const&);
    std::string to_string () const;

    information_to_reattach (information_to_reattach const&) = delete;
    information_to_reattach& operator= (information_to_reattach const&) = delete;
    information_to_reattach (information_to_reattach&&) = default;
    information_to_reattach& operator= (information_to_reattach&&) = default;

  private:
    friend class ::gspc::client;

    PIMPL (information_to_reattach);
  };
}
