#ifndef _UTIL_HPP
#define _UTIL_HPP 1

#include <pnetc/type/loaded_package.hpp>
#include <pnetc/type/assigned_package.hpp>
#include <pnetc/type/interval.hpp>
#include <pnetc/type/package.hpp>

namespace util
{
  inline long size
  (const ::pnetc::type::package::package package)
  {
    return package.right.trace - package.left.trace;
  }

  inline long size
  (const ::pnetc::type::assigned_package::assigned_package package)
  {
    return size (package.package);
  }

  inline long size
  (const ::pnetc::type::loaded_package::loaded_package package)
  {
    return size (package.assigned_package);
  }

  class interval_iterator
  {
  public:
    interval_iterator
    (const ::pnetc::type::assigned_package::assigned_package & p)
      : end_offset (p.intervals.offset.end())
      , end_size (p.intervals.size.end())
      , pos_offset (p.intervals.offset.begin())
      , pos_size (p.intervals.size.begin())
    {
      validate();
    }

    bool has_more () const { return pos_offset != end_offset; }
    void operator ++ () { ++pos_offset; ++pos_size; validate(); }
    ::pnetc::type::interval::interval operator * () const
    {
      ::pnetc::type::interval::interval i;

      i.offset = *pos_offset;
      i.size = *pos_size;

      return i;
    }

  private:
    typedef literal::stack_type::const_iterator it_t;

    const it_t & end_offset;
    const it_t & end_size;
    it_t pos_offset;
    it_t pos_size;

    void validate () const
    {
      if (end_offset == pos_offset && end_size != pos_size)
        {
          throw std::runtime_error ("more sizes than offsets");
        }
      else if (end_offset != pos_offset && end_size == pos_size)
        {
          throw std::runtime_error ("more offsets than sizes");
        }
    }
  };
}

#endif
