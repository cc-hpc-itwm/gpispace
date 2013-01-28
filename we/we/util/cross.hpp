// mirko.rahn@itwm.fraunhofer.de

#include <we/type/token.hpp>
#include <we/type/id.hpp>

#include <we/type/transition.fwd.hpp>

#include <vector>

#include <boost/unordered_map.hpp>

namespace we
{
  namespace util
  {
    class iterators_type
    {
    public:
      iterators_type (const std::vector<token::type>&);
      const std::vector<token::type>::const_iterator& end() const;
      const std::vector<token::type>::const_iterator& pos() const;
      void operator++();
      void rewind();
    private:
      std::vector<token::type>::const_iterator _begin;
      std::vector<token::type>::const_iterator _end;
      std::vector<token::type>::const_iterator _pos;
    };

    typedef boost::unordered_map< petri_net::place_id_type
                                , iterators_type
                                > map_type;

    class cross_type
    {
    public:
      bool empty() const;
      bool step();
      bool eval (const we::type::transition_t&) const;
      void write_to (std::vector<std::pair< petri_net::place_id_type
                                          ,token::type
                                          >
                                >&
                    ) const;
      void push ( const petri_net::place_id_type&
                , const std::vector<token::type>&
                );
    private:
      map_type _m;
    };
  }
}
