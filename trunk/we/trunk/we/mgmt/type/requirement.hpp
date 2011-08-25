#ifndef WE_MGMT_TYPE_REQUIREMENT_HPP
#define WE_MGMT_TYPE_REQUIREMENT_HPP 1

#include <vector>
#include <ostream>
#include <boost/unordered_set.hpp>
#include <fhg/util/show.hpp>

namespace we
{
  namespace mgmt
  {
    template <typename T>
    struct requirement_t
    {
      typedef T value_type;
      typedef value_type argument_type;

      template <typename U>
      struct rebind
      {
        typedef requirement_t<U> other;
      };

      explicit
      requirement_t (value_type arg, const bool _mandatory = false)
        : value_(arg)
        , mandatory_(_mandatory)
      {}

      requirement_t (requirement_t<T> const &other)
        : value_(other.value_)
        , mandatory_(other.mandatory_)
      {}

      requirement_t<T> & operator=(requirement_t<T> const & rhs)
      {
        this->value_ = rhs.value_;
        this->mandatory_ = rhs.mandatory_;
        return *this;
      }

      ~requirement_t () {}

      virtual bool is_mandatory (void) const
      {
        return mandatory_;
      }

      const value_type & value(void) const
      {
        return value_;
      }

      void value(const value_type & val)
      {
        value_ = val;
      }
    private:
      bool mandatory_;
      value_type value_;
    };

    template <typename T>
    requirement_t<T> make_mandatory (T val)
    {
      return requirement_t<T> (val, true);
    }

    template <typename T>
    requirement_t<T> make_optional (T val)
    {
      return requirement_t<T> (val, false);
    }
  }
}

#endif
