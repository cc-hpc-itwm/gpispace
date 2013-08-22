// mirko.rahn@itwm.fraunhofer.de

#ifndef _BYTEARRAY_HPP
#define _BYTEARRAY_HPP

#include <algorithm>
#include <vector>

#include <iosfwd>
#include <sstream>

namespace bytearray
{
  class type
  {
  public:
    void push_back (char c);

    type();
    type (const type&);
    type (const char* const, const std::size_t);
    std::size_t copy (char* const buf, const std::size_t size) const;

    template<typename T>
    explicit type (const T* const x)
      : _v()
    {
      std::copy ((char *)x, (char *)x + sizeof (*x), std::back_inserter(_v));
    }
    template<typename T>
    std::size_t copy (T* const x) const
    {
      const std::size_t s (std::min (_v.size(), sizeof (*x)));

      std::copy (_v.begin(), _v.begin() + s, (char *)x);

      return s;
    }

    std::string to_string() const;

    friend std::ostream& operator<< (std::ostream&, const type&);
    friend std::size_t hash_value (const type&);
    friend bool operator== (const type&, const type&);
    friend bool operator< (const type&, const type&);

    type& operator= (const type& other);

    template<typename T>
    type& operator= (const T& other)
    {
      _v.clear();

      std::copy ( (char*)&other
                , (char*)&other + sizeof (T)
                , std::back_inserter(_v)
                );

      return *this;
    }

  private:
    std::vector<char> _v;
  };
}

#endif
