// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/id.hpp>

#include <numeric>
#include <vector>

#include <fhg/assert.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>

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

    bool eval
    ( const expr::parse::parser& parser
    , boost::function<std::string (const petri_net::place_id_type&)> translate
    ) const
    {
      expr::eval::context context;

      typename Traits<MAP>::map_it_t mpos (map.begin());
      const typename Traits<MAP>::map_it_t mend (map.end());
      pos_t::const_iterator state (pos.begin());

      while (mpos != mend)
      {
        context.bind ( translate (mpos->first)
                     , mpos->second[*state % mpos->second.size()].value
                     );

        ++mpos;
        ++state;
      }

      return parser.eval_all_bool (context);
    }

    void write_to (std::vector<typename Traits<MAP>::ret_t>& v) const
    {
      v.clear();

      typename Traits<MAP>::map_it_t mpos (map.begin());
      const typename Traits<MAP>::map_it_t mend (map.end());
      pos_t::const_iterator state (pos.begin());

      while (mpos != mend)
      {
        v.push_back ( typename Traits<MAP>::ret_t
                      ( mpos->first
                      , mpos->second[*state % mpos->second.size()]
                      )
                    );

        ++mpos;
        ++state;
      }
    }
  };
}

#endif
