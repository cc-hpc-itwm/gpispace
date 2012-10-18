// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_MAYBE_HPP
#define _FHG_UTIL_MAYBE_HPP 1

#include <fhg/util/show.hpp>

#include <boost/functional/hash.hpp>

#include <boost/variant.hpp>
#include <boost/call_traits.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      struct Nothing {};

      class isJust : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const Nothing &) const { return false; }

        template<typename T>
        bool operator () (const T &) const { return true; }
      };

      template<typename T>
      class get_with_default : public boost::static_visitor<T>
      {
      private:
        T dflt;

      public:
        get_with_default (T _dflt) : dflt (_dflt) {}

        T operator () (const Nothing &) const { return dflt; }
        T operator () (T x) const { return x; }
      };

      template<typename T>
      class get : public boost::static_visitor<T>
      {
      public:
        T operator () (const Nothing &) const
        {
          throw std::runtime_error ("maybe: Nothing");
        }
        T  operator () (T x) const { return x; }
      };
    }

    template<typename T>
    struct maybe
    {
    private:
      typedef boost::variant <detail::Nothing, T> maybe_type;
      typedef typename boost::call_traits<T>::const_reference const_reference;

      maybe_type m;

    public:
      maybe () : m () {}
      maybe (const_reference t) : m (t) {}

      operator bool (void) const
      {
        return boost::apply_visitor (detail::isJust(), m);
      }

      const_reference operator * (void) const
      {
        return boost::apply_visitor (detail::get<const_reference>(), m);
      }

      const_reference get_with_default (const_reference dflt) const
      {
        return boost::apply_visitor
          ( detail::get_with_default<const_reference> (dflt)
          , m
          );
      }

      void clear()
      {
        m = maybe_type(detail::Nothing());
      }

      maybe<T>& operator = (const_reference x)
      {
        m = x; return *this;
      }

      template<typename U>
      friend std::ostream & operator << (std::ostream &, const maybe<U> &);
    };

    template<typename T>
    inline std::size_t hash_value (const maybe<T> & x)
    {
      boost::hash<T> hasher;

      return x ? (1 + hasher (*x)) : 0;
    };

    template<typename T>
    inline bool operator == (const maybe<T> & x, const maybe<T> & y)
    {
      return x ? (y ? (*x == *y) : false) : (y ? false : true);
    }

    template<typename T>
    std::ostream & operator << (std::ostream & s, const maybe<T> & m)
    {
      return s << (m ? ("Just " + fhg::util::show(*m)) : "Nothing");
    };

    template<typename T>
    maybe<T> Nothing (void)
    {
      return maybe<T>();
    }

    template<typename T>
    maybe<T> Just (const T & t)
    {
      return maybe<T>(t);
    }

    template<typename T, typename U>
    maybe<U> fmap (U (*f)(const T &), const maybe<T> & m)
    {
      return m ? Just<U>(f (*m)) : Nothing<U>();
    }
  }
}

#endif
