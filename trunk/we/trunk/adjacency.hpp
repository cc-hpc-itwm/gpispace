// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _ADJACENCY_HPP
#define _ADJACENCY_HPP

#include <vector>
#include <stdexcept>

namespace adjacency
{
  template<typename L, typename R>
  struct const_it
  {
  private:
    typedef std::pair<L,R> pair_t;
    typedef std::vector<pair_t> vec_t;
    typedef typename vec_t::const_iterator it_t;

    it_t pos;
    const it_t end;
  public:
    const_it (const it_t & _pos, const it_t & _end) : pos (_pos), end (_end) {}

    const bool has_more (void) const { return (pos != end) ? true : false; }
    void operator ++ (void) { ++pos; }

    const L & operator * (void) const { return pos->first; }
    const R & operator () (void) const { return pos->second; }
  };

  template<typename ROW, typename COL, typename ADJ>
  class table
  {
  private:
    const ADJ invalid;

    typedef std::pair<ROW,ADJ> row_adj_t;
    typedef std::pair<COL,ADJ> col_adj_t;

    typedef std::vector<row_adj_t> row_adj_vec_t;
    typedef std::vector<col_adj_t> col_adj_vec_t;
    
    typedef std::vector<col_adj_vec_t> row_tab_t;
    typedef std::vector<row_adj_vec_t> col_tab_t;

    row_tab_t row_tab; // size >= 1
    col_tab_t col_tab; // size >= 1

    template<typename T, typename MAT>
    void adjust_size (const T & x, MAT & mat)
    {
      typedef typename MAT::size_type sz_t;

      sz_t sz (mat.size()); // size >= 1

      while (x >= sz)
        sz <<= 1;

      mat.resize (sz); // size >= 1
    }

  public:
    const const_it<COL,ADJ> row_const_it (const ROW & r) const
    {
      // size >= 1
      return (r < row_tab.size())
        ? const_it<COL,ADJ> (row_tab[r].begin(), row_tab[r].end())
        : const_it<COL,ADJ> (row_tab[0].begin(), row_tab[0].begin()) // empty
        ;
    }

    const const_it<ROW,ADJ> col_const_it (const COL & c) const
    {
       // size >= 1
      return (c < col_tab.size())
        ? const_it<ROW,ADJ> (col_tab[c].begin(), col_tab[c].end())
        : const_it<ROW,ADJ> (col_tab[0].begin(), col_tab[0].begin()) // empty
        ;
    }

    table (const ADJ & _invalid, const ROW & r = 1, const COL & c = 1)
      : invalid (_invalid)
      , row_tab (std::max (static_cast<ROW>(1), r)) // size >= 1
      , col_tab (std::max (static_cast<COL>(1), c)) // size >= 1
    {}

    const ADJ get_adjacent (const ROW & r, const COL & c) const
    {
      ADJ v (invalid);

      if (r < row_tab.size())
        for ( typename col_adj_vec_t::const_iterator it (row_tab[r].begin())
            ; it != row_tab[r].end()
            ; ++it
            )
          if (it->first == c)
            {
              v = it->second;
              break;
            }

      return v;
    }

    void clear_adjacent (const ROW & r, const COL & c)
    {
      if (r < row_tab.size())
        for ( typename col_adj_vec_t::iterator it (row_tab[r].begin())
            ; it != row_tab[r].end()
            ; ++it
            )
          if (it->first == c)
            {
              row_tab[r].erase (it);
              break;
            }

      if (c < col_tab.size())
        for ( typename row_adj_vec_t::iterator it (col_tab[c].begin())
            ; it != col_tab[c].end()
            ; ++it
            )
          if (it->first == r)
            {
              col_tab[c].erase (it);
              break;
            }
    }

  public:
    void set_adjacent (const ROW & r, const COL & c, const ADJ & v)
    {
      adjust_size<ROW> (r, row_tab);
      adjust_size<COL> (c, col_tab);

      row_tab[r].push_back (col_adj_t (c, v));
      col_tab[c].push_back (row_adj_t (r, v));
    }
  };
} // namespace adjacency

#endif // _ADJACENCY_HPP
