#ifndef _NET_HPP
#define _NET_HPP

// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#include <ostream>
#include <iomanip>

#include <map>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

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

class token_not_there : public std::runtime_error
{
public:
  token_not_there () : std::runtime_error("token not on this place") {}
  ~token_not_there() throw() {}
};

// Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
// cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
// It follows that an uint64_t is enough for now.
struct handle_t
{
public:
  typedef uint64_t T;
private:
  T v;
public:
  handle_t () : v(std::numeric_limits<uint64_t>::min()) {}
  static const T invalid (void) { return std::numeric_limits<uint64_t>::max(); }
  const T & operator * (void) const { return v; }
  const void operator ++ (void) { ++v; }
};

// bimap, that keeps a bijection between objects and some index
template<typename T>
class auto_bimap
{
private:
  // unordered, unique, viewable
  typedef boost::bimaps::unordered_set_of<T> elem_collection_t;

  // unordered, unique, viewable
  typedef boost::bimaps::unordered_set_of<handle_t::T> id_collection_t;

  typedef boost::bimap<elem_collection_t, id_collection_t> bimap_t;
  typedef typename bimap_t::value_type val_t;

  bimap_t bimap;
  handle_t h;

  const std::string description;

public:
  auto_bimap (const std::string & descr) : description (descr) {}

  typedef typename bimap_t::iterator iterator;

  iterator begin (void) { return bimap.begin(); }
  iterator end (void) { return bimap.end(); }

  typedef typename bimap_t::const_iterator const_iterator;

  const const_iterator begin (void) const { return bimap.begin(); }
  const const_iterator end (void) const { return bimap.end(); }

  const handle_t::T & get_id (const T & x) const throw (no_such)
  {
    typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

    if (it == bimap.left.end())
      throw no_such (description);

    return it->second;
  }

  const T & get_elem (const handle_t::T & i) const throw (no_such)
  {
    typename bimap_t::right_map::const_iterator it (bimap.right.find (i));

    if (it == bimap.right.end())
      throw no_such ("index for " + description);

    return it->second;
  }

  const handle_t::T add (const T & x) throw (already_there)
  {
    if (bimap.left.find (x) != bimap.left.end())
      throw already_there (description);

    handle_t::T i (*h); ++h;

    bimap.insert (val_t (x, i));

    return i;
  }

  const void erase (const handle_t::T & i) { bimap.right.erase (i); }

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
public:
  bi_const_it (const auto_bimap<T> & bm) : pos (bm.begin()), end (bm.end()) {}

  const bool has_more (void) const { return (pos != end) ? true : false; }
  void operator ++ (void) { ++pos; }
  const handle_t::T & operator * (void) const { return pos->right; }
};

// adjacency_matrix, grows on demand
class adjacency_matrix
{
public:
  typedef handle_t::T size_t;

private:
  size_t row;
  size_t col;

  handle_t::T * val;

  const inline size_t linear (const size_t & r, const size_t & c) const
  {
    return r * col + c;
  }

public:
  adjacency_matrix (const size_t & r, const size_t & c)
    throw (std::bad_alloc)
    : row (r)
    , col (c)
    , val (NULL)
  {
    val = new handle_t::T[row * col];

    if (val != NULL)
      std::fill (val, val + row * col, handle_t().invalid());
  }

  ~adjacency_matrix ()
  {
    if (val != NULL)
      delete[] val;
    
    val = NULL;
  }

  const handle_t::T & adjacent (const size_t & r, const size_t & c) const
    throw (std::out_of_range)
  {
    if (r > row - 1)
      throw std::out_of_range("row");

    if (c > col - 1)
      throw std::out_of_range("col");

    return val[linear (r, c)];
  }

  handle_t::T & adjacent (const size_t & r, const size_t & c)
    throw (std::bad_alloc)
  {
    if (r > row - 1)
      {
        handle_t::T * newval = new handle_t::T [(2 * row) * col];

        if (newval != NULL)
          {
            std::copy (val, val + row * col, newval);
            std::fill ( newval +      row  * col
                      , newval + (2 * row) * col
                      , handle_t().invalid()
                      );
          }

        delete[] val;

        val = newval;

        row *= 2;
      }

    if (c > col - 1)
      {
        handle_t::T * newval = new handle_t::T[row * (2 * col)];

        if (newval != NULL)
          for (size_t r (0); r < row; ++r)
            {
              std::copy ( val    +  r      *      col
                        , val    + (r + 1) *      col
                        , newval +  r      * (2 * col)
                        );
              std::fill ( newval + r * (2 * col) +     col
                        , newval + r * (2 * col) + 2 * col
                        , handle_t().invalid()
                        );
            }

        delete[] val;

        val = newval;

        col *= 2;
      }

    return val[linear (r, c)];
  }

