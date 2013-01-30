// mirko.rahn@itwm.fraunhofer.de

#include <we/util/cross.hpp>

#include <we/type/transition.hpp>

//! \todo eliminate this
#include <we/type/net.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

namespace we
{
  namespace util
  {
    iterators_type::iterators_type (const std::vector<token::type>& tokens)
      : _begin (tokens.begin())
      , _end (tokens.end())
      , _pos (tokens.begin())
    {}
    const std::vector<token::type>::const_iterator& iterators_type::end() const
    {
      return _end;
    }
    const std::vector<token::type>::const_iterator& iterators_type::pos() const
    {
      return _pos;
    }
    void iterators_type::operator++()
    {
      ++_pos;
    }
    void iterators_type::rewind()
    {
      _pos = _begin;
    }

    namespace
    {
      typedef boost::unordered_map< petri_net::place_id_type
                                  , iterators_type
                                  > map_type;

      bool do_step (map_type::iterator slot, const map_type::iterator& end)
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
            return do_step (slot, end);
          }
        }

        return true;
      }
    }

    bool cross_type::empty() const
    {
      return _m.empty();
    }
    bool cross_type::step()
    {
      return do_step (_m.begin(), _m.end());
    }
    bool cross_type::eval (const we::type::transition_t& transition) const
    {
      //! \todo use is_const_true and boost::optional...
      if (transition.condition().expression() == "true")
      {
        return true;
      }

      expr::eval::context context;

      typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

      BOOST_FOREACH (const pits_type& pits, _m)
      {
        context.bind ( transition.name_of_place (pits.first)
                     , pits.second.pos()->value
                     );
      }

      return transition.condition().parser().eval_all_bool (context);
    }
    void cross_type::write_to
      (boost::unordered_map<petri_net::place_id_type,token::type>& choice) const
    {
      choice.clear();

      typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

      BOOST_FOREACH (const pits_type& pits, _m)
      {
        choice.insert (std::make_pair (pits.first, *pits.second.pos()));
      }
    }
    void cross_type::push ( const petri_net::place_id_type& place_id
                          , const std::vector<token::type>& tokens
                          )
    {
      _m.insert (std::make_pair (place_id, iterators_type (tokens)));
    }
  }
}
