// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_MAYBE_HPP
#define _FHG_UTIL_MAYBE_HPP 1

#include <fhg/util/boost.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    template<typename T>
    struct maybe
    {
    public:
      maybe () : _impl (boost::none) {}
      maybe (const T& t) : _impl (t) {}
      maybe (const boost::none_t&) : _impl (boost::none) {}
      maybe (const boost::optional<T>& impl) : _impl (impl) {}

      operator bool (void) const
      {
        return _impl;
      }

      const T& operator* (void) const
      {
        return *_impl;
      }

      const T& get_value_or (const T& dflt) const
      {
        return _impl.get_value_or (dflt);
      }

      maybe<T>& operator= (const T& x)
      {
        _impl = x; return *this;
      }

    private:
      template<typename U>
        friend std::ostream& operator<< (std::ostream &, const maybe<U> &);
      template<typename U>
        friend bool operator== (const maybe<U>&, const maybe<U> &);
      template<typename U>
        inline std::size_t hash_value (const maybe<U> & x);
      template<typename U, typename V>
        friend maybe<V> fmap (V (*f)(const U &), const maybe<U> & m);
      template<typename U>
        friend void fmap (void (*f)(const U &), const maybe<U> & m);

      boost::optional<T> _impl;
    };

    template<typename T>
    std::ostream & operator<< (std::ostream & s, const maybe<T> & m)
    {
      return s << m._impl;
    };

    template<typename T>
    inline std::size_t hash_value (const maybe<T> & x)
    {
      return hash_value (x._impl);
    };

    template<typename T>
    inline bool operator== (const maybe<T> & x, const maybe<T> & y)
    {
      return x._impl == y._impl;
    }

    template<typename T, typename U>
      maybe<U> fmap (U (*f)(const T &), const maybe<T> & m)
    {
      return fhg::util::boost::fmap<T,U> (f, m._impl);
    }

    template<typename T>
      void fmap (void (*f)(const T &), const maybe<T> & m)
    {
      fhg::util::boost::fmap<T> (f, m._impl);
    }
  }
}

#endif
