// mirko.rahn@itwm.fraunhofer.de

#ifndef _CROSS_HPP
#define _CROSS_HPP

#include <util.hpp>

#include <cassert>

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
  struct star_iterator : public util::it<typename Traits<MAP>::map_it_t>
  {
  private:
    typedef util::it<typename Traits<MAP>::map_it_t> super;
    typedef typename Traits<MAP>::ret_t ret_t;

    pos_t::const_iterator pos;
#ifndef NDEBUG
    const pos_t::const_iterator end;
#endif
    ret_t val;

    void set_val (void)
    {
      if (super::has_more())
        {
          const typename MAP::const_iterator & m (super::pos);

          assert (pos != end);
          assert (*pos < m->second.size());

          val = typename Traits<MAP>::ret_t (m->first, m->second[*pos]);
        }
    }

  public:
    star_iterator (const MAP & map, const pos_t & _pos)
      : super (map.begin(), map.end())
      , pos (_pos.begin())
#ifndef NDEBUG
      , end (_pos.end())
#endif
    {
      set_val();
    }
    void operator ++ (void) { ++pos; super::operator++(); set_val(); }
    ret_t operator * (void) const { return val; }
    const ret_t * operator -> (void) const { return &val; }
  };

  template<typename MAP>
  struct bracket_iterator : public util::it<typename Traits<MAP>::map_it_t>
  {
  private:
    typedef util::it<typename Traits<MAP>::map_it_t> super;

    std::size_t k;

  public:
    bracket_iterator (const MAP & map, const std::size_t & _k)
      : super (map.begin(), map.end())
      , k (_k)
    {}
    void operator ++ (void)
    {
      const typename MAP::const_iterator & m (super::pos);

      assert (m->second.size() > 0);

      k /= m->second.size();

      super::operator++();
    }
    typename Traits<MAP>::ret_t operator * (void) const
    {
      const typename MAP::const_iterator & m (super::pos);

      assert (m->second.size() > 0);

      return typename Traits<MAP>::ret_t ( m->first
                                         , m->second[k % m->second.size()]
                                         );
    }
  };

  template<typename MAP>
  class cross
  {
  private:
    const MAP & map;
    pos_t pos;
    bool _has_more;
    std::size_t _size;

    void step (std::size_t mark, typename Traits<MAP>::map_it_t it)
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

      for (typename Traits<MAP>::map_it_t m (map.begin()); m != map.end(); ++m)
        {
          pos.push_back (0);

          _size *= m->second.size();

          _has_more &= (!m->second.empty());
        }
    }

    cross (const MAP & _map) : map (_map), pos () { rewind(); }
    bool has_more (void) const { return _has_more; }
    // beware: overflow!
    std::size_t size (void) const { return _size; }
    void operator ++ () { step(0, map.begin()); }

    star_iterator<MAP> operator * (void) const
    {
      return star_iterator<MAP>(map,pos);
    }

    star_iterator<MAP> by (const pos_t & p) const
    {
      return star_iterator<MAP>(map,p);
    }

    bracket_iterator<MAP> operator [] (const std::size_t & k) const
    {
      return bracket_iterator<MAP>(map,k);
    }

    template<typename IT>
    typename Traits<MAP>::vec_t gen_operator (IT & it) const
    {
      typename Traits<MAP>::vec_t v;

      for ( ; it.has_more(); ++it)
        v.push_back (*it);

      return v;
    }

    typename Traits<MAP>::vec_t get_vec (void) const
    {
      star_iterator<MAP> s (operator * ());

      return gen_operator (s);
    }

    typename Traits<MAP>::vec_t get_vec (const pos_t & p) const
    {
      star_iterator<MAP> s (by (p));

      return gen_operator (s);
    }

    typename Traits<MAP>::vec_t get_vec (std::size_t k) const
    {
      bracket_iterator<MAP> b (operator [] (k));

      return gen_operator (b);
    }
  };
}

#endif
