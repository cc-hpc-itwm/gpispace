// Copyright (C) 2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/net.common.hpp>

#include <gspc/we/type/net.hpp>

#include <gspc/util/print_container.hpp>
#include <gspc/testing/random.hpp>

#include <gspc/util/join.formatter.hpp>
#include <fmt/core.h>
#include <stdexcept>

  [[noreturn]] void unexpected_workflow_response
    (gspc::pnet::type::value::value_type const&, gspc::pnet::type::value::value_type const&)
  {
    throw std::logic_error ("got unexpected workflow_response");
  }

  [[noreturn]] void unexpected_eureka (gspc::we::type::eureka_ids_type const& ids)
  {
    throw std::logic_error
      { fmt::format ( "Unexpected call to eureka: {}"
                    , gspc::util::print_container ("{", ", ", "}", ids)
                    )
      };
  }

  std::mt19937& random_engine()
  {
    static std::mt19937 _
      (gspc::testing::detail::GLOBAL_random_engine()());

    return _;
  }

  gspc::we::type::property::type no_properties()
  {
    return {};
  }
