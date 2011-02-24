/* -*- mode: c++ -*- */

#include "adjacency.hpp"

namespace adjacency
{
  template<typename ROW, typename COL, typename ADJ>
  table<ROW,COL,ADJ>::table ( const ADJ & _invalid
                            , const ROW & r = 1
                            , const COL & c = 1
                            )
    : invalid (_invalid)
    , row_tab (std::max (static_cast<ROW>(1), r)) // size >= 1
    , col_tab (std::max (static_cast<COL>(1), c)) // size >= 1
  {}

  template<typename ROW, typename COL, typename ADJ>
  const const_it<COL,ADJ>
  table<ROW,COL,ADJ>::row_const_it (const ROW & r) const
  {
    // size >= 1
    return (r < row_tab.size())
      ? const_it<COL,ADJ> (row_tab[r].begin(), row_tab[r].end())
      : const_it<COL,ADJ> (row_tab[0].begin(), row_tab[0].begin()) // empty
      ;
  }

  template<typename ROW, typename COL, typename ADJ>
  const const_it<ROW,ADJ>
  table<ROW,COL,ADJ>::col_const_it (const COL & c) const
  {
    // size >= 1
    return (c < col_tab.size())
      ? const_it<ROW,ADJ> (col_tab[c].begin(), col_tab[c].end())
      : const_it<ROW,ADJ> (col_tab[0].begin(), col_tab[0].begin()) // empty
      ;
  }

  template<typename ROW, typename COL, typename ADJ>
  const ADJ
  table<ROW,COL,ADJ>::get_adjacent (const ROW & r, const COL &c) const
  {
    typedef typename col_adj_vec_t::const_iterator it_t;

    const boost::optional<it_t> mit
      (find<it_t,const row_tab_t,ROW,COL>(row_tab, r, c));

    return mit ? (*mit)->second : invalid;
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::clear_adjacent (const ROW & r, const COL & c)
  {
    {
      typedef typename col_adj_vec_t::iterator it_t;

      const boost::optional<it_t> mit
        (find<it_t,row_tab_t,ROW,COL>(row_tab, r, c));

      if (mit)
        {
          row_tab[r].erase (*mit);
        }
    }

    {
      typedef typename row_adj_vec_t::iterator it_t;

      const boost::optional<it_t> mit
        (find<it_t,col_tab_t,COL,ROW>(col_tab, c, r));

      if (mit)
        {
          col_tab[c].erase (*mit);
        }
    }
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::set_adjacent ( const ROW & r
                                        , const COL & c
                                        , const ADJ & v
                                        )
  {
    adjust_size<ROW> (r, row_tab);
    adjust_size<COL> (c, col_tab);

    row_tab[r].push_back (col_adj_t (c, v));
    col_tab[c].push_back (row_adj_t (r, v));
  }

  template<typename ROW, typename COL, typename ADJ>
  template<typename T, typename MAT>
  void table<ROW, COL, ADJ>::adjust_size (const T & x, MAT & mat)
  {
    typename MAT::size_type sz (mat.size()); // size >= 1

    while (x >= sz)
      {
        sz <<= 1;
      }

    mat.resize (sz); // size >= 1
  }

  template<typename ROW, typename COL, typename ADJ>
  template<typename IT, typename M, typename A, typename B>
  const boost::optional<IT>
  table<ROW,COL,ADJ>::find (M & m, const A & a, const B & b) const
  {
    if (a < m.size())
      {
        for (IT it (m[a].begin()); it != m[a].end(); ++it)
          {
            if (it->first == b)
              {
                return it;
              }
          }
      }

    return boost::none;
  }
} // namespace adjacency
