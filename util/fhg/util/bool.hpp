#ifndef FHG_UTIL_BOOL_HPP
#define FHG_UTIL_BOOL_HPP

namespace fhg
{
  namespace util
  {
    struct bool_t
    {
      bool_t ()
        : v (false)
      {}

      bool_t (bool b)
        : v (b)
      {}

      bool_t (bool_t const &other)
        : v (other.v)
      {}

      operator bool() const
      {
        return v;
      }

      bool_t & operator= (const bool_t &rhs)
      {
        v = rhs.v;
        return *this;
      }
      bool_t & operator= (const bool rhs)
      {
        v = rhs;
        return *this;
      }
    private:
      bool v;
    };
  }
}

#endif // FHG_UTIL_BOOL_HPP
