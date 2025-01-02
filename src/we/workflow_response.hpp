// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <string>

namespace we
{
  using workflow_response_callback
    = std::function<void ( pnet::type::value::value_type const& description
                         , pnet::type::value::value_type const& value
                         )>;

  std::string get_response_id (pnet::type::value::value_type const&);
  pnet::type::value::value_type make_response_description
    (std::string response_id, pnet::type::value::value_type const& value);

  bool is_response_description
    (pnet::type::signature::signature_type const&);

  static inline std::string response_description_requirements()
  {
    return "the field 'response_id :: string'";
  }
}
