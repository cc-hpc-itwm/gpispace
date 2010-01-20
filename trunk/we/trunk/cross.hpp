// mirko.rahn@itwm.fraunhofer.de

#ifndef _CROSS_HPP
#define _CROSS_HPP

#include <vector>

namespace cross
{
  template<typename MAP>
  class cross
  {
  private:
    typedef typename MAP::mapped_type::value_type val_t;
    typedef typename MAP::key_type key_t;
    typedef typename MAP::const_iterator it_t;
    typedef std::pair<key_t,val_t> ret_t;
    typedef std::vector<ret_t> vec_t;
    typedef std::vector<std::size_t> pos_t;

    const MAP & map;
    pos_t pos;
    bool _has_more;

  public:
    cross (const MAP & _map) 
      : map (_map)
      , pos ()
      , _has_more (true)
    {
      for (it_t m (map.begin()); m != map.end(); ++m)
        {
          pos.push_back (0);

          _has_more &= (!m->second.empty());
        }
    }

    bool has_more (void) const { return _has_more; }

    vec_t operator * (void) const
    {
      vec_t v;
      pos_t::const_iterator p (pos.begin());

      for (it_t m (map.begin()); m != map.end(); ++m, ++p)
        v.push_back (ret_t (m->first, m->second[*p]));

      return v;
    }

  private:
    void step (std::size_t mark, it_t it)
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
    void operator ++ ()
    {
      step(0, map.begin());
    }
  };
}

#endif
