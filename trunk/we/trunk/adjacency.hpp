// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _ADJACENCY_HPP
#define _ADJACENCY_HPP

#include <vector>
#include <stdexcept>

#include <ostream>
#include <iomanip>

namespace adjacency
{
  template<typename IDX, typename ADJ>
  class table
  {
  private:
    IDX row;
    IDX col;

    const ADJ invalid;

    typedef std::pair<IDX,ADJ> adj_t;
    typedef std::vector<adj_t> adj_vec_t;
    typedef std::vector<adj_vec_t> table_t;

  public:
    typedef typename adj_vec_t::const_iterator const_it;
    typedef typename std::pair<const_it,const_it> range_const_it;

  private:
    // store table as well as transposed table to allow fast iteration
    // row wise and column wise

    table_t tableN;
    table_t tableT;

    const ADJ gen_get (const IDX & x, const IDX & y, const table_t & t) const
    {
      ADJ v (invalid);

      for ( typename adj_vec_t::const_iterator it (t[x].begin())
          ; it != t[x].end()
          ; ++it
          )
        if (it->first == y)
          {
            v = it->second;
            break;
          }

      return v;
    }

    void gen_clear (const IDX & x, const IDX & y, table_t & t)
    {
      for ( typename adj_vec_t::iterator it (t[x].begin())
          ; it != t[x].end()
          ; ++it
          )
        if (it->first == y)
          {
            t[x].erase (it);
            break;
          }
    }

  public:
    table (const IDX & r, const IDX & c, const ADJ & x)
      : row (r)
      , col (c)
      , invalid (x)
      , tableN (row)
      , tableT (col)
    {}

    const range_const_it get_range_const_it (const IDX & x) const
    {
      return range_const_it (tableN[x].begin(),tableN[x].end());
    }

    const range_const_it get_range_const_itT (const IDX & x) const
    {
      return range_const_it (tableT[x].begin(),tableT[x].end());
    }

    const ADJ get_adjacent (const IDX & r, const IDX & c) const
      throw (std::out_of_range)
    {
      if (r > row - 1)
        throw std::out_of_range("adjacency::table.get_adjacent: row");

      return gen_get (r, c, tableN);
    }

    const ADJ get_adjacentT (const IDX & r, const IDX & c) const
      throw (std::out_of_range)
    {
      if (c > col - 1)
        throw std::out_of_range("adjacency::table.get_adjacentT: col");

      return gen_get (c, r, tableT);
    }

    void clear_adjacent (const IDX & r, const IDX & c)
      throw (std::out_of_range)
    {
      if (r > row - 1)
        throw std::out_of_range("adjacency::table.clear_adjacent: row");

      if (c > col - 1)
        throw std::out_of_range("adjacency::table.clear_adjacent: col");

      gen_clear (r, c, tableN);
      gen_clear (c, r, tableT);
    }

    void set_adjacent (const IDX & r, const IDX & c, const IDX & x)
    {
      if (r > row - 1)
        {
          row = std::max (r + 1, 2 * row);

          tableN.resize (row);
        }

      if (c > col - 1)
        {
          col = std::max (c + 1, 2 * col);

          tableT.resize (col);
        }

      tableN[r].push_back (adj_t (c, x));
      tableT[c].push_back (adj_t (r, x));
    }

    template<typename I, typename A>
    friend std::ostream & operator << (std::ostream &, const table<I,A> &);
  };

  template<typename IDX, typename ADJ>
  std::ostream & operator << (std::ostream & s, const table<IDX,ADJ> & m)
  {
    s << "adjacency::table:";
    s << " (row = " << m.row << ", col = " << m.col << ")" << std::endl;

    const unsigned int w (3);

    s << std::setw(w) << "";

    for (IDX c (0); c < m.col; ++c)
      s << std::setw (w) << c;

    s << std::endl;

    for (IDX r (0); r < m.row; ++r)
      {
        s << std::setw(w) << r;

        for (IDX c (0); c < m.col; ++c)
          {
            const ADJ adj (m.get_adjacent (r, c));

            s << std::setw(w);

            if (adj == m.invalid)
              s << ".";
            else
              s << adj;
          }

        s << std::endl;
      }

    s << "  transposed:" << std::endl << std::setw(w) << "";

    for (IDX r (0); r < m.row; ++r)
      s << std::setw (w) << r;

    s << std::endl;

    for (IDX c (0); c < m.col; ++c)
      {
        s << std::setw(w) << c;

        for (IDX r (0); r < m.row; ++r)
          {
            const ADJ adj (m.get_adjacentT (r, c));

            s << std::setw(w);

            if (adj == m.invalid)
              s << ".";
            else
              s << adj;
          }

        s << std::endl;
      }

    return s;
  };

  // iterate through adjacencies
  template<typename IDX, typename ADJ>
  struct const_it
  {
  private:
    typename table<IDX,ADJ>::range_const_it range_const_it;
    typename table<IDX,ADJ>::const_it & pos;
    const typename table<IDX,ADJ>::const_it & end;

  public:
    const_it (const table<IDX,ADJ> & m, const ADJ & x, const bool & fix_is_fst)
      : range_const_it ( fix_is_fst 
                       ? m.get_range_const_it (x) 
                       : m.get_range_const_itT (x)
                       )
      , pos (range_const_it.first)
      , end (range_const_it.second)
    {}

    const bool has_more (void) const { return (pos != end) ? true : false; }
    void operator ++ (void) { ++pos; }

    const ADJ & edge_id (void) const { return pos->second; }
    const IDX & obj_id (void) const { return pos->first; }

    const ADJ & operator () (void) const { return edge_id(); }
    const IDX & operator * (void) const { return obj_id(); }
  };
} // namespace adjacency

#endif // _ADJACENCY_HPP
