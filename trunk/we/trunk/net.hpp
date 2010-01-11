#ifndef _NET_HPP
#define _NET_HPP

// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#include <ostream>
#include <iomanip>

#include <map>
#include <set>
#include <vector>

#include <algorithm>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/bimap/support/lambda.hpp>

// exceptions
class no_such : public std::runtime_error
{
public:
  no_such (const std::string & msg) : std::runtime_error(msg) {}
  ~no_such() throw() {}
};

class already_there : public std::runtime_error
{
public:
  already_there (const std::string & msg) : std::runtime_error(msg) {}
  ~already_there() throw() {}
};

class transition_not_enabled : public std::runtime_error
{
public:
  transition_not_enabled (const std::string & msg) : std::runtime_error(msg) {}
  ~transition_not_enabled() throw () {}
};

// Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
// cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
// It follows that an uint64_t is enough for now.
namespace handle
{
  typedef uint64_t T;
  static const T invalid (std::numeric_limits<T>::max());

  struct handle_t
  {
  private:
    T v;
  public:
    handle_t () : v(std::numeric_limits<uint64_t>::min()) {}
    const T & operator * (void) const { return v; }
    const void operator ++ (void) { ++v; }
  };
};

// set with access to nth element
template<typename T>
struct svector
{
private:
  typedef typename std::vector<T> vec_t;
  typedef typename vec_t::iterator it;
  typedef typename vec_t::const_iterator const_it;
  typedef std::pair<it,it> pit_t;

  vec_t vec;
public:
  typedef typename vec_t::size_type size_type;

  it insert (const T & x)
  {
    const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

    return (std::distance (pit.first, pit.second) == 0)
      ? vec.insert (pit.second, x) : pit.first;
  }

  it erase (const T & x) throw (no_such)
  {
    const it pos (std::lower_bound (vec.begin(), vec.end(), x));

    return (pos == vec.end()) ? pos : vec.erase (pos);
  }

  typename vec_t::const_reference & at (typename vec_t::size_type n) const
  {
    return vec.at (n);
  }

  const_it begin (void) const { return vec.begin(); }
  const_it end (void) const { return vec.end(); }

  const bool empty (void) const { return vec.empty(); }
  const size_type size (void) const { return vec.size(); }

  const bool operator == (const svector<T> & other) const
  {
    return (vec == other.vec);
  }
};

// bimap, that keeps a bijection between objects and some index
template<typename T>
class auto_bimap
{
private:
  // unordered, unique, viewable
  typedef boost::bimaps::unordered_set_of<T> elem_collection_t;

  // unordered, unique, viewable
  typedef boost::bimaps::unordered_set_of<handle::T> id_collection_t;

  typedef boost::bimap<elem_collection_t, id_collection_t> bimap_t;
  typedef typename bimap_t::value_type val_t;

  bimap_t bimap;
  handle::handle_t h;

  const std::string description;

public:
  auto_bimap (const std::string & descr) : description (descr) {}

  typedef typename bimap_t::iterator iterator;

  iterator begin (void) { return bimap.begin(); }
  iterator end (void) { return bimap.end(); }

  typedef typename bimap_t::const_iterator const_iterator;

  const const_iterator begin (void) const { return bimap.begin(); }
  const const_iterator end (void) const { return bimap.end(); }

  const handle::T & get_id (const T & x) const throw (no_such)
  {
    typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

    if (it == bimap.left.end())
      throw no_such (description);

    return it->second;
  }

  const T & get_elem (const handle::T & i) const throw (no_such)
  {
    typename bimap_t::right_map::const_iterator it (bimap.right.find (i));

    if (it == bimap.right.end())
      throw no_such ("index for " + description);

    return it->second;
  }

  const handle::T add (const T & x) throw (already_there)
  {
    if (bimap.left.find (x) != bimap.left.end())
      throw already_there (description);

    handle::T i (*h); ++h;

    bimap.insert (val_t (x, i));

    return i;
  }

  const void erase (const handle::T & i) { bimap.right.erase (i); }

