#ifndef _SVECTOR_HPP
#define _SVECTOR_HPP

#include <vector>

// set with access to nth element
template<typename T>
struct svector
{
private:
  typedef typename std::vector<T> vec_t;
  typedef typename vec_t::iterator it;
  typedef typename vec_t::const_iterator const_it;
  typedef std::pair<it,it> pit_t;

  vec_t vec;
public:
  typedef typename vec_t::size_type size_type;

  it insert (const T & x)
  {
    const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

    return (std::distance (pit.first, pit.second) == 0)
      ? vec.insert (pit.second, x) : pit.first;
  }

  it erase (const T & x)
  {
    const it pos (std::lower_bound (vec.begin(), vec.end(), x));

    return (pos == vec.end()) ? pos : vec.erase (pos);
  }

  typename vec_t::const_reference & at (typename vec_t::size_type n) const
  {
    return vec.at (n);
  }

  const_it begin (void) const { return vec.begin(); }
  const_it end (void) const { return vec.end(); }

  const bool empty (void) const { return vec.empty(); }
  const size_type size (void) const { return vec.size(); }

  const bool operator == (const svector<T> & other) const
  {
    return (vec == other.vec);
  }
};
#endif // _SVECTOR_HPP
