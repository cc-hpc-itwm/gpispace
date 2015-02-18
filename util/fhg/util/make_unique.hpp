// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <memory>

namespace fhg
{
  namespace util
  {
    //! \todo C++14: std::make_unique()
    template<class T, class... Args>
      std::unique_ptr<T> make_unique (Args&&... args)
    {
      return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
    }
  }
}
