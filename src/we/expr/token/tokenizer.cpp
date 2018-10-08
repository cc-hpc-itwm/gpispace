// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/token/tokenizer.hpp>

#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <boost/variant.hpp>

#include <we/type/value/read.hpp>

#include <fhg/util/parse/require.hpp>

#include <functional>
#include <iterator>

namespace expr
{
  namespace token
  {
    tokenizer::tokenizer (fhg::util::parse::position& _p)
      : _pos (_p)
      , _token (eof)
      , _tokval()
      , _ref()
    {}

    const pnet::type::value::value_type& tokenizer::value() const
    {
      return _tokval;
    }
    const token::type& tokenizer::token() const
    {
      return _token;
    }
    const std::list<std::string>& tokenizer::get_ref() const
    {
      return _ref;
    }
    fhg::util::parse::position& tokenizer::pos()
    {
      return _pos;
    }

    namespace
    {
      typedef boost::make_recursive_variant
              < std::map<char, boost::recursive_variant_>
              , std::function<void (tokenizer&)>
              >::type node_type;

      typedef std::map<char, node_type> child_type;

      void put ( const std::string::const_iterator pos
               , const std::string::const_iterator end
               , const std::function<void (tokenizer&)> f
               , child_type& m
               )
      {
        const child_type::iterator child (m.find (*pos));

        if (std::next (pos) == end)
        {
          if (child != m.end())
          {
            throw std::runtime_error
              (std::string ("Upps, doubled action(-path)!? "));
          }

          m.insert (child, std::make_pair (*pos, f));
        }
        else
        {
          put ( std::next (pos)
              , end
              , f
              , boost::get<child_type>
                ( child == m.end()
                ? m.insert (child, std::make_pair (*pos, child_type()))->second
                : child->second
                )
              );
        }
      }

      void put ( const std::string& key
               , const std::function<void (tokenizer&)> f
               , child_type& m
               )
      {
        if (key.empty())
        {
          throw std::runtime_error ("Upps, action with empty key!?");
        }

        put (key.begin(), key.end(), f, m);
      }