  const handle::T modify (const handle::T & i, const T & x)
    throw (no_such, already_there)
  {
    typename bimap_t::right_map::iterator it (bimap.right.find (i));

    if (it == bimap.right.end())
      throw no_such ("index for " + description);

    if (bimap.right.modify_data (it, boost::bimaps::_data = x) == false)
      throw already_there (description + " after modify");

    return i;
  }

  const handle::T replace (const handle::T & i, const T & x)
    throw (no_such, already_there)
  {
    typename bimap_t::right_map::iterator it (bimap.right.find (i));

    if (it == bimap.right.end())
      throw no_such ("index for " + description);

    if (bimap.right.replace_data (it, x) == false)
      throw already_there (description + " after replace");

    return i;
  }

  template<typename U>
  friend std::ostream & operator << (std::ostream &, const auto_bimap<U> &);
};

template<typename T>
std::ostream & operator << (std::ostream & s, const auto_bimap<T> & bm)
{
  typedef typename auto_bimap<T>::const_iterator bm_it;

  s << "bimap (" << bm.description << "):" << std::endl;

  for (bm_it it (bm.begin()), it_end (bm.end()); it != it_end; ++it)
    s << " -- " << it->left << " <=> " << it->right << std::endl;

  return s;
};

template<typename T>
struct bi_const_it
{
private:
  typedef typename auto_bimap<T>::const_iterator it;
  it pos;
  const it end;
  const std::size_t count_;
public:
  bi_const_it (const auto_bimap<T> & bm)
    : pos (bm.begin())
    , end (bm.end())
    , count_(std::distance(pos, end))
  {}

  const bool has_more (void) const { return (pos != end) ? true : false; }
  void operator ++ (void) { ++pos; }
  const handle::T & operator * (void) const { return pos->right; }
  const std::size_t count (void) const { return count_; }
};

// adjacency_matrix, grows on demand
class adjacency_matrix
{
public:
  typedef handle::T size_t;

private:
  size_t row;
  size_t col;

  typedef std::pair<size_t,handle::T> adj_t;
  typedef std::vector<adj_t> adj_vec_t;
  typedef std::vector<adj_vec_t> adj_table_t;

  adj_table_t table;
  adj_table_t tableT;

  const handle::T gen_get ( const size_t & x
                          , const size_t & y
                          , const adj_table_t & t
                          ) const
  {
    handle::T v (handle::invalid);

    for ( adj_vec_t::const_iterator it (t[x].begin())
        ; it != t[x].end() && v == handle::invalid
        ; ++it
        )
      if (it->first == y)
        v = it->second;

    return v;
  }

  void gen_clear (const size_t & x, const size_t & y, adj_table_t & t)
  {
    adj_vec_t::iterator it (t[x].begin());

    for (; it != t[x].end(); ++it)
      if (it->first == y)
        break;

    if (it != t[x].end())
      t[x].erase (it);
  }

public:
  typedef adj_vec_t::const_iterator const_it;
  typedef std::pair<const_it,const_it> vec_it;

  const vec_it get_vec_it (const size_t & x) const
  {
    return vec_it (table[x].begin(),table[x].end());
  }

  const vec_it get_vec_itT (const size_t & x) const
  {
    return vec_it (tableT[x].begin(),tableT[x].end());
  }

  adjacency_matrix (const size_t & r, const size_t & c)
    throw (std::bad_alloc)
    : row (r)
    , col (c)
    , table (row)
    , tableT (col)
  {}

  const handle::T get_adjacent (const size_t & r, const size_t & c) const
    throw (std::out_of_range)
  {
    if (r > row - 1)
      throw std::out_of_range("row");

    return gen_get (r, c, table);
  }

  const handle::T get_adjacentT (const size_t & r, const size_t & c) const
    throw (std::out_of_range)
  {
    if (c > col - 1)
      throw std::out_of_range("col");

    return gen_get (c, r, tableT);
  }

  void clear_adjacent (const size_t & r, const size_t & c)
    throw (std::out_of_range)
  {
    if (r > row - 1)
      throw std::out_of_range("row");

    gen_clear (r, c, table);

    if (c > col - 1)
      throw std::out_of_range("col");

    gen_clear (c, r, tableT);
  }

