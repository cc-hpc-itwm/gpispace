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

      operator bool() const
      {
        return v;
      }

    private:
      bool v;
    };
  }
}

#endif // FHG_UTIL_BOOL_HPP
