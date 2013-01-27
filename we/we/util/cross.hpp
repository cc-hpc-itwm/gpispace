// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/id.hpp>
#include <we/type/condition.hpp>
#include <we/type/transition.hpp>

#include <vector>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

namespace cross
{
  class iterators_type
  {
  public:
    explicit iterators_type (const std::vector<token::type>& tokens)
      : _begin (tokens.begin())
      , _end (tokens.end())
      , _pos (tokens.begin())
    {}
    const std::vector<token::type>::const_iterator& end() const
    {
      return _end;
    }
    const std::vector<token::type>::const_iterator& pos() const
    {
      return _pos;
    }
    void operator++()
    {
      ++_pos;
    }
    void rewind()
    {
      _pos = _begin;
    }
    bool empty() const
    {
      return _begin == _end;
    }

  private:
    std::vector<token::type>::const_iterator _begin;
    std::vector<token::type>::const_iterator _end;
    std::vector<token::type>::const_iterator _pos;
  };

  typedef boost::unordered_map< petri_net::place_id_type
                              , iterators_type
                              > map_type;

  namespace pred
  {
    bool second_empty (const map_type::const_iterator::value_type& pits)
    {
      return pits.second.empty();
    }
  }

  class cross
  {
  private:
    map_type& _map;
    bool _has_more;

    void step (map_type::iterator slot)
    {
      ++slot->second;

      if (slot->second.pos() == slot->second.end())
        {
          slot->second.rewind();

          ++slot;

          if (slot == _map.end())
            {
              _has_more= false;
            }
          else
            {
              step (slot);
            }
        }
    }

    bool determine_has_more() const
    {
      return
        (not _map.empty())
        &&
        (  std::find_if (_map.begin(), _map.end(), pred::second_empty)
        == _map.end()
        )
        ;
    }

  public:
    explicit cross (map_type& map)
      : _map (map)
      , _has_more (determine_has_more())
    {}

    bool has_more() const { return _has_more; }
    void operator++() { step (_map.begin()); }

    bool eval (const we::type::transition_t& transition) const
    {
      if (transition.condition().expression() == "true")
      {
        return true;
      }

      expr::eval::context context;

      BOOST_FOREACH (const map_type::const_iterator::value_type& pits, _map)
      {
        context.bind ( transition.name_of_place (pits.first)
                     , pits.second.pos()->value
                     );
      }

      return transition.condition().parser().eval_all_bool (context);
    }

    void write_to ( std::vector<std::pair< petri_net::place_id_type
                                         , token::type
                                         >
                               >& v
                  ) const
    {
      v.clear();

      BOOST_FOREACH (const map_type::const_iterator::value_type& pits, _map)
      {
        v.push_back (std::make_pair (pits.first, *pits.second.pos()));
      }
    }
  };
}

#endif
