// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/util/it.hpp>

#include <vector>

#include <cassert>

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
    ret_t operator * (void) const
    {
      return ret_t (pos->first, pos->second[*state % pos->second.size()]);
    }
  };

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

    void rewind (void)
    {
      pos.clear();

      _has_more = (map.begin() != map.end());

      pos_t::const_iterator s (shift.begin());

      for (it_t m (map.begin()); m != map.end(); ++m, ++s)
        {
          assert (s != shift.end());

          pos.push_back (*s);

          _has_more &= (!m->second.empty());
        }
    }

  public:
    explicit cross (const MAP & _map)
      : map (_map)
      , shift (map.size())
    {
      std::fill (shift.begin(), shift.end(), 0);

      rewind();
    }

    // const and non-const version since the compiler chooses the
    // non-template function first
    cross (const MAP & _map, const pos_t & _shift)
      : map (_map)
      , shift (_shift)
    {
      assert (shift.size() == map.size());

      rewind();
    }

    cross (const MAP & _map, pos_t & _shift)
      : map (_map)
      , shift (_shift)
    {
      assert (shift.size() == map.size());

      rewind();
    }

    template<typename Engine>
    cross (const MAP & _map, Engine & engine)
      : map (_map)
    {
      for (it_t m (map.begin()); m != map.end(); ++m)
        {
          boost::uniform_int<> dist (0, m->second.size() - 1);
          shift.push_back (dist (engine));
        }

      rewind();
    }

    bool has_more (void) const { return _has_more; }
    void operator ++ () { step (0, map.begin(), shift.begin()); }

    unsigned long size (void) const
    {
      unsigned long size (1);

      for (it_t pos (map.begin()); pos != map.end(); ++pos)
	{
	  size *= pos->second.size();
	}

      return size;
    }

    iterator<MAP> operator * (void) const { return iterator<MAP> (map, pos); }
  };
}

#endif