  void set_adjacent (const size_t & r, const size_t & c, const size_t & x)
    throw (std::bad_alloc)
  {
    if (r > row - 1)
      {
        row *= 2;

        table.resize (row);
      }

    if (c > col - 1)
      {
        col *= 2;

        tableT.resize (col);
      }

    table[r].push_back (adj_t (c, x));
    tableT[c].push_back (adj_t (r, x));
  }

  friend std::ostream & operator << (std::ostream &, const adjacency_matrix &);
};

std::ostream & operator << (std::ostream & s, const adjacency_matrix & m)
{
  s << "adjacency_matrix:";
  s << " (row = " << m.row << ", col = " << m.col << ")" << std::endl;

  const unsigned int w (3);

  s << std::setw(w) << "";

  for (size_t c (0); c < m.col; ++c)
    s << std::setw (w) << c;

  s << std::endl;

  for (size_t r (0); r < m.row; ++r)
    {
      s << std::setw(w) << r;

      for (size_t c (0); c < m.col; ++c)
        {
          const handle::T adj (m.get_adjacent (r, c));

          s << std::setw(w);

          if (adj == handle::invalid)
            s << ".";
          else
            s << adj;
        }

      s << std::endl;
    }

  s << "  transposed:" << std::endl << std::setw(w) << "";

  for (size_t r (0); r < m.row; ++r)
    s << std::setw (w) << r;

  s << std::endl;

  for (size_t c (0); c < m.col; ++c)
    {
      s << std::setw(w) << c;

      for (size_t r (0); r < m.row; ++r)
        {
          const handle::T adj (m.get_adjacentT (r, c));

          s << std::setw(w);

          if (adj == handle::invalid)
            s << ".";
          else
            s << adj;
        }

      s << std::endl;
    }

  return s;
};

// iterate through adjacencies
struct adj_const_it
{
private:
  adjacency_matrix::vec_it vec_it;
  adjacency_matrix::const_it pos;
  const adjacency_matrix::const_it end;

public:
  adj_const_it ( const adjacency_matrix & m
               , const handle::T & x
               , const bool & fix_is_fst
               )
    : vec_it (fix_is_fst ? m.get_vec_it (x) : m.get_vec_itT (x))
    , pos (vec_it.first)
    , end (vec_it.second)
  {}

  const bool has_more (void) const { return (pos != end) ? true : false; }
  void operator ++ (void) { ++pos; }

  const handle::T & edge_id (void) const { return pos->second; }
  const adjacency_matrix::size_t & obj_id (void) const { return pos->first; }

  const handle::T & operator () (void) const { return edge_id(); }
  const adjacency_matrix::size_t & operator * (void) const { return obj_id(); }
};

template<typename Token, typename PID>
struct omap_t
{
public:
  typedef boost::bimaps::unordered_multiset_of<Token> token_collection_t;
  typedef boost::bimaps::unordered_multiset_of<PID> place_collection_t;

  typedef typename boost::bimap< token_collection_t
                               , place_collection_t
                               , boost::bimaps::unordered_multiset_of_relation<>
                               > bimap_t;
  typedef typename bimap_t::value_type value_t;
  typedef typename bimap_t::right_map::const_iterator place_const_it;
};

