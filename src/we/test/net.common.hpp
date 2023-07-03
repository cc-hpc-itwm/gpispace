// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/eureka.hpp>
#include <we/type/property.hpp>
#include <we/type/value.hpp>

#include <random>

  [[noreturn]] void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&);

  [[noreturn]] void unexpected_eureka (we::type::eureka_ids_type const&);

  std::mt19937& random_engine();

  we::type::property::type no_properties();
