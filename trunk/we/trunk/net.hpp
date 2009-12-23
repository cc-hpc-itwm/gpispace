
// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#include <boost/bimap.hpp>
#include <map>

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

// bimap, that keeps a bijection between objects and some index
template<typename T, typename I = unsigned long>
class auto_bimap
{
private:
  typedef boost::bimap<T,I> bimap_t;
  typedef typename bimap_t::value_type val_t;

  bimap_t bimap;
  I num;

  const std::string description;

public:
  auto_bimap (const std::string descr, const I initial = 1)
  : bimap ()
  , num (initial)
  , description (descr)
  {}

  const typename bimap_t::const_iterator it_begin (void) const
  {
    return bimap.begin();
  }

  const typename bimap_t::const_iterator it_end (void) const
  {
    return bimap.end();
  }

  const I get_id (const T x) const throw (no_such)
  {
    typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

    if (it == bimap.left.end())
      throw no_such (description);

    return it->second;
  }

  const T get_elem (const I i) const throw (no_such)
  {
    typename bimap_t::right_map::const_iterator it (bimap.right.find (i));

    if (it == bimap.right.end())
      throw no_such ("index for " + description);

    return it->second;
  }

  const I add (const T x) throw (already_there)
  {
    if (bimap.left.find (x) != bimap.left.end())
      throw already_there (description);

    I i (num);

    bimap.insert (val_t (x, i));

    ++num;

    return i;
  }

  const void erase (const T x)
  {
    bimap.left.erase (x);
  }
};

// function object to get elems from an auto_bimap
template<typename T, typename I>
struct biget
{
private:
  const auto_bimap<T, I> & bim;
public:
  biget (const auto_bimap<T, I> & bim_) : bim(bim_) {}
  const T get (const I i) const { return bim.get_elem (i); }
};

template<typename T, typename I>
struct bi_it
{
private:
  typedef typename boost::bimap<T,I>::const_iterator it;

  it pos;
  const it end;
public:
  bi_it (const auto_bimap<T,I> & bm)
    : pos (bm.it_begin())
    , end (bm.it_end())
  {}

  const bool has_more (void) const { return (pos != end) ? true : false; }
  void operator ++ (void) { ++pos; }
  const T operator * (void) { return pos->left; }
};

// adjacency_matrix, grows on demand
template<typename IDX = unsigned long, typename ID = unsigned long>
class adjacency_matrix
{
private:
  BOOST_STATIC_ASSERT(sizeof(ID) == sizeof(IDX));
  BOOST_STATIC_ASSERT(std::numeric_limits<ID>::is_integer);
  BOOST_STATIC_ASSERT(std::numeric_limits<IDX>::is_integer);
  BOOST_STATIC_ASSERT(  std::numeric_limits<ID>::is_signed
                     == std::numeric_limits<IDX>::is_signed);

  IDX row;
  IDX col;

  ID * val;
  ID invalid;

  const inline IDX linear (const IDX r, const IDX c) const
  {
    return r * col + c;
  }

public:
  adjacency_matrix (const IDX row_, const IDX col_, const ID invalid_ = 0)
    throw (std::bad_alloc)
    : row     (row_)
    , col     (col_)
    , val     (NULL)
    , invalid (invalid_)
  {
    

    val = new ID[row * col];

    if (val != NULL)
      std::fill (val, val + row * col, invalid);
  }

  ~adjacency_matrix ()
  {
    if (val != NULL)
      delete[] val;
    
    val = NULL;
  }

  const ID get_adjacent_nogrow (const IDX r, const IDX c) const
    throw (std::out_of_range)
  {
    if (r > row - 1)
      throw std::out_of_range("row");

    if (c > col - 1)
      throw std::out_of_range("col");

    return val[linear (r, c)];
  }

  ID & adjacent (const IDX r, const IDX c) throw (std::bad_alloc)
  {
    if (r > row - 1)
      {
        ID * newval = new ID [(2 * row) * col];

        if (newval != NULL)
          {
            std::copy (val, val + row * col, newval);
            std::fill (newval + row * col, newval + 2 * row * col, invalid);
          }

        delete[] val;

        val = newval;

        row *= 2;
      }

    if (c > col - 1)
      {
        ID * newval = new ID[row * (2 * col)];

        if (newval != NULL)
          for (IDX r (0); r < row; ++r)
            {
              std::copy ( val    +  r      *      col
                        , val    + (r + 1) *      col
                        , newval +  r      * (2 * col)
                        );
              std::fill ( newval + r * (2 * col) +     col
                        , newval + r * (2 * col) + 2 * col
                        , invalid
                        );
            }

        delete[] val;

        val = newval;

        col *= 2;
      }

    return val[linear (r, c)];
  }
};

