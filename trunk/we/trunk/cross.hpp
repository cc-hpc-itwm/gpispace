// mirko.rahn@itwm.fraunhofer.de

#ifndef _CROSS_HPP
#define _CROSS_HPP

#include <cassert>

#include <vector>

namespace cross
{
  typedef std::vector<std::size_t> pos_t;

  template<typename MAP>
  struct star_iterator
  {
  private:
    typedef typename MAP::mapped_type::value_type val_t;
    typedef typename MAP::key_type key_t;
    typedef typename MAP::const_iterator map_it_t;
    typedef std::pair<key_t,val_t> ret_t;

    map_it_t m;
    const map_it_t m_end;
    pos_t::const_iterator pos;

  public:
    star_iterator (const MAP & map, const pos_t & _pos) 
      : m (map.begin())
      , m_end (map.end())
      , pos (_pos.begin())
    {}

    bool has_more (void) const { return (m != m_end) ? true : false; }
    void operator ++ (void) { ++pos; ++m; }
    ret_t operator * (void) const
    {
      assert (m != m_end);
      assert (*pos < m->second.size());

      return ret_t (m->first, m->second[*pos]);
    }
  };

  template<typename MAP>
  struct bracket_iterator
  {
  private:
    typedef typename MAP::mapped_type::value_type val_t;
    typedef typename MAP::key_type key_t;
    typedef typename MAP::const_iterator map_it_t;
    typedef std::pair<key_t,val_t> ret_t;

    map_it_t m;
    const map_it_t m_end;
    std::size_t k;

  public:
    bracket_iterator (const MAP & map, const std::size_t & _k) 
      : m (map.begin())
      , m_end (map.end())
      , k (_k)
    {}

    bool has_more (void) const { return (m != m_end) ? true : false; }
    void operator ++ (void) 
    {
      assert (m->second.size() > 0);

      k /= m->second.size();

      ++m;
    }
    ret_t operator * (void) const
    {
      assert (m != m_end);
      assert (m->second.size() > 0);

      return ret_t (m->first, m->second[k % m->second.size()]);
    }
  };

  template<typename MAP>
  class cross
  {
  private:
    typedef typename MAP::mapped_type::value_type val_t;
    typedef typename MAP::key_type key_t;
    typedef typename MAP::const_iterator map_it_t;
    typedef std::pair<key_t,val_t> ret_t;
    typedef std::vector<ret_t> vec_t;

    const MAP & map;
    pos_t pos;
    bool _has_more;
    std::size_t _size;

    void step (std::size_t mark, map_it_t it)
    {
      ++pos[mark];

      if (pos[mark] == it->second.size())
        {
          pos[mark] = 0;

          ++mark;
          ++it;

          if (it == map.end())
            {
              _has_more = false;
            }
          else
            {
              step (mark, it);
            }
        }
    }

  public:
    void rewind (void)
    {
      _size = 1;
      _has_more = true;

      for (map_it_t m (map.begin()); m != map.end(); ++m)
        {
          pos.push_back (0);

          _size *= m->second.size();

          _has_more &= (!m->second.empty());
        }
    }

    cross (const MAP & _map) : map (_map) { rewind(); }
    bool has_more (void) const { return _has_more; }
    // beware: overflow!
    std::size_t size (void) const { return _size; }
    void operator ++ () { step(0, map.begin()); }

    vec_t operator [] (std::size_t k)
    {
      vec_t v;

      for (map_it_t m (map.begin()); m != map.end(); ++m)
        {
          v.push_back (ret_t (m->first, m->second[k % m->second.size()]));

          k /= m->second.size();
        }

      return v;
    }

    vec_t operator * (void) const
    {
      vec_t v;
      pos_t::const_iterator p (pos.begin());

      for (map_it_t m (map.begin()); m != map.end(); ++m, ++p)
        v.push_back (ret_t (m->first, m->second[*p]));

      return v;
    }

    star_iterator<MAP> get_star_it (void) const
    {
      return star_iterator<MAP>(map,pos);
    }

    bracket_iterator<MAP> get_bracket_it (const std::size_t & k) const
    {
      return bracket_iterator<MAP>(map,k);
    }
  };
}

#endif
