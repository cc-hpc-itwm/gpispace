// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_ADJACENCY_HPP
#define _CONTAINER_ADJACENCY_HPP

#include <we/container/exception.hpp>
#include <we/type/connection.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace adjacency
{
  template<typename ROW, typename COL>
  class table
  {
  public:
    const petri_net::connection_t& get_adjacent (const ROW&, const COL&) const;
    void clear_adjacent (const ROW&, const COL&);
    void set_adjacent (const ROW&, const COL&, const petri_net::connection_t&);
    boost::unordered_set<petri_net::connection_t> adjacencies() const;

    const boost::unordered_map<COL,petri_net::connection_t>&
    col_adj_tab (const ROW& r) const
    {
      typename row_tab_t::const_iterator pos (row_tab.find (r));

      if (pos != row_tab.end())
        {
          return pos->second;
        }

      static boost::unordered_map<COL,petri_net::connection_t> col_adj_tab_empty;

      return col_adj_tab_empty;
    }

    const boost::unordered_map<ROW,petri_net::connection_t>&
    row_adj_tab (const COL& c) const
    {
      typename col_tab_t::const_iterator pos (col_tab.find (c));

      if (pos != col_tab.end())
        {
          return pos->second;
        }

      static boost::unordered_map<ROW,petri_net::connection_t> row_adj_tab_empty;

      return row_adj_tab_empty;
    }

  private:
    typedef boost::unordered_map<ROW,petri_net::connection_t> row_adj_tab_t;
    typedef boost::unordered_map<COL,petri_net::connection_t> col_adj_tab_t;

    typedef boost::unordered_map<ROW,col_adj_tab_t> row_tab_t;
    typedef boost::unordered_map<COL,row_adj_tab_t> col_tab_t;

    row_tab_t row_tab;
    col_tab_t col_tab;

    friend class boost::serialization::access;
    template<typename Archive> void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(row_tab);
      ar & BOOST_SERIALIZATION_NVP(col_tab);
    }
  };

  template<typename ROW, typename COL>
  const petri_net::connection_t&
  table<ROW,COL>::get_adjacent ( const ROW& r
                               , const COL& c
                               ) const
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

    throw we::container::exception::no_such
      ((boost::format ("get_adjacent: %1% <-> %2%") % r % c).str());
  }

  template<typename ROW, typename COL>
  void table<ROW,COL>::clear_adjacent (const ROW& r, const COL& c)
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

  template<typename ROW, typename COL>
  void table<ROW,COL>::set_adjacent ( const ROW& r
                                    , const COL& c
                                    , const petri_net::connection_t& v
                                    )
  {
    typename row_tab_t::const_iterator const pos (row_tab.find (r));

    if (pos != row_tab.end() && pos->second.find (c) != pos->second.end())
      {
        throw we::container::exception::already_there
          ( ( boost::format ("set_adjacent: %1% <-|%3%|-> %2%") % r % c % "v"
            ).str()
          );
      }

    row_tab[r][c] = col_tab[c][r] = v;
  }

  //! \todo Implement more efficient if necessary
  template<typename ROW, typename COL>
  boost::unordered_set<petri_net::connection_t>
  table<ROW,COL>::adjacencies() const
  {
    boost::unordered_set<petri_net::connection_t> s;

    BOOST_FOREACH (const typename row_tab_t::value_type& tab, row_tab)
      {
        BOOST_FOREACH ( const typename col_adj_tab_t::value_type& pos
                      , tab.second
                      )
          {
            s.insert (pos.second);
          }
      }

    return s;
  }
} // namespace adjacency

#endif