// iterate through adjacencies
template <typename IDX, typename ID>
struct adj_it
{
private:
  typedef adjacency_matrix<IDX, ID> mat_t;

  const mat_t & m;
  const IDX max;
  const ID fix;
  const bool fix_is_fst;
  const ID invalid;

  IDX pos;
  ID adj;

  inline bool is_invalid (void)
  {
    adj = (fix_is_fst == true)
        ? m.get_adjacent_nogrow (fix, pos)
        : m.get_adjacent_nogrow (pos, fix)
        ;

    return (adj == invalid);
  }

  inline void step (void)
  {
    while (pos < max && is_invalid())
      ++pos;
  }

public:
  adj_it ( const mat_t & m_
         , const IDX max_
         , const ID fix_
         , const bool fix_is_fst_
         , const ID & invalid_
         ) 
  : m (m_)
  , max (max_)
  , fix (fix_)
  , fix_is_fst (fix_is_fst_)
  , invalid (invalid_)
  , pos (0)
  , adj (invalid)
  {
    step();
  }

  const bool has_more (void) const { return (pos != max) ? true : false; }
  void operator ++ (void) { ++pos; step(); }

  const IDX get_pos (void) const { return pos; }
  const ID get_adj (void) const { return adj; }

  const IDX operator * (void) const { return get_pos(); }
};

// iterator through adjacencies, returns objects given by auto_bimap
template<typename T, typename Edge, typename IDX, typename ID>
struct adj_obj_it
{
private:
  adj_it<IDX, ID> ait;
  biget<T, IDX> bigT;
  biget<Edge, IDX> bigE;
public:
  adj_obj_it ( const adjacency_matrix<IDX, ID> & m
             , const IDX max
             , const ID fix
             , const bool fix_is_fst
             , const ID invalid
             , const auto_bimap<T, IDX> & biT
             , const auto_bimap<Edge, IDX> & biE
             )
    : ait (m, max, fix, fix_is_fst, invalid)
    , bigT (biT)
    , bigE (biE)
  {}

  const bool has_more (void) const { return ait.has_more(); }
  void operator ++ (void) { ++ait; }

  const T get_T (void) const { return bigT.get (ait.get_pos()); }
  const Edge get_Edge (void) const { return bigE.get (ait.get_adj()); }

  const T operator * (void) const { return get_T(); }
};

// the net itself
template<typename Place, typename Transition, typename Edge, typename ID = unsigned long>
class net
{
private:
  auto_bimap<Place, ID> pmap;
  auto_bimap<Transition, ID> tmap;

  const ID invalid;
  auto_bimap<Edge, ID> emap;

  typedef std::map<ID, ID> map_id_t;
  typedef typename map_id_t::iterator map_id_it_t;

  map_id_t emap_in_p;
  map_id_t emap_out_p;
  map_id_t emap_in_t;
  map_id_t emap_out_t;

  typedef ID IDX;
  typedef adjacency_matrix<IDX, ID> adj_matrix;

  adj_matrix adj_pt;
  adj_matrix adj_tp;

  IDX max_place;
  IDX max_transition;
  IDX max_edge;

public:
  net (const IDX places_ = 100, const IDX transitions_ = 250)
  throw (std::bad_alloc)
    : pmap ("place", 0)
    , tmap ("transition", 0)
    , invalid (0)
    , emap ("edge name", invalid + 1)
    , emap_in_p ()
    , emap_out_p ()
    , emap_in_t ()
    , emap_out_t ()
    , adj_pt (places_, transitions_, 0)
    , adj_tp (transitions_, places_, 0)
    , max_place (0)
    , max_transition (0)
    , max_edge (0)
  {};

  ~net () {};

private:
  const ID get_place_id (const Place place) const throw (no_such)
  {
    return pmap.get_id (place);
  }

  const ID get_transition_id (const Transition transition) const throw (no_such)
  {
    return tmap.get_id (transition);
  }

  const ID get_edge_id (const Edge edge) const throw (no_such)
  {
    return emap.get_id (edge);
  }

public:
  const ID add_place (const Place place) throw (already_there)
  {
    ++max_place;

    return pmap.add (place);
  }

