/* -*- mode: c++ -*- */

#include "adjacency.hpp"

#ifndef _WE_CONTAINER_ADJACENCY_IPP
#define _WE_CONTAINER_ADJACENCY_IPP 1

namespace adjacency
{
  template<typename ROW, typename COL, typename ADJ>
  table<ROW,COL,ADJ>::table ( const ADJ & _invalid
                            , const ROW & r
                            , const COL & c
                            )
    : invalid (_invalid)
    , row_tab ()
    , col_tab ()
  {}

  template<typename ROW, typename COL, typename ADJ>
  const const_it<COL,ADJ>
  table<ROW,COL,ADJ>::row_const_it (const ROW & r) const
  {
    typename row_tab_t::const_iterator pos (row_tab.find (r));

    if (pos != row_tab.end())
      {
        return const_it<COL,ADJ> (pos->second.begin(), pos->second.end());
      }
    else
      {
        const col_adj_tab_t v;

        return const_it<COL,ADJ> (v.end(), v.end());
      }
  }

  template<typename ROW, typename COL, typename ADJ>
  const const_it<ROW,ADJ>
  table<ROW,COL,ADJ>::col_const_it (const COL & c) const
  {
    typename col_tab_t::const_iterator pos (col_tab.find (c));

    if (pos != col_tab.end())
      {
        return const_it<ROW,ADJ> (pos->second.begin(), pos->second.end());
      }
    else
      {
        const row_adj_tab_t v;

        return const_it<ROW,ADJ> (v.end(), v.end());
      }
  }

  template<typename ROW, typename COL, typename ADJ>
  const ADJ
  table<ROW,COL,ADJ>::get_adjacent (const ROW & r, const COL &c) const
  {
    typename row_tab_t::const_iterator pos (row_tab.find (r));

    if (pos != row_tab.end())
      {
        typename col_adj_tab_t::const_iterator it (pos->second.find (c));

        if (it != pos->second.end())
          {
            return it->second;
          }
      }

    return invalid;
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::clear_adjacent (const ROW & r, const COL & c)
  {
    {
      typename row_tab_t::iterator pos (row_tab.find (r));

      if (pos != row_tab.end())
        {
          pos->second.erase (c);
        }
    }

    {
      typename col_tab_t::iterator pos (col_tab.find (c));

      if (pos != col_tab.end())
        {
          pos->second.erase (r);
        }
    }
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::set_adjacent ( const ROW & r
                                        , const COL & c
                                        , const ADJ & v
                                        )
  {
    row_tab[r][c] = v;
    col_tab[c][r] = v;
  }
} // namespace adjacency

#endif
