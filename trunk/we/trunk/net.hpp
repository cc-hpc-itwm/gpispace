
// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#include <boost/bimap.hpp>
#include <vector>

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

    I i (num++);

    bimap.insert (val_t (x, i));

    return num;
  }
};

template<typename IDX = unsigned int, typename ID = unsigned long>
class adjacency_matrix
{
private:
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

template<typename Place, typename Transition, typename Edge, typename ID = unsigned long>
class net
{
private:
  auto_bimap<Place, ID> pmap;
  auto_bimap<Transition, ID> tmap;

  const ID invalid;
  auto_bimap<Edge, ID> emap;

  typedef unsigned int IDX;

  adjacency_matrix<IDX, ID> adj_pt;
  adjacency_matrix<IDX, ID> adj_tp;

  IDX places;
  IDX transitions;

public:
  net (const IDX num_places = 100, const IDX num_transitions = 250)
  throw (std::bad_alloc)
    : pmap ("place", 0)
    , tmap ("transition", 0)
    , invalid (0)
    , emap ("edge name", invalid + 1)
    , adj_pt (num_places, num_transitions, 0)
    , adj_tp (num_transitions, num_places, 0)
    , places (0)
    , transitions (0)
  {};

  ~net () {};

private:
  const ID get_place_id (const Place place) const throw (no_such)
  {
    return pmap.get_id (place);
  }

  const Place get_place (const ID i) const throw (no_such)
  {
    return pmap.get_elem (i);
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
  ID add_place (const Place place) throw (already_there)
  {
    ++places;

    return pmap.add (place);
  }

  ID add_transition (const Transition transition) throw (already_there)
  {
    ++transitions;

    return tmap.add (transition);
  }

  ID add_edge_place_to_transition
  (const Edge edge, const Place place, const Transition transition)
    throw (no_such, already_there)
  {
    const ID pid (get_place_id (place));
    const ID tid (get_transition_id (transition));

    ID & a (adj_pt.adjacent(pid, tid));
    
    if (a != invalid)
      throw already_there ("adjacency");

    const ID eid (emap.add (edge));
    
    a = eid;

    return eid;
  }

  ID add_edge_transition_to_place
  (const Edge edge, const Transition transition, const Place place)
    throw (no_such, already_there)
  {
    const ID tid (get_transition_id (transition));
    const ID pid (get_place_id (place));

    ID & a (adj_tp.adjacent(tid, pid));

    if (a != invalid)
      throw already_there ("adjacency");
    
    const ID eid (emap.add (edge));
    
    a = eid;

    return eid;
  }

//   const std::vector<Place> out (const Transition transition) const
//     throw (no_such)
//   {
//     std::vector<Place> ret;

//     const ID tid (get_transition_id (transition));

//     for (ID pid(0); pid < places; ++pid)
//       {
//         ID & a (adj_tp.adjacent(tid, pid));

//         if (a != invalid)
//           ret.push_back (get_place(pid));
//       }

//     return ret;
//   }
};
