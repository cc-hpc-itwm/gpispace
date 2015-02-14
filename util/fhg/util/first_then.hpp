// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhg/util/ostream_modifier.hpp>

#include <functional>
#include <iostream>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class first_then : public ostream::modifier
    {
    public:
      first_then (const T& f, const T& t)
        : _value (f)
        , _modify (std::bind (&first_then::set, this, t))
      {}
      virtual std::ostream& operator() (std::ostream& os) const override
      {
        os << _value;
        _modify();
        return os;
      }

    private:
      mutable T _value;
      mutable std::function<void ()> _modify;

      void set (const T& value) const
      {
        _value = value;
        _modify = &first_then::nop;
      }
      static void nop() {}
    };
  }
}