  friend std::ostream & operator << (std::ostream &, const adjacency_matrix &);
};

std::ostream & operator << (std::ostream & s, const adjacency_matrix & m)
{
  s << "adjacency_matrix:";
  s << " (row = " << m.row << ", col = " << m.col << ")" << std::endl;

  s << std::setw(5) << "";

  for (adjacency_matrix::size_t c(0); c < m.col; ++c)
    s << std::setw(4) << c;

  s << std::endl;

  for (adjacency_matrix::size_t r(0); r < m.row; ++r)
    {
      s << std::setw(3) << r << "  ";

      for (adjacency_matrix::size_t c(0); c < m.col; ++c)
        {
          const handle_t::T v (m.val[m.linear (r, c)]);

          s << std::setw(4);

          if (v == handle_t().invalid())
            {
              s << ".";
            }
          else
            {
              s << v;
            }
        }

      s << std::endl;
    }

  return s;
};

// iterate through adjacencies
struct adj_const_it
{
private:
  const adjacency_matrix & m;
  const adjacency_matrix::size_t max;
  const handle_t::T fix;
  const bool fix_is_fst;

  adjacency_matrix::size_t pos;
  handle_t::T adj;

  inline bool is_invalid (void)
  {
    adj = (fix_is_fst == true) ? m.adjacent (fix, pos)
                               : m.adjacent (pos, fix)
                               ;

    return (adj == handle_t().invalid());
  }

  inline void step (void)
  {
    while (pos < max && is_invalid())
      ++pos;
  }

public:
  adj_const_it ( const adjacency_matrix & m_
               , const adjacency_matrix::size_t & max_
               , const handle_t::T & fix_
               , const bool & fix_is_fst_
               ) 
  : m (m_)
  , max (max_)
  , fix (fix_)
  , fix_is_fst (fix_is_fst_)
  , pos (0)
  , adj (handle_t().invalid())
  {
    step();
  }

  const bool has_more (void) const { return (pos != max) ? true : false; }
  void operator ++ (void) { ++pos; step(); }

  const handle_t::T & edge_id (void) const { return adj; }
  const adjacency_matrix::size_t & obj_id (void) const { return pos; }

  const handle_t::T & operator () (void) const { return edge_id(); }
  const adjacency_matrix::size_t & operator * (void) const { return obj_id(); }
};

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{
public:
  typedef handle_t::T pid_t;
  typedef handle_t::T tid_t;
  typedef handle_t::T eid_t;

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

private:
  adj_matrix adj_pt;
  adj_matrix adj_tp;

  adjacency_matrix::size_t max_place;
  adjacency_matrix::size_t max_transition;
  adjacency_matrix::size_t max_edge;
  adjacency_matrix::size_t num_places;
  adjacency_matrix::size_t num_transitions;
  adjacency_matrix::size_t num_edges;

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
  {};

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
    eid_t & a (m.adjacent (x, y));

    if (a != handle_t().invalid())
      throw already_there ("adjacency");

    const eid_t eid (emap.add (edge));

    a = eid;

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

  typedef adj_const_it adj_place_const_it;
  typedef adj_const_it adj_transition_const_it;

  const adj_place_const_it out_of_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_tp, max_place, tid, true);
  }

  const adj_place_const_it in_to_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_pt, max_place, tid, false);
  }

  const adj_transition_const_it out_of_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_pt, max_transition, pid, true);
  }

  const adj_transition_const_it in_to_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_tp, max_transition, pid, false);
  }

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

        assert (adj_pt.adjacent (pid, tid) == eid);

        adj_pt.adjacent (pid, tid) = handle_t().invalid();

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

        assert (adj_tp.adjacent (tid, pid) == eid);
        
        adj_tp.adjacent (tid, pid) = handle_t().invalid();

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

  template<typename P, typename T, typename E, typename O>
  friend std::ostream & operator << (std::ostream &, const net<P,T,E,O> &);
};

template<typename I>
std::ostream & operator << (std::ostream & s, const std::map<I, I> & m)
{
  for ( typename std::map<I, I>::const_iterator it (m.begin())
      ; it != m.end()
      ; ++it
      )
    s << " -- " << it->first << " => " << it -> second << std::endl;

  return s;
}

template<typename P, typename T, typename E, typename O>
std::ostream & operator << (std::ostream & s, const net<P,T,E,O> & n)
{
  s << n.pmap;
  s << n.tmap;
  s << n.emap;
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
