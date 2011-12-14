// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_ADJACENCY_HPP
#define _CONTAINER_ADJACENCY_HPP

#include <we/util/it.hpp>

#include <stdexcept>

#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/optional.hpp>

namespace adjacency
{
  template<typename L,typename R>
  struct IT
  {
  public:
    typedef std::pair<L,R> pair_t;
    typedef std::vector<pair_t> vec_t;
    typedef typename vec_t::const_iterator type;
  };

  template<typename L, typename R>
  struct const_it : public it::it<typename IT<L,R>::type>
  {
  private:
    typedef typename IT<L,R>::type it_t;

  public:
    const_it (const it_t & _pos, const it_t & _end)
      : const_it::super(_pos,_end) {}

    const L & operator * (void) const { return const_it::super::pos->first; }
    const R & operator () (void) const { return const_it::super::pos->second; }
  };

  template<typename ROW, typename COL, typename ADJ>
  class table
  {
  public:
    table (const ADJ &, const ROW &, const COL &);
    const const_it<COL,ADJ> row_const_it (const ROW &) const;
    const const_it<ROW,ADJ> col_const_it (const COL &) const;
    const ADJ get_adjacent (const ROW &, const COL &) const;
    void clear_adjacent (const ROW & r, const COL & c);
    void set_adjacent (const ROW & r, const COL & c, const ADJ & v);

  private:
    ADJ invalid;

    typedef std::pair<ROW,ADJ> row_adj_t;
    typedef std::pair<COL,ADJ> col_adj_t;

    typedef std::vector<row_adj_t> row_adj_vec_t;
    typedef std::vector<col_adj_t> col_adj_vec_t;

    typedef std::vector<col_adj_vec_t> row_tab_t;
    typedef std::vector<row_adj_vec_t> col_tab_t;

    row_tab_t row_tab; // size >= 1
    col_tab_t col_tab; // size >= 1

    template<typename T, typename MAT> void adjust_size (const T &, MAT &);

    template<typename IT, typename M, typename A, typename B>
    const boost::optional<IT> find (M &, const A &, const B &) const;

    friend class boost::serialization::access;
    template<typename Archive> void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(invalid);
      ar & BOOST_SERIALIZATION_NVP(row_tab);
      ar & BOOST_SERIALIZATION_NVP(col_tab);
    }
  };
} // namespace adjacency

#ifndef WE_PRECOMPILE
#include <we/container/adjacency.impl.hpp>
#endif

#endif
