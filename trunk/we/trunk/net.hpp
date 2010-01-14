// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_HPP
#define _NET_HPP

#include <adjacency.hpp>
#include <bijection.hpp>
#include <multirel.hpp>
#include <svector.hpp>

#include <limits>

#include <map>

#include <boost/function.hpp>

namespace petri_net
{
  namespace exception
  {
    class transition_not_enabled : public std::runtime_error
    {
    public:
      transition_not_enabled (const std::string & msg) 
        : std::runtime_error(msg) 
      {}
      ~transition_not_enabled() throw () {}
    };
  }

  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
  // It follows that an uint64_t is enough for now.
  typedef uint64_t pid_t;
  typedef uint64_t tid_t;
  typedef uint64_t eid_t;

  static const eid_t eid_invalid (std::numeric_limits<eid_t>::max());
};

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
  const petri_net::pid_t get_pid 
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.first;
  };

  template<typename Token>
  const petri_net::eid_t get_eid 
  (const typename Traits<Token>::place_via_edge_t & place_via_edge)
  {
    return place_via_edge.second;
  };

  template<typename Token>
  const petri_net::pid_t get_pid 
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_pid<Token> (token_input.second);
  };

  template<typename Token>
  const petri_net::eid_t get_eid 
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return get_eid<Token> (token_input.second);
  };

  template<typename Token>
  const Token get_token
  (const typename Traits<Token>::token_input_t & token_input)
  {
    return token_input.first;
  };

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
    const output_t operator () ( const input_t &
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
  template<typename Token, const Token F (const Token &)>
  class PassThroughWithFun
  {
  private:
    typedef typename Traits<Token>::input_t input_t;

    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef typename Traits<Token>::output_t output_t;

  public:
    const output_t operator () ( const input_t & input
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
  inline const Token Const (const Token & token)
  {
    return token;
  }

  // simple pass the tokens through
  template<typename Token>
  class PassThrough : public PassThroughWithFun<Token, Const<Token> >
  {};

  // apply a function, that depends on the edges only
  template< typename Token
          , const typename Traits<Token>::edges_only_t 
               F (const typename Traits<Token>::edges_only_t &)
          >
  class EdgesOnly
  {
  private:
    typedef typename Traits<Token>::input_t input_t;

    typedef typename Traits<Token>::output_descr_t output_descr_t;
    typedef typename std::pair<Token, petri_net::pid_t> token_on_place_t;
    typedef typename Traits<Token>::output_t output_t;

    typedef typename Traits<Token>::edges_only_t edges_only_t;

  public:
    const output_t operator () ( const input_t & input
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
      edges_only_t out (F (in));

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
};

namespace petri_net
{
// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{
public:
  enum edge_type {PT,TP};

  typedef svector<tid_t> enabled_t;

  typedef bijection::bi_const_it<Place,pid_t> place_const_it;
  typedef bijection::bi_const_it<Transition,tid_t> transition_const_it;
  typedef bijection::bi_const_it<Edge,eid_t> edge_const_it;

  typedef adjacency::const_it<pid_t,eid_t> adj_place_const_it;
  typedef adjacency::const_it<tid_t,eid_t> adj_transition_const_it;

  typedef multirel::right_const_it<Token, pid_t> token_place_it;

  typedef TransitionFunction::Traits<Token> tf_traits;
  typedef typename tf_traits::place_via_edge_t place_via_edge_t;
  typedef typename tf_traits::token_input_t token_input_t;
  typedef typename tf_traits::input_t input_t;

  typedef typename tf_traits::output_descr_t output_descr_t;
  typedef typename tf_traits::output_t output_t;

  typedef boost::function<output_t (input_t &, output_descr_t &)> transfun_t;

private:
  bijection::bijection<Place,pid_t> pmap; // Place <-> internal id
  bijection::bijection<Transition,tid_t> tmap; // Transition <-> internal id
  bijection::bijection<Edge,eid_t> emap; // Edge <-> internal id

  typedef std::map<eid_t, pid_t> map_pid_t;
  typedef typename map_pid_t::iterator map_pid_it_t;
  typedef typename map_pid_t::const_iterator map_pid_const_it_t;

  map_pid_t emap_in_p; // internal edge id -> internal place id
  map_pid_t emap_out_p; // internal edge id -> internal place id

  typedef std::map<eid_t, tid_t> map_tid_t;
  typedef typename map_tid_t::iterator map_tid_it_t;
  typedef typename map_tid_t::const_iterator map_tid_const_it_t;

  map_tid_t emap_in_t; // internal edge id -> internal transition id
  map_tid_t emap_out_t; // internal edge id -> internal transition id

  adjacency::table<pid_t,tid_t,eid_t> adj_pt;
  adjacency::table<tid_t,pid_t,eid_t> adj_tp;

  pid_t num_places;
  tid_t num_transitions;
  eid_t num_edges;

  typename multirel::multirel<Token,pid_t> token_place_rel;

  enabled_t enabled;

  std::map<tid_t, transfun_t> transfun;

public:
  net (const pid_t & _places = 10, const tid_t & _transitions = 10)
    throw (std::bad_alloc)
    : pmap ("place")
    , tmap ("transition")
    , emap ("edge name")
    , emap_in_p ()
    , emap_out_p ()
    , emap_in_t ()
    , emap_out_t ()
    , adj_pt (eid_invalid, _places, _transitions)
    , adj_tp (eid_invalid, _transitions, _places)
    , num_places (0)
    , num_transitions (0)
    , num_edges (0)
    , token_place_rel ()
    , enabled ()
  {};

  // numbers of elements
  const pid_t get_num_places (void) const
  {
    return num_places;
  }

  const tid_t get_num_transitions (void) const
  {
    return num_transitions;
  }

  const eid_t get_num_edges (void) const
  {
    return num_edges;
  }

  // get id
  const pid_t & get_place_id (const Place & place) const
    throw (bijection::exception::no_such)
  {
    return pmap.get_id (place);
  }

  const tid_t & get_transition_id (const Transition & transition) const
    throw (bijection::exception::no_such)
  {
    return tmap.get_id (transition);
  }

  const eid_t & get_edge_id (const Edge & edge) const
    throw (bijection::exception::no_such)
  {
    return emap.get_id (edge);
  }

  // get element
  const Place & place (const pid_t & pid) const
    throw (bijection::exception::no_such)
  {
    return pmap.get_elem (pid);
  }

  const Transition & transition (const tid_t & tid) const
    throw (bijection::exception::no_such)
  {
    return tmap.get_elem (tid);
  }

  const Edge & edge (const eid_t & eid) const
    throw (bijection::exception::no_such)
  {
    return emap.get_elem (eid);
  }

  // add element
  const pid_t add_place (const Place & place)
    throw (bijection::exception::already_there)
  {
    ++num_places;

    return pmap.add (place);
  }

  const tid_t set_transition_function (const tid_t & tid, const transfun_t & f)
  {
    transfun[tid] = f;

    return tid;
  }

  const tid_t add_transition 
  ( const Transition & transition
  , const transfun_t & f = TransitionFunction::Default<Token>()
  )
    throw (bijection::exception::already_there)
  {
    ++num_transitions;

    return set_transition_function (tmap.add (transition), f);
  }

private:
  template<typename ROW, typename COL>
  const eid_t add_edge
  ( const Edge & edge
  , const ROW & r
  , const COL & c
  , adjacency::table<ROW,COL,eid_t> & m
  )
    throw (bijection::exception::already_there)
  {
    if (m.get_adjacent (r, c) != eid_invalid)
      throw bijection::exception::already_there ("adjacency");

    const eid_t eid (emap.add (edge));

    m.set_adjacent (r, c, eid);

    ++num_edges;

    return eid;
  }

public:
  const eid_t add_edge_place_to_transition
  (const Edge & edge, const pid_t & pid, const tid_t & tid)
    throw (bijection::exception::already_there)
  {
    const eid_t eid (add_edge<pid_t,tid_t> (edge, pid, tid, adj_pt));

    emap_out_p[eid] = pid;
    emap_in_t[eid] = tid;

    return eid;
  }

  const eid_t add_edge_transition_to_place
  (const Edge & edge, const tid_t & tid, const pid_t & pid)
    throw (bijection::exception::already_there)
  {
    const eid_t eid (add_edge<tid_t,pid_t> (edge, tid, pid, adj_tp));

    emap_in_p[eid] = pid;
    emap_out_t[eid] = tid;

    return eid;
  }

  // iterate through elements
  const place_const_it places (void) const
  {
    return place_const_it (pmap);
  }

  const transition_const_it transitions (void) const
  {
    return transition_const_it (tmap);
  }

  const edge_const_it edges (void) const
  {
    return edge_const_it (emap);
  }

  // iterate through adjacencies
  const adj_place_const_it out_of_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_tp.row_const_it (tid));
  }

  const adj_place_const_it in_to_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_pt.col_const_it (tid));
  }

  const adj_transition_const_it out_of_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_pt.row_const_it (pid));
  }

  const adj_transition_const_it in_to_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_tp.col_const_it (pid));
  }

  // get edge info
  const edge_type get_edge_info ( const eid_t & eid
                                , pid_t & pid
                                , tid_t & tid
                                ) const
  {
    const map_pid_const_it_t out_p (emap_out_p.find (eid));

    if (out_p != emap_out_p.end())
      {
        const map_tid_const_it_t in_t (emap_in_t.find (eid));

        pid = out_p->second;
        tid = in_t->second;

        return PT;
      }
    else
      {
        const map_pid_const_it_t in_p (emap_in_p.find (eid));
        const map_tid_const_it_t out_t (emap_out_t.find (eid));

        tid = out_t->second;
        pid = in_p->second;

        return TP;
      }
  }

  // delete elements
  const eid_t & delete_edge (const eid_t & eid)
  {
    const map_pid_it_t out_p (emap_out_p.find (eid));

    if (out_p != emap_out_p.end())
      {
        // place -> transition

        const map_tid_it_t in_t (emap_in_t.find (eid));

        assert (in_t != emap_in_t.end());

        const pid_t pid (out_p->second);
        const tid_t tid (in_t->second);

        assert (adj_pt.get_adjacent (pid, tid) == eid);

        adj_pt.clear_adjacent (pid, tid);

        emap_in_t.erase (in_t);
        emap_out_p.erase (out_p);
      }
    else
      {
        // transition -> place

        const map_pid_it_t in_p (emap_in_p.find (eid));
        const map_tid_it_t out_t (emap_out_t.find (eid));

        assert (out_t != emap_out_t.end());
        assert (in_p != emap_in_p.end());

        const tid_t tid (out_t->second);
        const pid_t pid (in_p->second);

        assert (adj_tp.get_adjacent (tid, pid) == eid);

        adj_tp.clear_adjacent (tid, pid);

        emap_in_p.erase (in_p);
        emap_out_t.erase (out_t);
      }

    emap.erase (eid);

    --num_edges;

    return eid;
  }

  const pid_t & delete_place (const pid_t & pid)
    throw (bijection::exception::no_such)
  {
    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge (tit());

    for ( adj_transition_const_it tit (in_to_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge (tit());

    pmap.erase (pid);

    --num_places;

    return pid;
  }

  const tid_t & delete_transition (const tid_t & tid)
    throw (bijection::exception::no_such)
  {
    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      delete_edge (pit());

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      delete_edge (pit());

    tmap.erase (tid);

    --num_transitions;

    return tid;

  }

  // modify and replace
  // erased in case of conflict after modification
  const pid_t modify_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return pmap.modify (pid, place);
  }

  // kept old value in case of conflict after modification
  const pid_t replace_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return pmap.replace (pid, place);
  }

  const tid_t modify_transition ( const tid_t & tid
                                , const Transition & transition
                                )
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return tmap.modify (tid, transition);
  }

  const tid_t replace_transition ( const tid_t & tid
                                 , const Transition & transition
                                 )
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return tmap.replace (tid, transition);
  }

  const eid_t modify_edge (const eid_t & eid, const Edge & edge)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return emap.modify (eid, edge);
  }

  const eid_t replace_edge (const eid_t & eid, const Edge & edge)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return emap.replace (eid, edge);
  }

  // deal with tokens
