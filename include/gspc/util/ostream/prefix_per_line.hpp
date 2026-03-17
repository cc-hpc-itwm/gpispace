#pragma once

#include <gspc/util/ostream/line_by_line.hpp>

#include <iostream>
#include <string>



    namespace gspc::util::ostream
    {
      class prefix_per_line : private line_by_line, public std::ostream
      {
      public:
        prefix_per_line (std::string, std::ostream& os = std::cout);
      };
    }
