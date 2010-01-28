// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _ADJACENCY_HPP
#define _ADJACENCY_HPP

#include <util.hpp>

#include <stdexcept>

#include <vector>

namespace adjacency
{
  namespace exception
  {
    class not_found : public std::runtime_error
    {
    public:
      not_found() : std::runtime_error("not_found") {}
      ~not_found() throw() {}
    };
  }

  template<typename L,typename R>
  struct IT
  {
  public:
    typedef std::pair<L,R> pair_t;
    typedef std::vector<pair_t> vec_t;
    typedef typename vec_t::const_iterator type;
  };

  template<typename L, typename R>
  struct const_it : public util::it<typename IT<L,R>::type>
  {
  private:
    typedef typename IT<L,R>::type it_t;
    typedef typename util::it<it_t> super;
  public:
    const_it (const it_t & _pos, const it_t & _end) : super(_pos,_end) {}

    const L & operator * (void) const { return super::pos->first; }
    const R & operator () (void) const { return super::pos->second; }
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
      typename MAT::size_type sz (mat.size()); // size >= 1

      while (x >= sz)
        sz <<= 1;

      mat.resize (sz); // size >= 1
    }

    template<typename IT, typename M, typename A, typename B>
    const IT find (M & m, const A & a, const B & b) const
      throw (exception::not_found)
    {
      if (a < m.size())
        for (IT it (m[a].begin()); it != m[a].end(); ++it)
          if (it->first == b)
            return it;

      throw exception::not_found();
    }

  public:
    table (const ADJ & _invalid, const ROW & r = 1, const COL & c = 1)
      : invalid (_invalid)
      , row_tab (std::max (static_cast<ROW>(1), r)) // size >= 1
      , col_tab (std::max (static_cast<COL>(1), c)) // size >= 1
    {}

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

    const ADJ get_adjacent (const ROW & r, const COL & c) const
    {
      try
        {
          typedef typename col_adj_vec_t::const_iterator it_t;

          return find<it_t,const row_tab_t,ROW,COL>(row_tab, r, c)->second;
        }
      catch (exception::not_found)
        {
          return invalid;
        }
    }

    void clear_adjacent (const ROW & r, const COL & c)
    {
      try
        {
          typedef typename col_adj_vec_t::iterator it_t;

          const it_t it (find<it_t,row_tab_t,ROW,COL>(row_tab, r, c));

          row_tab[r].erase (it);
        }
      catch (exception::not_found) {} // do nothing

      try
        {
          typedef typename row_adj_vec_t::iterator it_t;

          const it_t it (find<it_t,col_tab_t,COL,ROW>(col_tab, c, r));

          col_tab[c].erase (it);
        }
      catch (exception::not_found) {} // do nothing
    }

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
