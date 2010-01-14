// transition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _TRANSFUN_HPP
#define _TRANSFUN_HPP

#include <netfwd.hpp>

#include <map>
#include <vector>

#include <boost/function.hpp>

namespace TransitionFunction
{
  template<typename Token>
  struct Traits
  {
  public:
    // a transition gets a number of input tokens, taken from places,
    // connected to the transition via edges
    // so input is of type: [(Token,(Place,Edge))]
    // the same holds true for the output, but the tokens are to be produced
    typedef std::pair<petri_net::pid_t, petri_net::eid_t> place_via_edge_t;
    typedef std::pair<Token, place_via_edge_t> token_input_t;
    typedef std::vector<token_input_t> input_t;

    typedef std::vector<place_via_edge_t> output_descr_t;
    typedef std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> output_t;

    typedef std::map<petri_net::eid_t, Token> edges_only_t;
  };

  template<typename Token>
  petri_net::pid_t get_pid
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.first;
  }

  template<typename Token>
  petri_net::eid_t get_eid 
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.second;
  }

  template<typename Token>
  petri_net::pid_t get_pid 
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_pid<Token> (token_input.second);
  }

  template<typename Token>
  petri_net::eid_t get_eid 
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_eid<Token> (token_input.second);
  }

  template<typename Token>
  Token get_token
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return token_input.first;
  }

  // default construct all output tokens, always possible
  template<typename Token>
  class Default
  {
  private:
    typedef typename Traits<Token>::input_t input_t;

    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef typename Traits<Token>::output_t output_t;

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
        output.push_back (token_on_place_t (Token(), get_pid <Token>(*it)));

      return output;
    }
  };

  // needs the same number of input and output tokens
  // applies a function without context to each token
  // stays with the order given in input/output_descr
  template<typename Token, Token F (const Token &)>
  class PassThroughWithFun
  {
  private:
    typedef typename Traits<Token>::input_t input_t;

    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef typename Traits<Token>::output_t output_t;

  public:
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
            throw std::runtime_error ("not enough input tokens to pass through");

          output.push_back (token_on_place_t ( F(get_token<Token>(*it_in))
                                             , get_pid<Token>(*it_out)
                                             )
                           );
        }

      if (it_in != input.end())
        throw std::runtime_error ("not enough output places to pass through");

      return output;
    }
  };

  template<typename Token>
  inline Token Const (const Token & token)
  {
    return token;
  }

  // simple pass the tokens through
  template<typename Token>
  class PassThrough : public PassThroughWithFun<Token, Const<Token> >
  {};

  // apply a function, that depends on the edges only
  template<typename Token>
  class EdgesOnly
  {
  private:
    typedef typename Traits<Token>::input_t input_t;

    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef typename Traits<Token>::output_t output_t;

    typedef typename Traits<Token>::edges_only_t edges_only_t;
    
    typedef typename Traits<Token>::edges_only_t map_t;

    typedef boost::function<map_t (const map_t * const)> F;

    F f;

  public:
    EdgesOnly (F _f) : f (_f) {}

    output_t operator () ( const input_t & input
                         , const output_descr_t & output_descr
                         ) const
    {
      // collect input as Map (Edge -> Token)
      edges_only_t in;

      for ( typename input_t::const_iterator it (input.begin())
          ; it != input.end()
          ; ++it
          )
        in[get_eid<Token>(*it)] = get_token<Token>(*it);

      // calculate output as Map (Edge -> Token)
      edges_only_t out (f (&in));

      // fill the output vector
      output_t output;

      for ( typename output_descr_t::const_iterator it (output_descr.begin())
          ; it != output_descr.end()
          ; ++it
          )
        {
          typename edges_only_t::iterator res (out.find (get_eid<Token>(*it)));

          if (res == out.end())
            throw std::runtime_error ("missing edge in output map");

          output.push_back (token_on_place_t (res->second, get_pid<Token>(*it)));

          out.erase (res);
        }

      if (!out.empty())
        throw std::runtime_error ("to much edges in output map");

      return output;
    }
  };
}

#endif // _TRANSFUN_HPP