      const child_type& create_description()
      {
        static child_type ts;

#define ACTION(_key,_action) put (_key, _action, ts)

#define UNARY(_key, _token)                                             \
        ACTION (_key, std::bind (&tokenizer::unary, std::placeholders::_1, _token, _key))

#define SET_TOKEN(_key, _token)                                         \
        ACTION (_key, std::bind (&tokenizer::set_token, std::placeholders::_1, _token))

#define SET_VALUE(_key, _value)                                         \
        ACTION (_key, std::bind (&tokenizer::set_value, std::placeholders::_1, _value))

        ACTION ("!", std::bind (&tokenizer::notne, std::placeholders::_1));
        ACTION ("${", std::bind (&tokenizer::identifier, std::placeholders::_1));
        ACTION ("*", std::bind (&tokenizer::mulpow, std::placeholders::_1));
        ACTION ("|", std::bind (&tokenizer::or_boolean_integral, std::placeholders::_1));
        ACTION ("&", std::bind (&tokenizer::and_boolean_integral, std::placeholders::_1));
        ACTION ("-", std::bind (&tokenizer::negsub, std::placeholders::_1));
        ACTION ("/", std::bind (&tokenizer::divcomment, std::placeholders::_1));
        ACTION ("<", std::bind (&tokenizer::cmp, std::placeholders::_1, lt, le));
        ACTION (">", std::bind (&tokenizer::cmp, std::placeholders::_1, gt, ge));

        SET_TOKEN ("%", modint);
        SET_TOKEN ("(", lpr);
        SET_TOKEN (")", rpr);
        SET_TOKEN ("+", add);
        SET_TOKEN (",", sep);
        SET_TOKEN (":=", define);
        SET_TOKEN (":and:", _and_boolean);
        SET_TOKEN (":eq:", eq);
        SET_TOKEN (":ge:", ge);
        SET_TOKEN (":gt:", gt);
        SET_TOKEN (":le:", le);
        SET_TOKEN (":lt:", lt);
        SET_TOKEN (":ne:", ne);
        SET_TOKEN (":or:", _or_boolean);
        SET_TOKEN ("==", eq);
        SET_TOKEN ("^", _powint);
        SET_TOKEN ("div", divint);
        SET_TOKEN ("int", _toint);
        SET_TOKEN ("long", _tolong);
        SET_TOKEN ("uint", _touint);
        SET_TOKEN ("ulong", _toulong);
        SET_TOKEN ("float", _tofloat);
        SET_TOKEN ("double", _todouble);
        SET_TOKEN ("max", max);
        SET_TOKEN ("min", min);
        SET_TOKEN ("mod", modint);
        SET_TOKEN ("round", _round);

        SET_TOKEN ("map_assign", _map_assign);
        SET_TOKEN ("map_get_assignment", _map_get_assignment);
        SET_TOKEN ("map_is_assigned", _map_is_assigned);
        SET_TOKEN ("map_unassign", _map_unassign);
        UNARY ("map_empty", _map_empty);
        UNARY ("map_size", _map_size);

        SET_VALUE ("e", 2.7182818284590452354);
        SET_VALUE ("pi", 3.14159265358979323846);

        SET_VALUE ("false", false);
        SET_VALUE ("true", true);

        UNARY ("abs", abs);
        UNARY ("ceil", _ceil);
        UNARY ("cos", _cos);
        UNARY ("floor", _floor);
        UNARY ("log", _log);
        UNARY ("sin", _sin);
        UNARY ("sqrt", _sqrt);

        SET_TOKEN ("bitset_and", _bitset_and);
        SET_TOKEN ("bitset_count", _bitset_count);
        SET_TOKEN ("bitset_delete", _bitset_delete);
        SET_TOKEN ("bitset_insert", _bitset_insert);
        SET_TOKEN ("bitset_is_element", _bitset_is_element);
        SET_TOKEN ("bitset_or", _bitset_or);
        SET_TOKEN ("bitset_xor", _bitset_xor);
        UNARY ("bitset_fromhex", _bitset_fromhex);
        UNARY ("bitset_tohex", _bitset_tohex);

        SET_TOKEN ("set_insert", _set_insert);
        SET_TOKEN ("set_erase", _set_erase);
        SET_TOKEN ("set_is_element", _set_is_element);
        SET_TOKEN ("set_is_subset", _set_is_subset);
        UNARY ("set_empty", _set_empty);
        UNARY ("set_pop", _set_pop);
        UNARY ("set_size", _set_size);
        UNARY ("set_top", _set_top);

        SET_TOKEN ("stack_join", _stack_join);
        SET_TOKEN ("stack_push", _stack_push);
        UNARY ("stack_empty", _stack_empty);
        UNARY ("stack_pop", _stack_pop);
        UNARY ("stack_size", _stack_size);
        UNARY ("stack_top", _stack_top);

#undef SET_VALUE
#undef SET_TOKEN
#undef UNARY
#undef ACTION

        return ts;
      }

      const node_type& description()
      {
        static node_type m (create_description());

        return m;
      }

      class visitor_names : public boost::static_visitor<void>
      {
      public:
        visitor_names (std::string& names, const std::string& prefix = "")
          : _names (names)
          , _prefix (prefix)
        {}

        void operator() (const child_type& m) const
        {
          for (auto const& cn : m)
          {
            boost::apply_visitor
              ( visitor_names (_names, _prefix + cn.first)
              , cn.second
              );
          }
        }
        void operator() (const std::function<void (tokenizer&)>&) const
        {
          if (!_names.empty())
          {
            _names += ", ";
          }
          _names += _prefix;
        }

      private:
        std::string& _names;
        const std::string _prefix;
      };

      std::string names (const node_type& node)
      {
        std::string names;

        boost::apply_visitor (visitor_names (names), node);

        return names;
      }

