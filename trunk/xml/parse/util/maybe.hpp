// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_MAYBE_HPP
#define _XML_PARSE_MAYBE_HPP

#include <we/util/show.hpp>

template<typename T>
struct maybe
{
private:
  T t;
  bool given;

public:
  maybe () : t(), given(false) {}
  maybe (const T & _t) : t (_t), given (true) {}

  bool isJust (void) const { return given; }
  bool isNothing (void) const { return !given; }

  const T & operator * (void) const
  {
    if (isNothing())
      {
        throw std::runtime_error ("maybe: Nothing");
      }

    return t;
  }

  template<typename U>
  friend std::ostream & operator << (std::ostream &, const maybe<U> &);
};

template<typename T>
std::ostream & operator << (std::ostream & s, const maybe<T> & m)
{
  return s << (m.isNothing() ? "Nothing" : ("Just " + ::util::show(m.t)));
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
