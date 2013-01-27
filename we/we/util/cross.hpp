// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_CROSS_HPP
#define _UTIL_CROSS_HPP

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/id.hpp>
#include <we/type/condition.hpp>

#include <vector>

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

namespace cross
{
  typedef boost::unordered_map< petri_net::place_id_type
                              , std::vector<token::type>
                              > token_by_place_id_t;

  namespace pred
  {
    bool second_empty (const token_by_place_id_t::const_iterator::value_type& v)
    {
      return v.second.empty();
    }
  }

  class cross
  {
  private:
    const token_by_place_id_t& _map;
    std::vector<std::size_t> _pos;
    bool _has_more;

    void step (std::size_t slot, token_by_place_id_t::const_iterator it)
    {
      ++_pos[slot];

      if (_pos[slot] == it->second.size())
        {
          _pos[slot] = 0;

          ++slot;
          ++it;

          if (it == _map.end())
            {
              _has_more = false;
            }
          else
            {
              step (slot, it);
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
    explicit cross (const token_by_place_id_t& map)
      : _map (map)
      , _pos (_map.size(), 0)
      , _has_more (determine_has_more())
    {}

    bool has_more() const { return _has_more; }
    void operator++() { step (0, _map.begin()); }

    bool eval
    ( const condition::type& condition
    , boost::function<std::string (const petri_net::place_id_type&)> translate
    ) const
    {
      if (condition.expression() == "true")
      {
        return true;
      }

      expr::eval::context context;

      token_by_place_id_t::const_iterator mpos (_map.begin());
      const token_by_place_id_t::const_iterator mend (_map.end());
      std::vector<std::size_t>::const_iterator state (_pos.begin());

      while (mpos != mend)
      {
        context.bind ( translate (mpos->first)
                     , mpos->second[*state % mpos->second.size()].value
                     );

        ++mpos;
        ++state;
      }

      return condition.parser().eval_all_bool (context);
    }

    void write_to ( std::vector<std::pair< petri_net::place_id_type
                                         , token::type
                                         >
                               >& v
                  ) const
    {
      v.clear();

      token_by_place_id_t::const_iterator mpos (_map.begin());
      const token_by_place_id_t::const_iterator mend (_map.end());
      std::vector<std::size_t>::const_iterator state (_pos.begin());

      while (mpos != mend)
      {
        v.push_back ( std::make_pair
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