// iterate through the tokens on a place
template<typename Token, typename PID>
struct token_on_place_it
{
private:
  typedef typename omap_t<Token,PID>::place_const_it pc_it;
  pc_it pos;
  const pc_it end;
  const std::size_t count_;
public:
  token_on_place_it (std::pair<pc_it, pc_it> its)
    : pos (its.first)
    , end (its.second)
    , count_(std::distance (pos, end))
  {}
  const bool has_more (void) const { return (pos != end) ? true : false; }
  void operator ++ (void) { ++pos; }
  const Token & operator * (void) const { return pos->second; }
  const std::size_t count (void) const { return count_; }
};

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{
public:
  typedef handle::T pid_t;
  typedef handle::T tid_t;
  typedef handle::T eid_t;

  enum edge_type {PT,TP};

private:
  auto_bimap<Place> pmap; // Place <-> internal id
  auto_bimap<Transition> tmap; // Transition <-> internal id
  auto_bimap<Edge> emap; // Edge <-> internal id

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

  typedef adjacency_matrix adj_matrix;

  adj_matrix adj_pt;
  adj_matrix adj_tp;

  adjacency_matrix::size_t max_place;
  adjacency_matrix::size_t max_transition;
  adjacency_matrix::size_t max_edge;
  adjacency_matrix::size_t num_places;
  adjacency_matrix::size_t num_transitions;
  adjacency_matrix::size_t num_edges;

  typedef typename omap_t<Token,pid_t>::bimap_t obimap_t;
  typedef typename omap_t<Token,pid_t>::value_t oval_t;
  typedef typename obimap_t::const_iterator omap_const_it;
  typedef typename obimap_t::iterator omap_it;
  typedef typename std::pair<omap_it,omap_it> omap_range_it;

  obimap_t omap;

public:
  net ( const adjacency_matrix::size_t & places_ = 100
      , const adjacency_matrix::size_t & transitions_ = 250
      )
    throw (std::bad_alloc)
    : pmap ("place")
    , tmap ("transition")
    , emap ("edge name")
    , emap_in_p ()
    , emap_out_p ()
    , emap_in_t ()
    , emap_out_t ()
    , adj_pt (places_, transitions_)
    , adj_tp (transitions_, places_)
    , max_place (0)
    , max_transition (0)
    , max_edge (0)
    , num_places (0)
    , num_transitions (0)
    , num_edges (0)
    , enabled ()
  {};

  // numbers of elements
  const adjacency_matrix::size_t get_num_places (void) const
  {
    return num_places;
  }

  const adjacency_matrix::size_t get_num_transitions (void) const
  {
    return num_transitions;
  }

  const adjacency_matrix::size_t get_num_edges (void) const
  {
    return num_edges;
  }

  // get id
  const pid_t & get_place_id (const Place & place) const throw (no_such)
  {
    return pmap.get_id (place);
  }

  const tid_t & get_transition_id (const Transition & transition) const
    throw (no_such)
  {
    return tmap.get_id (transition);
  }

  const eid_t & get_edge_id (const Edge & edge) const throw (no_such)
  {
    return emap.get_id (edge);
  }

  // get element
  const Place & place (const pid_t & pid) const throw (no_such)
  {
    return pmap.get_elem (pid);
  }

  const Transition & transition (const tid_t & tid) const throw (no_such)
  {
    return tmap.get_elem (tid);
  }

  const Edge & edge (const eid_t & eid) const throw (no_such)
  {
    return emap.get_elem (eid);
  }

  // add element
  const pid_t add_place (const Place & place) throw (already_there)
  {
    ++max_place;
    ++num_places;

    return pmap.add (place);
  }

  const tid_t add_transition (const Transition & transition)
    throw (already_there)
  {
    ++max_transition;
    ++num_transitions;

    return tmap.add (transition);
  }

private:
  const eid_t add_edge
  ( const Edge & edge
  , const adjacency_matrix::size_t & x
  , const adjacency_matrix::size_t & y
  , adj_matrix & m
  )
    throw (no_such, already_there)
  {
    try
      {
        if (m.get_adjacent (x, y) != handle::invalid)
          throw already_there ("adjacency");
      }
    catch (std::out_of_range)
      {
        /* do nothing, was not there */
      }

    const eid_t eid (emap.add (edge));

    m.set_adjacent (x, y, eid);

    ++max_edge;
    ++num_edges;

    return eid;
  }

public:
  const eid_t add_edge_place_to_transition
  (const Edge & edge, const pid_t & pid, const tid_t & tid)
    throw (no_such, already_there)
  {
    const eid_t eid (add_edge (edge, pid, tid, adj_pt));

    emap_out_p[eid] = pid;
    emap_in_t[eid] = tid;

    return eid;
  }

  const eid_t add_edge_place_to_transition
  (const Edge & edge, const Place & place, const Transition & transition)
    throw (no_such, already_there)
  {
    const pid_t pid (get_place_id (place));
    const tid_t tid (get_transition_id (transition));

    return add_edge_place_to_transition (edge, pid, tid);
  }

  const eid_t add_edge_transition_to_place
  (const Edge & edge, const tid_t & tid, const pid_t & pid)
    throw (no_such, already_there)
  {
    const eid_t eid (add_edge (edge, tid, pid, adj_tp));

    emap_in_p[eid] = pid;
    emap_out_t[eid] = tid;

    return eid;
  }

  const eid_t add_edge_transition_to_place
  (const Edge & edge, const Transition & transition, const Place & place)
    throw (no_such, already_there)
  {
    const pid_t pid (get_place_id (place));
    const tid_t tid (get_transition_id (transition));

    return add_edge_transition_to_place (edge, tid, pid);
  }

  // iterate through elements
  typedef bi_const_it<Place> place_const_it;
  typedef bi_const_it<Transition> transition_const_it;
  typedef bi_const_it<Edge> edge_const_it;

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
  typedef adj_const_it adj_place_const_it;
  typedef adj_const_it adj_transition_const_it;

  const adj_place_const_it out_of_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_tp, tid, true);
  }

  const adj_place_const_it in_to_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_pt, tid, false);
  }

  const adj_transition_const_it out_of_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_pt, pid, true);
  }

  const adj_transition_const_it in_to_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_tp, pid, false);
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

  const eid_t & delete_edge (const Edge & edge) throw (no_such)
  {
    return delete_edge (get_edge_id (edge));
  }

  const pid_t & delete_place (const pid_t & pid) throw (no_such)
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

  const pid_t & delete_place (const Place & place) throw (no_such)
  {
    return delete_place (get_place_id (place));
  }

  const tid_t & delete_transition (const tid_t & tid) throw (no_such)
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

  const tid_t & delete_transition (const Transition & transition)
    throw (no_such)
  {
    return delete_transition (get_transition_id (transition));
  }

  // modify and replace
  // erased in case of conflict after modification
  const pid_t modify_place (const pid_t & pid, const Place & place)
    throw (no_such, already_there)
  {
    return pmap.modify (pid, place);
  }

  const pid_t modify_place (const Place & old_place, const Place & new_place)
    throw (no_such, already_there)
  {
    return modify_place (get_place_id (old_place), new_place);
  }

  // kept old value in case of conflict after modification
  const pid_t replace_place (const pid_t & pid, const Place & place)
    throw (no_such, already_there)
  {
    return pmap.replace (pid, place);
  }

  const pid_t replace_place (const Place & old_place, const Place & new_place)
    throw (no_such, already_there)
  {
    return replace_place (get_place_id (old_place), new_place);
  }

  const tid_t modify_transition ( const tid_t & tid
                                , const Transition & transition
                                )
    throw (no_such, already_there)
  {
    return tmap.modify (tid, transition);
  }

  const tid_t modify_transition ( const Transition & old_transition
                                , const Transition & new_transition
                                )
    throw (no_such, already_there)
  {
    return tmap.modify (get_transition_id (old_transition), new_transition);
  }

  const tid_t replace_transition ( const tid_t & tid
                                 , const Transition & transition
                                 )
    throw (no_such, already_there)
  {
    return tmap.replace (tid, transition);
  }

  const tid_t replace_transition ( const Transition & old_transition
                                 , const Transition & new_transition
                                 )
    throw (no_such, already_there)
  {
    return tmap.replace (get_transition_id (old_transition), new_transition);
  }

  const eid_t modify_edge (const eid_t & eid, const Edge & edge)
    throw (no_such, already_there)
  {
    return emap.modify (eid, edge);
  }

  const eid_t modify_edge (const Edge & old_edge, const Edge & new_edge)
    throw (no_such, already_there)
  {
    return emap.modify (get_edge_id (old_edge), new_edge);
  }

  const eid_t replace_edge (const eid_t & eid, const Edge & edge)
    throw (no_such, already_there)
  {
    return emap.replace (eid, edge);
  }

  const eid_t replace_edge (const Edge & old_edge, const Edge & new_edge)
    throw (no_such, already_there)
  {
    return emap.replace (get_edge_id (old_edge), new_edge);
  }

  // deal with tokens
  typedef svector<tid_t> enabled_t;

