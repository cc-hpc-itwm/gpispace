// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_MAYBE_HPP
#define _XML_PARSE_MAYBE_HPP

#include <we/util/show.hpp>

#include <boost/functional/hash.hpp>

#include <boost/variant.hpp>

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

  class isNothing : public boost::static_visitor<bool>
  {
  public:
    bool operator () (const Nothing &) const { return true; }

    template<typename T>
    bool operator () (const T &) const { return false; }
  };

  template<typename T>
  class get_with_default : public boost::static_visitor<const T &>
  {
  private:
    const T & dflt;

  public:
    get_with_default (const T & _dflt) : dflt (_dflt) {}

    const T & operator () (const Nothing &) const { return dflt; }
    const T & operator () (const T & x) const { return x; }
  };

  template<typename T>
  class get : public boost::static_visitor<const T &>
  {
  public:
    const T & operator () (const Nothing &) const 
    {
      throw std::runtime_error ("maybe: Nothing");
    }
    const T & operator () (const T & x) const { return x; }
  };
}

template<typename T>
struct maybe
{
private:
  typedef boost::variant <detail::Nothing, T> maybe_type;

  maybe_type m;

public:
  maybe () : m () {}
  maybe (const T & t) : m (t) {}

  bool isJust (void) const 
  {
    return boost::apply_visitor (detail::isJust(), m);
  }

  bool isNothing (void) const 
  {
    return boost::apply_visitor (detail::isNothing(), m);
  }

  const T & operator * (void) const
  {
    return boost::apply_visitor (detail::get<T>(), m);
  }

  const T & get_with_default (const T & dflt) const
  {
    return boost::apply_visitor (detail::get_with_default<T> (dflt), m);
  }

  void operator = (const T & x)
  {
    m = x;
  }

  template<typename U>
  friend std::ostream & operator << (std::ostream &, const maybe<U> &);
};

template<typename T>
inline std::size_t hash_value (const maybe<T> & x)
{
  boost::hash<T> hasher;

  return x.isNothing() ? 0 : (1 + hasher (*x));
};

template<typename T>
inline bool operator == (const maybe<T> & x, const maybe<T> & y)
{
  return x.isNothing()
    ? (y.isNothing() ? true : false)
    : (y.isNothing() ? false : (*x == *y))
    ;
}

template<typename T>
std::ostream & operator << (std::ostream & s, const maybe<T> & m)
{
  return s << (m.isNothing() ? "Nothing" : ("Just " + ::util::show(*m)));
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
  return m.isNothing() ? Nothing<U>() : Just<U>(f (*m));
}

#endif