private:
  void add_enabled_transitions (const pid_t & pid)
  {
    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      if (can_fire (*tit))
        enabled.insert (*tit);
  }

  void del_enabled_transitions (const pid_t & pid)
  {
    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      if (!can_fire (*tit))
        enabled.erase (*tit);
  }

public:  
  const enabled_t & enabled_transitions (void) const
  {
    return enabled;
  }

  const void verify_enabled_transitions (void) const
  {
    enabled_t comp;

    for (transition_const_it t (transitions()); t.has_more(); ++t)
      if (can_fire (*t))
        comp.insert (*t);

    assert (comp == enabled);
  }

  const bool put_token (const pid_t & pid, const Token & token)
  {
    const bool successful (token_place_rel.add (token, pid));

    if (successful)
      add_enabled_transitions (pid);

    return successful;
  }

  const token_place_it get_token (const pid_t & pid) const
  {
    return token_place_rel.left_of (pid);
  }

  const bool has_token (const pid_t & pid) const
  {
    return token_place_rel.contains_right (pid);
  }

  const std::size_t delete_one_token (const pid_t & pid, const Token & token)
  {
    const std::size_t dist (token_place_rel.delete_one (token, pid));

    if (dist > 0)
      del_enabled_transitions (pid);

    return (dist > 0) ? 1 : 0;
  }

  const std::size_t delete_all_token (const pid_t & pid, const Token & token)
  {
    del_enabled_transitions (pid);

    return token_place_rel.delete_all (token, pid);
  }

  // WORK HERE: implement more efficient
  const std::size_t replace_one_token
  (const pid_t & pid, const Token & old_token, const Token & new_token)
  {
    const std::size_t k (delete_one_token (pid, old_token));

    if (k > 0)
      put_token (pid, new_token);

    return k;
  }

  // WORK HERE: implement more efficient
  const std::size_t replace_all_token
  (const pid_t & pid, const Token & old_token, const Token & new_token)
  {
    const std::size_t k (delete_all_token (pid, old_token));

    for (std::size_t i (0); i < k; ++i)
      put_token (pid, new_token);

    return k;
  }

  // FIRE
  const bool can_fire (const tid_t & tid) const
  {
    bool can_fire = true;

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more() && can_fire
        ; ++pit
        )
      can_fire = has_token (*pit);

    return can_fire;
  }

  void fire (const tid_t & tid) throw (exception::transition_not_enabled)
  {
    input_t input;
    output_descr_t output_descr;

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        const token_place_it tp (get_token (*pit));

        if (!tp.has_more())
          throw exception::transition_not_enabled ("during call of fire");

        input.push_back (token_input_t (*tp, place_via_edge_t(*pit, pit())));

        delete_one_token (*pit, *tp);
      }

    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      output_descr.push_back (place_via_edge_t (*pit, pit()));

    const output_t output (transfun[tid](input, output_descr));

    for ( typename output_t::const_iterator out (output.begin())
        ; out != output.end()
        ; ++out
        )
      put_token (out->second, out->first);
  }
};
}

#endif // _NET_HPP
