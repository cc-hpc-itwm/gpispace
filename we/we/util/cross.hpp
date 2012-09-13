// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/util/it.hpp>

#include <numeric>
#include <vector>

#include <fhg/assert.hpp>

#include <boost/random.hpp>

namespace cross
{
  typedef std::vector<std::size_t> pos_t;

  template<typename MAP>
  struct Traits
  {
  public:
    typedef typename MAP::mapped_type::value_type val_t;
    typedef typename MAP::key_type key_t;
    typedef typename MAP::const_iterator map_it_t;
    typedef std::pair<key_t,val_t> ret_t;
    typedef std::vector<ret_t> vec_t;
  };

  template<typename MAP>
  struct iterator
  {
  private:
    typedef typename Traits<MAP>::map_it_t map_it_t;
    typedef typename Traits<MAP>::ret_t ret_t;
    typedef typename Traits<MAP>::key_t key_t;
    typedef typename Traits<MAP>::val_t val_t;

    map_it_t pos;
    const map_it_t end;
    pos_t::const_iterator state;

  public:
    iterator (const MAP & map, const pos_t & _pos)
      : pos (map.begin())
      , end (map.end())
      , state(_pos.begin())
    {}

    bool has_more (void) const { return pos != end; }

    void operator ++ (void) { if (has_more()) { ++pos; ++state; } }
    const key_t & key (void) const { return pos->first; }
    const val_t & val (void) const
    {
      return pos->second[*state % pos->second.size()];
    }
    ret_t operator * (void) const { return ret_t (key(), val()); }
  };

  namespace pred
  {
    template<typename MAP>
    bool second_empty (const typename MAP::const_iterator::value_type& v)
    {
      return v.second.empty();
    }
  }

  namespace binop
  {
    template<typename T, typename MAP>
    T product (T x, const typename MAP::const_iterator::value_type& v)
    {
      return x * v.second.size();
    }
  }

  template<typename MAP>
  class cross
  {
  private:
    const MAP & map;
    pos_t pos;
    pos_t shift;
    bool _has_more;

    typedef typename Traits<MAP>::map_it_t it_t;

    void step (std::size_t slot, it_t it, pos_t::const_iterator s)
    {
      ++pos[slot];

      if (pos[slot] == it->second.size() + *s)
        {
          pos[slot] = *s;

          ++slot;
          ++it;
          ++s;

          if (it == map.end())
            {
              _has_more = false;
            }
          else
            {
              step (slot, it, s);
            }
        }
    }

    bool determine_has_more (void) const
    {
      return
        (not map.empty())
        &&
        (  std::find_if (map.begin(), map.end(), pred::second_empty<MAP>)
        == map.end()
        )
        ;
    }

  public:
    explicit cross (const MAP & _map)
      : map (_map)
      , pos (map.size(), 0)
      , shift (map.size(), 0)
      , _has_more (determine_has_more())
    {}

    // const and non-const version since the compiler chooses the
    // non-template function first
    cross (const MAP & _map, const pos_t & _shift)
      : map (_map)
      , pos (_shift)
      , shift (_shift)
      , _has_more (determine_has_more())
    {
      assert (shift.size() == map.size());
    }

    cross (const MAP & _map, pos_t & _shift)
      : map (_map)
      , pos (_shift)
      , shift (_shift)
      , _has_more (determine_has_more())
    {
      assert (shift.size() == map.size());
    }

    template<typename Engine>
    cross (const MAP & _map, Engine & engine)
      : map (_map)
      , pos (map.size())
      , shift (map.size())
      , _has_more (determine_has_more())
    {
      pos_t::iterator s (shift.begin());
      pos_t::iterator p (pos.begin());

      for (it_t m (map.begin()), end (map.end()); m != end; ++m)
        {
          boost::uniform_int<> dist (0, m->second.size() - 1);
          *s = dist (engine);
          *p++ = *s++;
        }
    }

    bool has_more (void) const { return _has_more; }
    void operator ++ () { step (0, map.begin(), shift.begin()); }

    unsigned long size (void) const
    {
      return std::accumulate ( map.begin(), map.end()
                             , 1UL
                             , binop::product<unsigned long, MAP>
                             );
    }

    iterator<MAP> operator * (void) const { return iterator<MAP> (map, pos); }
  };
}

#endif
