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

  template<typename MAP, typename STATE>
  struct iterator : public it::it<typename Traits<MAP>::map_it_t>
  {
  protected:
    typedef typename Traits<MAP>::ret_t ret_t;

    STATE state;
    ret_t val;

    virtual ret_t get (void) const = 0;
    virtual void step (void) = 0;

    void set_val (void) { if (iterator::super::has_more()) val = get(); }

  public:
    iterator (const MAP & map, const STATE & _state)
      : iterator::super (map.begin(), map.end()), state(_state) {}

    void operator ++ (void) { step(); iterator::super::operator++(); set_val(); }
    ret_t operator * (void) const { return val; }
    const ret_t * operator -> (void) const { return &val; }
  };

  template<typename MAP>
  struct star_iterator : public iterator<MAP,pos_t::const_iterator>
  {
  private:
    typedef iterator<MAP,pos_t::const_iterator> super;
    
    typename super::ret_t get (void) const
    {
      return typename super::ret_t ( super::pos->first
                                   , super::pos->second[*super::state]
                                   );
    }
    void step (void) { ++super::state; }

  public:
    star_iterator (const MAP & _map, const pos_t & _pos)
      : super (_map, _pos.begin()) { super::set_val(); }
  };

  template<typename MAP>
  struct bracket_iterator : public iterator<MAP,std::size_t>
  {
  private:
    typedef iterator<MAP,std::size_t> super;

    typename super::ret_t get (void) const
    {
      return typename super::ret_t 
        ( super::pos->first
        , super::pos->second[super::state % super::pos->second.size()]
        );
    }
    void step (void) { super::state /= super::pos->second.size(); }

  public:
    bracket_iterator (const MAP & map, const std::size_t & k)
      : super (map, k) { super::set_val(); }
  };

  template<typename MAP>
  class cross
  {
  private:
    const MAP map;
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
      _size = 1;
      _has_more = true;

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

    bracket_iterator<MAP> operator [] (const std::size_t & k) const
    {
      return bracket_iterator<MAP>(map,k);
    }

    vec_t get_vec (void) const { return get (operator * ()); }
    vec_t get_vec (const pos_t & p) const { return get (by (p)); }
    vec_t get_vec (std::size_t k) const { return get (operator [] (k)); }
  };
}

#endif
