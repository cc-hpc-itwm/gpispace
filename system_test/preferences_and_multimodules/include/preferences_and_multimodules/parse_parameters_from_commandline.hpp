// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <preferences_and_multimodules/parameters.hpp>

namespace preferences_and_multimodules
{
  Parameters parse_parameters_from_commandline
    ( ParametersDescription const& execution_options
    , ParametersDescription const& workflow_options
    , int argc
    , char** argv
    );
}
