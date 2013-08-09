// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/get.hpp>

#include <we/type/value/show.hpp>
#include <we/type/literal/show.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      empty_path::empty_path()
        : std::runtime_error ("get: empty path")
      {}
    }
  }
}
