// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <string>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace port
        {
          namespace orientation
          {
            enum type
              { NORTH
              , EAST
              , SOUTH
              , WEST
              };

            std::string show (const type&);
            type read (const std::string&);
            type invert (const type&);
          }
        }
      }
    }
  }
}
