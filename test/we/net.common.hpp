// Copyright (C) 2016,2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/eureka.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/value.hpp>

#include <random>

  [[noreturn]] void unexpected_workflow_response
    (gspc::pnet::type::value::value_type const&, gspc::pnet::type::value::value_type const&);

  [[noreturn]] void unexpected_eureka (gspc::we::type::eureka_ids_type const&);

  std::mt19937& random_engine();

  gspc::we::type::property::type no_properties();
