// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/test/net.common.hpp>

#include <we/type/net.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/format.hpp>

#include <stdexcept>

  [[noreturn]] void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
  {
    throw std::logic_error ("got unexpected workflow_response");
  }

  [[noreturn]] void unexpected_eureka (we::type::eureka_ids_type const& ids)
  {
    throw std::logic_error
      (str ( ::boost::format ("Unexpected call to eureka: %1%")
           % fhg::util::print_container ("{", ", ", "}", ids)
           )
      );
  }

  std::mt19937& random_engine()
  {
    static std::mt19937 _
      (fhg::util::testing::detail::GLOBAL_random_engine()());

    return _;
  }

  we::type::property::type no_properties()
  {
    return {};
  }
