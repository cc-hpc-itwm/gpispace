// mirko.rahn@itwm.fraunhofer.de

#include <we/util/cross.hpp>

#include <we/type/transition.hpp>

//! \todo eliminate this
#include <we/type/net.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/utility.hpp>

namespace we
{
  namespace util
  {
    iterators_type::iterators_type (std::list<pnet::type::value::value_type>& tokens)
      : _begin (tokens.begin())
      , _end (tokens.end())
      , _pos_and_distance (tokens.begin(), 0)
    {}
    iterators_type::iterators_type (const std::list<pnet::type::value::value_type>::iterator& token)
      : _begin (token)
      , _end (boost::next (token))
      , _pos_and_distance (token, 0)
    {}
    bool iterators_type::end() const
    {
      return _end == _pos_and_distance.first;
    }
    const pos_and_distance_type& iterators_type::pos_and_distance() const
    {
      return _pos_and_distance;
    }
    void iterators_type::operator++()
    {
      ++_pos_and_distance.first;
      ++_pos_and_distance.second;
    }
    void iterators_type::rewind()
    {
      _pos_and_distance.first = _begin;
      _pos_and_distance.second = 0;
    }

    namespace
    {
      typedef boost::unordered_map< petri_net::place_id_type
                                  , iterators_type
                                  > map_type;

      bool do_step (map_type::iterator slot, const map_type::iterator& end)
      {
        ++slot->second;

        if (slot->second.end())
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
        context.bind_ref ( transition.name_of_place (pits.first)
                         , *pits.second.pos_and_distance().first
                         );
      }

      return transition.condition().parser().eval_all_bool (context);
    }
    void cross_type::write_to (boost::unordered_map< petri_net::place_id_type
                                                   , pos_and_distance_type
                                                   >& choice
                              ) const
    {
      choice.clear();

      typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

      BOOST_FOREACH (const pits_type& pits, _m)
      {
        choice.insert
          (std::make_pair (pits.first, pits.second.pos_and_distance()));
      }
    }
    void cross_type::push ( const petri_net::place_id_type& place_id
                          , std::list<pnet::type::value::value_type>& tokens
                          )
    {
      _m.insert (std::make_pair (place_id, iterators_type (tokens)));
    }
    void cross_type::push ( const petri_net::place_id_type& place_id
                          , const std::list<pnet::type::value::value_type>::iterator& token
                          )
    {
      _m.insert (std::make_pair (place_id, iterators_type (token)));
    }
  }
}
