// transition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_TRANS_HPP
#define _FUNCTION_TRANS_HPP

#include <we/type/id.hpp>

#include <vector>

#include <boost/unordered_map.hpp>

#include <boost/function.hpp>

namespace Function { namespace Transition
{
  typedef petri_net::pid_t pid_t;
  typedef petri_net::eid_t eid_t;

  template<typename Token>
  struct Traits
  {
  public:
    // a transition gets a number of input tokens, taken from places,
    // connected to the transition via edges
    // so input is of type: [(Token,(Place,Edge))]
    // the same holds true for the output, but the tokens are to be produced
    typedef std::pair<pid_t, eid_t> place_via_edge_t;
    typedef std::pair<Token, place_via_edge_t> token_input_t;
    typedef std::vector<token_input_t> input_t;

    typedef boost::unordered_map<pid_t,eid_t> output_descr_t;
    typedef std::pair<Token, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> output_t;

    typedef boost::function<output_t ( const input_t &
                                     , const output_descr_t &
                                     )
                           > fun_t;
  };

  template<typename Token>
  pid_t get_pid
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.first;
  }

  template<typename Token>
  pid_t get_pid
  (const typename Traits<Token>::output_descr_t::value_type & place_via_edge)
  {
    return place_via_edge.first;
  }

  template<typename Token>
  eid_t get_eid
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.second;
  }

  template<typename Token>
  eid_t get_eid
  (const typename Traits<Token>::output_descr_t::value_type & place_via_edge)
  {
    return place_via_edge.second;
  }

  template<typename Token>
  pid_t get_pid
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_pid<Token> (token_input.second);
  }

  template<typename Token>
  eid_t get_eid
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_eid<Token> (token_input.second);
  }

  template<typename Token>
  const Token & get_token
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return token_input.first;
  }

  template<typename Token>
  pid_t get_pid
  (const typename Traits<Token>::token_on_place_t & token_on_place)
  {
    return token_on_place.second;
  }

  template<typename Token>
  const Token & get_token
  (const typename Traits<Token>::token_on_place_t & token_on_place)
  {
    return token_on_place.first;
  }

  // eat some function
  template<typename Token>
  class Generic
  {
  private:
    typedef typename Traits<Token>::input_t input_t;
    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename Traits<Token>::output_t output_t;
    typedef typename Traits<Token>::token_on_place_t token_on_place_t;

  public:
    typedef boost::function<output_t ( const input_t &
                                     , const output_descr_t &
                                     )
                           > Function;

  private:
    const Function f;

  public:
    explicit Generic (const Function & _f) : f (_f) {}

    output_t operator () ( const input_t & input
                         , const output_descr_t & output_descr
                         ) const
    {
      return f (input, output_descr);
    }
  };

  // generic match the input/output description, use all information available
  template<typename Token, typename Descr>
  class MatchWithFun
  {
  private:
    typedef typename Traits<Token>::input_t input_t;
    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename Traits<Token>::output_t output_t;
    typedef typename Traits<Token>::token_on_place_t token_on_place_t;
    typedef typename Traits<Token>::token_input_t token_input_t;
    typedef typename Traits<Token>::place_via_edge_t place_via_edge_t;

  public:
    typedef boost::function<Descr (const token_input_t &)> DescrIn;
    typedef boost::function<Descr (const place_via_edge_t &)> DescrOut;

    typedef boost::function<Token ( const Descr &
                                  , const token_input_t &
                                  , const place_via_edge_t &
                                  )
                           > Apply;

  private:
    const DescrIn descr_in;
    const DescrOut descr_out;
    const Apply apply;

  public:
    MatchWithFun ( const DescrIn & _descr_in
                 , const DescrOut & _descr_out
                 , const Apply & _apply
                 )
      : descr_in (_descr_in)
      , descr_out (_descr_out)
      , apply (_apply)
    {}

    output_t operator () ( const input_t & input
                         , const output_descr_t & output_descr
                         ) const
    {
      // collect map descr -> token_input_t
      typedef typename boost::unordered_map<Descr,token_input_t> map_t;
      map_t m;

      for ( typename input_t::const_iterator it (input.begin())
          ; it != input.end()
          ; ++it
          )
        m[descr_in(*it)] = *it;

      // match into the output according to the output description
      output_t output;

      for ( typename output_descr_t::const_iterator it (output_descr.begin())
          ; it != output_descr.end()
          ; ++it
          )
        {
          const Descr descr (descr_out (*it));

          const typename map_t::iterator m_it (m.find (descr));

          if (m_it == m.end())
            throw std::runtime_error ("MatchWithFun: missing input edge");

          output.push_back
            (token_on_place_t ( apply (descr, m_it->second, *it)
                              , get_pid<Token> (*it)
                              )
            );

          m.erase (m_it);
        }

      if (!m.empty())
        throw std::runtime_error ("MatchWithFun: missing output edge");

      return output;
    }
  };

  template<typename Token, typename Descr>
  Token const & apply_const ( const Descr &
                    , const typename Traits<Token>::token_input_t & token_input
                    , const typename Traits<Token>::place_via_edge_t &
                    )
  {
    return get_token<Token> (token_input);
  }

  template<typename Token, typename Descr>
  Descr descr_in_by_eid
  ( boost::function<Descr (const eid_t &)> f
  , const typename Traits<Token>::token_input_t & token_input
  )
  {
    return f (get_eid<Token> (token_input));
  }

  template<typename Token, typename Descr>
  Descr descr_out_by_eid
  ( boost::function<Descr (const eid_t &)> f
  , const typename Traits<Token>::place_via_edge_t & place_via_edge
  )
  {
    return f (get_eid<Token> (place_via_edge));
  }

  // match edge descriptions
  template<typename Token, typename Descr>
  class MatchEdge : public MatchWithFun<Token, Descr>
  {
  public:
    typedef boost::function<Descr (const eid_t &)> Function;

    explicit MatchEdge (const Function & f)
      : MatchWithFun<Token,Descr>
        ( boost::bind (&descr_in_by_eid<Token,Descr>, f, _1)
        , boost::bind (&descr_out_by_eid<Token,Descr>, f, _1)
        , apply_const<Token,Descr>
        )
    {}
  };

  // default construct all output tokens, always possible
  template<typename Token>
  class Default
  {
  private:
    typedef typename Traits<Token>::input_t input_t;
    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename Traits<Token>::output_t output_t;
    typedef typename Traits<Token>::token_on_place_t token_on_place_t;

  public:
    output_t operator () ( const input_t &
                         , const output_descr_t & output_descr
                         ) const
    {
      output_t output;

      for ( typename output_descr_t::const_iterator it (output_descr.begin())
          ; it != output_descr.end()
          ; ++it
          )
        output.push_back (token_on_place_t (Token(), get_pid<Token>(*it)));

      return output;
    }
  };

  // needs the same number of input and output tokens
  // applies a function without context to each token
  // stays with the order given in input/output_descr
  template<typename Token>
  class PassWithFun
  {
  private:
    typedef typename Traits<Token>::input_t input_t;
    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename Traits<Token>::output_t output_t;
    typedef typename Traits<Token>::token_on_place_t token_on_place_t;

  public:
    typedef boost::function<Token (const Token &)> Function;

  private:
    const Function f;

  public:
    explicit PassWithFun (const Function & _f) : f (_f) {}

    output_t operator () ( const input_t & input
                         , const output_descr_t & output_descr
                         ) const
    {
      output_t output;

      typename output_descr_t::const_iterator it_out (output_descr.begin());
      typename input_t::const_iterator it_in (input.begin());

      for ( ; it_out != output_descr.end(); ++it_out, ++it_in)
        {
          if (it_in == input.end())
            throw std::runtime_error ("pass through: missing input token");

          output.push_back (token_on_place_t ( f(get_token<Token>(*it_in))
                                             , get_pid<Token>(*it_out)
                                             )
                           );
        }

      if (it_in != input.end())
        throw std::runtime_error ("pass through: missing output places");

      return output;
    }
  };

  template<typename Token>
  inline Token token_const (const Token & token) { return token; }

  // simple pass the tokens through
  template<typename Token>
  class Pass : public PassWithFun<Token>
  {
  public:
    Pass () : PassWithFun<Token>(token_const<Token>) {}
  };

  // apply a function, that depends on the edges only
  template<typename Token>
  class EdgesOnly
  {
  public:
    typedef typename boost::unordered_map<eid_t, Token> map_t;
    typedef boost::function<map_t (const map_t &)> Function;

  private:
    typedef typename Traits<Token>::input_t input_t;
    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename Traits<Token>::output_t output_t;
    typedef typename Traits<Token>::token_on_place_t token_on_place_t;

    const Function f;

  public:
    explicit EdgesOnly (const Function & _f) : f (_f) {}

    output_t operator () ( const input_t & input
                         , const output_descr_t & output_descr
                         ) const
    {
      // collect input as Map (Edge -> Token)
      map_t in;

      for ( typename input_t::const_iterator it (input.begin())
          ; it != input.end()
          ; ++it
          )
        in[get_eid<Token>(*it)] = get_token<Token>(*it);

      // calculate output as Map (Edge -> Token)
      map_t out (f (in));

      // fill the output vector
      output_t output;

      for ( typename output_descr_t::const_iterator it (output_descr.begin())
          ; it != output_descr.end()
          ; ++it
          )
        {
          const typename map_t::iterator res (out.find (get_eid<Token>(*it)));

          if (res == out.end())
            throw std::runtime_error ("edge only: missing edge in output map");

          output.push_back ( token_on_place_t ( res->second
                                              , get_pid<Token>(*it)
                                              )
                           );

          out.erase (res);
        }

      if (!out.empty())
        throw std::runtime_error ("edge only: to much edges in output map");

      return output;
    }
  };
}}

#endif