  const ID add_transition (const Transition transition) throw (already_there)
  {
    ++max_transition;

    return tmap.add (transition);
  }

private:
  const ID add_edge
  (const Edge edge, const IDX x, const IDX y, adj_matrix & m)
    throw (no_such, already_there)
  {
    ID & a (m.adjacent (x, y));

    if (a != invalid)
      throw already_there ("adjacency");

    const ID eid (emap.add (edge));

    a = eid;

    ++max_edge;

    return eid;
  }

public:
  const ID add_edge_place_to_transition
  (const Edge edge, const Place place, const Transition transition)
    throw (no_such, already_there)
  {
    const ID pid (get_place_id(place));
    const ID tid (get_transition_id (transition));
    const ID eid (add_edge (edge, pid, tid, adj_pt));

    emap_out_p[eid] = pid;
    emap_in_t[eid] = tid;

    return eid;
  }

  const ID add_edge_transition_to_place
  (const Edge edge, const Transition transition, const Place place)
    throw (no_such, already_there)
  {
    const ID tid (get_transition_id (transition));
    const ID pid (get_place_id (place));
    const ID eid (add_edge (edge, tid, pid, adj_tp));

    emap_in_p[eid] = pid;
    emap_out_t[eid] = tid;

    return eid;
  }

  typedef bi_it<Place, ID> place_it;
  typedef bi_it<Transition, ID> transition_it;
  typedef bi_it<Edge, ID> edge_it;

  const place_it places (void) const { return place_it (pmap); }
  const transition_it transitions (void) const { return transition_it (tmap); }
  const edge_it edges (void) const { return edge_it (emap); }

  typedef adj_obj_it<Place, Edge, IDX, ID> adj_place_it;
  typedef adj_obj_it<Place, Edge, IDX, ID> adj_transition_it;

private:
  const adj_place_it out_of_transition_by_id (const ID tid) const
  {
    return adj_place_it (adj_tp, max_place, tid, true, invalid, pmap, emap);
  }

  const adj_place_it in_to_transition_by_id (const ID tid) const
  {
    return adj_place_it (adj_pt, max_place, tid, false, invalid, pmap, emap);
  }

  const adj_transition_it out_of_place_by_id (const ID pid) const
  {
    return adj_transition_it (adj_pt, max_transition, pid, true, invalid, tmap, emap);
  }

  const adj_transition_it in_to_place_by_id (const ID pid) const
  {
    return adj_transition_it (adj_tp, max_transition, pid, false, invalid, tmap, emap);
  }

public:
  const adj_place_it out_of_transition (const Transition transition) const
  {
    return out_of_transition_by_id (get_transition_id (transition));
  }

  const adj_place_it in_to_transition (const Transition transition) const
  {
    return in_to_transition_by_id (get_transition_id (transition));
  }

  const adj_transition_it out_of_place (const Place place) const
  {
    return out_of_place_by_id (get_place_id (place));
  }

  const adj_transition_it in_to_place (const Place place) const
  {
    return in_to_place_by_id (get_place_id (place));
  }

private:
  const ID delete_edge_by_id (const ID eid)
  {
    const map_id_it_t in_p (emap_in_p.find (eid));
    const map_id_it_t in_t (emap_in_t.find (eid));
    const map_id_it_t out_p (emap_out_p.find (eid));
    const map_id_it_t out_t (emap_out_t.find (eid));

    if (out_p != emap_out_p.end())
      {
        // place -> transition

        assert (in_t != emap_in_t.end());

        const ID pid (out_p->second);
        const ID tid (in_t->second);

        assert (adj_pt.adjacent (pid, tid) == eid);

        adj_pt.adjacent (pid, tid) = invalid;

        emap_in_t.erase (in_t);
        emap_out_p.erase (out_p);
      }
    else
      {
        // transition -> place

        assert (out_t != emap_out_t.end());
        assert (in_p != emap_in_p.end());

        const ID tid (out_t->second);
        const ID pid (in_p->second);

        assert (adj_tp.adjacent (tid, pid) == eid);
        
        adj_tp.adjacent (tid, pid) = invalid;

        emap_in_p.erase (in_p);
        emap_out_t.erase (out_t);
      }

    return eid;
  }

public:
  const ID delete_edge (const Edge edge) throw (no_such)
  {
    const ID eid (get_edge_id (edge));

    emap.erase (edge);

    return delete_edge_by_id (eid);
  }

  const ID delete_place (const Place place) throw (no_such)
  {
    const ID pid (get_place_id (place));

    typedef typename net<Place,Transition,Edge,ID>::adj_transition_it trans_it;

    for ( trans_it tit (out_of_place_by_id (pid))
        ; tit.has_more()
        ; ++tit
        )
      {
        delete_edge_by_id (tit.get().second);
      }

    pmap.erase (place);

    return pid;
  }
};
