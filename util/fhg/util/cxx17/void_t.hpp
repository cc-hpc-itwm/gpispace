// bernd.loerwald@itwm.fraunhofer.de

#pragma once

//! \note n3911: TransformationTrait Alias `void_t`

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename...> using void_t = void;
    }
  }
}