      class visitor_tokenize : public boost::static_visitor<void>
      {
      public:
        visitor_tokenize (tokenizer& t, const bool& first = true)
          : _tokenizer (t)
          , _first (first)
        {}
        void operator() (const child_type& m) const
        {
          if (_tokenizer.is_eof())
          {
            throw exception::parse::expected (names (m), _tokenizer.pos().eaten());
          }
          else
          {
            for (auto const& cn : m)
              {
                if (*_tokenizer.pos() == cn.first)
                  {
                    ++_tokenizer.pos();

                    return boost::apply_visitor
                      (visitor_tokenize (_tokenizer, false), cn.second);
                  }
              }
          }

          if (_first)
          {
            _tokenizer.set_value (pnet::type::value::read (_tokenizer.pos()));
          }
          else
          {
            throw exception::parse::expected (names (m), _tokenizer.pos().eaten());
          }
        }

        void operator() (const std::function<void (tokenizer&)>& f) const
        {
          f (_tokenizer);
        }

      private:
        tokenizer& _tokenizer;
        const bool _first;
      };
    }

    void tokenizer::set_token (const token::type& t)
    {
      _token = t;
    }
    void tokenizer::set_value (const pnet::type::value::value_type& v)
    {
      set_token (val);
      _tokval = v;
    }

    bool tokenizer::is_eof()
    {
      return _pos.end() || *_pos == ';';
    }

    void tokenizer::cmp (const token::type& t, const token::type& e)
    {
      if (!is_eof() && *_pos == '=')
      {
        ++_pos;

        set_token (e);
      }
      else
      {
        set_token (t);
      }
    }

    void tokenizer::mulpow()
    {
      if (!is_eof() && *_pos == '*')
      {
        ++_pos;

        set_token (_pow);
      }
      else
      {
        set_token (mul);
      }
    }

    void tokenizer::or_boolean_integral()
    {
      if (!is_eof() && *_pos == '|')
      {
        ++_pos;

        set_token (_or_boolean);
      }
      else
      {
        set_token (_or_integral);
      }
    }

    void tokenizer::and_boolean_integral()
    {
      if (!is_eof() && *_pos == '&')
      {
        ++_pos;

        set_token (_and_boolean);
      }
      else
      {
        set_token (_and_integral);
      }
    }

    void tokenizer::negsub()
    {
      set_token (next_can_be_unary (_token) ? neg : sub);
    }

    void tokenizer::divcomment()
    {
      if (!is_eof() && *_pos == '*')
      {
        ++_pos;

        skip_comment (_pos.eaten());

        operator++();
      }
      else
      {
        set_token (div);
      }
    }

    void tokenizer::identifier()
    {
      set_token (ref);

      _ref.clear();

      do
        {
          _ref.push_back (fhg::util::parse::require::identifier (_pos));

          if (_pos.end())
          {
            throw exception::parse::expected ("'.' or '}'", _pos.eaten());
          }

          if (*_pos == '.')
          {
            ++_pos;
          }
        }
      while (!_pos.end() && *_pos != '}');

      fhg::util::parse::require::require (_pos, '}');
    }

    void tokenizer::notne()
    {
      if (!is_eof() && *_pos == '=')
      {
        ++_pos;

        set_token (ne);
      }
      else
      {
        unary (_not, "negation");
      }
    }

    void tokenizer::unary (const token::type& t, const std::string& descr)
    {
      if (next_can_be_unary (_token))
      {
        set_token (t);
      }
      else
      {
        throw exception::parse::misplaced (descr, _pos.eaten());
      }
    }

    void tokenizer::skip_comment (const std::size_t open)
    {
      while (!_pos.end())
        switch (*_pos)
          {
          case '/':
            ++_pos;
            if (!_pos.end() && *_pos == '*')
              {
                ++_pos; skip_comment (_pos.eaten());
              }
            break;
          case '*':
            ++_pos;
            if (!_pos.end() && *_pos == '/')
              {
                ++_pos; return;
              }
            break;
          default: ++_pos; break;
          }

      throw exception::parse::unterminated ("comment", open-2, _pos.eaten());
    }

    void tokenizer::operator++()
    {
      fhg::util::parse::require::skip_spaces (_pos);

      if (is_eof())
      {
        if (!_pos.end())
        {
          ++_pos;
        }

        set_token (eof);
      }
      else
      {
        boost::apply_visitor (visitor_tokenize (*this), description());
      }
    }
  }
}