private:
  enabled_t enabled;

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
    const bool successful (omap.insert (oval_t (token, pid)).second);

    if (successful)
      add_enabled_transitions (pid);

    return successful;
  }

  const bool put_token (const Place & place, const Token & token)
    throw (no_such)
  {
    return put_token (get_place_id (place), token);
  }

  typedef token_on_place_it<Token, pid_t> token_place_it;

  const token_place_it get_token (const pid_t & pid) const
  {
    return token_place_it (omap.right.equal_range (pid));
  }

  const token_place_it get_token (const Place & place) const
    throw (no_such)
  {
    return get_token (get_place_id (place));
  }

  const bool has_token (const pid_t & pid) const
  {
    return (omap.right.find (pid) != omap.right.end());
  }

  const bool has_token (const Place & place) const
  {
    return has_token (get_place_id (place));
  }

  const std::size_t delete_one_token (const pid_t & pid, const Token & token)
  {
    omap_range_it range_it (omap.equal_range (oval_t (token, pid)));

    const std::size_t dist (std::distance (range_it.first, range_it.second));

    if (dist > 0)
      {
        omap.erase (range_it.first);
        del_enabled_transitions (pid);
      }

    return (dist > 0) ? 1 : 0;
  }

  const std::size_t delete_one_token (const Place & place, const Token & token)
    throw (no_such)
  {
    return delete_one_token (get_place_id (place), token);
  }

  const std::size_t delete_all_token (const pid_t & pid, const Token & token)
  {
    omap_range_it range_it (omap.equal_range (oval_t (token, pid)));

    omap.erase (range_it.first, range_it.second);

    del_enabled_transitions (pid);

    return std::distance (range_it.first, range_it.second);
  }

  const std::size_t delete_all_token (const Place & place, const Token & token)
    throw (no_such)
  {
    return delete_all_token (get_place_id (place), token);
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

  void fire (const tid_t & tid) throw (transition_not_enabled)
  {
    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        const token_place_it tp (get_token (*pit));

        if (!tp.has_more())
          throw transition_not_enabled ("during call of fire");

        delete_one_token (*pit, *tp);
      }

    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      put_token (*pit, Token());
  }

  // output
  template<typename P, typename T, typename E, typename O>
  friend std::ostream & operator << (std::ostream &, const net<P,T,E,O> &);
};

