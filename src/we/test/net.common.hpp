// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/eureka.hpp>
#include <we/type/property.hpp>
#include <we/type/value.hpp>

#include <random>

  [[noreturn]] void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&);

  [[noreturn]] void unexpected_eureka (we::type::eureka_ids_type const&);

  std::mt19937& random_engine();

  we::type::property::type no_properties();
