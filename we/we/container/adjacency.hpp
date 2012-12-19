// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_ADJACENCY_HPP
#define _CONTAINER_ADJACENCY_HPP

#include <we/util/it.hpp>

#include <stdexcept>

#include <boost/serialization/nvp.hpp>

#include <boost/unordered_map.hpp>

namespace adjacency
{
  template<typename L,typename R>
  struct IT
  {
  public:
    typedef boost::unordered_map<L,R> vec_t;
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

    typedef boost::unordered_map<ROW,ADJ> row_adj_tab_t;
    typedef boost::unordered_map<COL,ADJ> col_adj_tab_t;

    typedef boost::unordered_map<ROW,col_adj_tab_t> row_tab_t;
    typedef boost::unordered_map<COL,row_adj_tab_t> col_tab_t;

    row_tab_t row_tab;
    col_tab_t col_tab;

    friend class boost::serialization::access;
    template<typename Archive> void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(invalid);
      ar & BOOST_SERIALIZATION_NVP(row_tab);
      ar & BOOST_SERIALIZATION_NVP(col_tab);
    }
  };
} // namespace adjacency

#include <we/container/adjacency.impl.hpp>

#endif