template<typename I>
std::ostream & operator << (std::ostream & s, const std::map<I, I> & m)
{
  typedef typename std::map<I, I>::const_iterator IT;

  for (IT it (m.begin()); it != m.end(); ++it)
    s << " -- " << it->first << " => " << it -> second << std::endl;

  return s;
}

template<typename P, typename T, typename E, typename O>
std::ostream & operator << (std::ostream & s, const net<P,T,E,O> & n)
{
  s << "##### OP<<" << std::endl;
  s << n.pmap;
  s << n.tmap;
  s << n.emap;

  s << "bimap (token):" << std::endl;

  for (BOOST_AUTO(tp, n.omap.begin()); tp != n.omap.end(); ++tp)
    s << "on place "
      << tp->right << " [" << n.place (tp->right) << "]"
      << ": "
      << tp->left
      << std::endl;

  s << "emap_in_p:" << std::endl << n.emap_in_p;
  s << "emap_out_p:" << std::endl << n.emap_out_p;
  s << "emap_in_t:" << std::endl << n.emap_in_t;
  s << "emap_out_t:" << std::endl << n.emap_out_t;
  s << "max_place = " << n.max_place << std::endl;
  s << "max_transition = " << n.max_transition << std::endl;
  s << "max_edge = " << n.max_edge << std::endl;
  s << "num_places = " << n.num_places << std::endl;
  s << "num_transitions = " << n.num_transitions << std::endl;
  s << "num_edges = " << n.num_edges << std::endl;
  s << "adj_pt: " << n.adj_pt;
  s << "adj_tp: " << n.adj_tp;

  return s;
}

#endif // _NET_HPP
