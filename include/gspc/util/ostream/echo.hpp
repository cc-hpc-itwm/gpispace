#pragma once

#include <gspc/util/ostream/line_by_line.hpp>

#include <iostream>



    namespace gspc::util::ostream
    {
      class echo : private line_by_line, public std::ostream
      {
      public:
        echo (std::ostream& os = std::cout);
      };
    }
