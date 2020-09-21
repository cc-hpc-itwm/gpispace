#include <iml/fuse/detail/PathOrHandle.hpp>

#include <sstream>
#include <string>

namespace iml
{
  namespace fuse
  {
    namespace detail
    {
      namespace
      {
        // \todo Move the logging endpoint parser to util-generic and
        // use here. Probably invent a better directory structure
        // before rewriting this quick hack.
        PathOrHandleBase parse (char const* path)
        {
          std::istringstream iss (path);

          char slash;
          if (!(iss >> slash) || slash != '/')
          {
            return Unknown {path};
          }
          else if (iss.peek() == std::char_traits<char>::eof())
          {
            return TopLevel{};
          }

          SegmentHandle segment;
          if (!(iss >> segment))
          {
            return Unknown {path};
          }
          else if (iss.peek() == std::char_traits<char>::eof())
          {
            return segment;
          }
          if (!(iss >> slash) || slash != '/')
          {
            return Unknown {path};
          }
          else if (iss.peek() == std::char_traits<char>::eof())
          {
            return segment;
          }

          // \todo Don't ignore segment ID: Implementations need to be
          // able to check if the allocation exists *in that segment*.

          AllocationHandle allocation;
          if (!(iss >> allocation))
          {
            return Unknown {path};
          }
          else if (iss.peek() == std::char_traits<char>::eof())
          {
            return allocation;
          }

          return Unknown {path};
        }
      }

      PathOrHandle::PathOrHandle (char const* path)
        : PathOrHandleBase (parse (path))
      {}
    }
  }
}
