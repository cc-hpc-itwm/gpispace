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
  struct star_iterator : public it::it<typename Traits<MAP>::map_it_t>
  {
  private:
    typedef typename Traits<MAP>::ret_t ret_t;

    pos_t::const_iterator pos;

  public:
    star_iterator (const MAP & map, const pos_t & _pos)
      : star_iterator::super (map.begin(), map.end()), pos(_pos.begin())
    {}

    void operator ++ (void) 
    {
      if (star_iterator::super::has_more())
        {
          star_iterator::super::operator++();
          ++pos;
        }
    }
    ret_t operator * (void) const
    {
      return ret_t ( star_iterator::super::pos->first
                   , star_iterator::super::pos->second[*pos]
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

    typedef typename Traits<MAP>::vec_t vec_t;

    template<typename IT>
    vec_t get (IT it) const
    {
      vec_t v;
      
      for ( ; it.has_more(); ++it)
        v.push_back (*it);

      return v;
    }

  public:
    void rewind (void)
    {
      _size = (map.begin() != map.end()) ? 1 : 0;
      _has_more = (map.begin() != map.end());

      for (it_t m (map.begin()); m != map.end(); ++m)
        {
          pos.push_back (0);

          _size *= m->second.size();

          _has_more &= (!m->second.empty());
        }
    }

    explicit cross (const MAP & _map) : map (_map), pos () { rewind(); }

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

    vec_t get_vec (void) const { return get (operator * ()); }
    vec_t get_vec (const pos_t & p) const { return get (by (p)); }
  };
}

#endif
