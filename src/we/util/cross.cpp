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
    cross_type::iterators_type::iterators_type (std::list<pnet::type::value::value_type>& tokens)
      : _begin (tokens.begin())
      , _end (tokens.end())
      , _pos_and_distance (tokens.begin(), 0)
    {}
    cross_type::iterators_type::iterators_type (const std::list<pnet::type::value::value_type>::iterator& token)
      : _begin (token)
      , _end (boost::next (token))
      , _pos_and_distance (token, 0)
    {}
    bool cross_type::iterators_type::end() const
    {
      return _end == _pos_and_distance.first;
    }
    const pos_and_distance_type& cross_type::iterators_type::pos_and_distance() const
    {
      return _pos_and_distance;
    }
    void cross_type::iterators_type::operator++()
    {
      ++_pos_and_distance.first;
      ++_pos_and_distance.second;
    }
    void cross_type::iterators_type::rewind()
    {
      _pos_and_distance.first = _begin;
      _pos_and_distance.second = 0;
    }

    bool cross_type::do_step
      (map_type::iterator slot, map_type::iterator const& end)
    {
      while (slot != end)
      {
        //! \note all sequences are non-empty
        ++slot->second;

        if (!slot->second.end())
        {
          return true;
        }

        slot->second.rewind();
        ++slot;
      }

      return false;
    }

    bool cross_type::enables
    ( boost::function<std::string const& (petri_net::place_id_type const&)> name
    , condition::type const& condition
    )
    {
      //! \note that means the transitions without in-port cannot fire
      //! instead of fire unconditionally
      if (_m.empty())
      {
        return false;
      }

      //! \todo use is_const_true and boost::optional...
      if (condition.expression() == "true")
      {
        return true;
      }

      do
      {
        expr::eval::context context;

        typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

        BOOST_FOREACH (const pits_type& pits, _m)
        {
          context.bind_ref
            (name (pits.first), *pits.second.pos_and_distance().first);
        }

        if (condition.parser().eval_all_bool (context))
        {
          return true;
        }
      }
      while (do_step (_m.begin(), _m.end()));

      return false;
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
