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

  inline bool step (map_type::iterator slot, const map_type::iterator& end)
  {
    ++slot->second;

    if (slot->second.pos() == slot->second.end())
    {
      slot->second.rewind();

      ++slot;

      if (slot == end)
      {
        return false;
      }
      else
      {
        return step (slot, end);
      }
    }

    return true;
  }

  inline bool step (map_type& m)
  {
    return step (m.begin(), m.end());
  }

  inline bool eval (const map_type& m, const we::type::transition_t& transition)
  {
    if (transition.condition().expression() == "true")
    {
      return true;
    }

    expr::eval::context context;

    BOOST_FOREACH (const map_type::const_iterator::value_type& pits, m)
    {
      context.bind ( transition.name_of_place (pits.first)
                   , pits.second.pos()->value
                   );
    }

    return transition.condition().parser().eval_all_bool (context);
  }
}

#endif
