// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/util/it.hpp>

#include <vector>

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
    const val_t & val (void) const { return pos->second[*state]; }
    ret_t operator * (void) const
    {
      return ret_t (pos->first, pos->second[*state]);
    }
  };

  template<typename MAP>
  class cross
  {
  private:
    const MAP & map;
    pos_t pos;
    bool _has_more;

    typedef typename Traits<MAP>::map_it_t it_t;

    void step (std::size_t slot, it_t it)
    {
      ++pos[slot];

      if (pos[slot] == it->second.size())
        {
          pos[slot] = 0;

          ++slot;
          ++it;

          if (it == map.end())
            {
              _has_more = false;
            }
          else
            {
              step (slot, it);
            }
        }
    }

  public:
    void rewind (void)
    {
      _has_more = (map.begin() != map.end());

      for (it_t m (map.begin()); m != map.end(); ++m)
        {
          pos.push_back (0);

          _has_more &= (!m->second.empty());
        }
    }

    explicit cross (const MAP & _map) : map (_map), pos () { rewind(); }

    bool has_more (void) const { return _has_more; }
    void operator ++ () { step(0, map.begin()); }

    iterator<MAP> operator * (void) const { return iterator<MAP>(map,pos); }

//     void get_vec (typename Traits<MAP>::vec_t & v) const
//     {
//       v.clear();

//       for (iterator<MAP> it (map, pos); it.has_more(); ++it)
//         v.push_back (*it);
//     }
  };
}

#endif
