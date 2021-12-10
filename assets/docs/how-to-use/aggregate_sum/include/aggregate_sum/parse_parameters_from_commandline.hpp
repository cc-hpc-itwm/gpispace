#pragma once

#include <aggregate_sum/Parameters.hpp>

namespace aggregate_sum
{
  Parameters parse_parameters_from_commandline
    (ParametersDescription const& execution_options,
     ParametersDescription const& workflow_options,
     int argc,
     char** argv
    );
}
