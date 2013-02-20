// adjacency table, stored as adjacency list, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_ADJACENCY_HPP
#define _CONTAINER_ADJACENCY_HPP

#include <we/container/exception.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

namespace adjacency
{
  template<typename ROW, typename COL, typename ADJ>
  class table
  {
  public:
    const boost::optional<ADJ> get_adjacent (const ROW&, const COL&) const;
    const ADJ get_adjacent (const ROW&, const COL&, const std::string&) const;
    bool is_adjacent (const ROW&, const COL&) const;
    void clear_adjacent (const ROW&, const COL&);
    void set_adjacent (const ROW&, const COL&, const ADJ&);
    void set_adjacent (const ROW&, const COL&, const ADJ&, const std::string&);
    boost::unordered_set<ADJ> adjacencies() const;

    const boost::unordered_map<COL,ADJ>&
    col_adj_tab (const ROW& r) const
    {
      typename row_tab_t::const_iterator pos (row_tab.find (r));

      if (pos != row_tab.end())
        {
          return pos->second;
        }

      static boost::unordered_map<COL,ADJ> col_adj_tab_empty;

      return col_adj_tab_empty;
    }

    const boost::unordered_map<ROW,ADJ>&
    row_adj_tab (const COL& c) const
    {
      typename col_tab_t::const_iterator pos (col_tab.find (c));

      if (pos != col_tab.end())
        {
          return pos->second;
        }

      static boost::unordered_map<ROW,ADJ> row_adj_tab_empty;

      return row_adj_tab_empty;
    }

  private:
    typedef boost::unordered_map<ROW,ADJ> row_adj_tab_t;
    typedef boost::unordered_map<COL,ADJ> col_adj_tab_t;

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

  template<typename ROW, typename COL, typename ADJ>
  const boost::optional<ADJ>
  table<ROW,COL,ADJ>::get_adjacent (const ROW& r, const COL&c) const
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

    return boost::none;
  }

  template<typename ROW, typename COL, typename ADJ>
  const ADJ
  table<ROW,COL,ADJ>::get_adjacent ( const ROW& r
                                   , const COL& c
                                   , const std::string& msg
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

    throw we::container::exception::no_such ("get_adjacent: " + msg);
  }

  template<typename ROW, typename COL, typename ADJ>
  bool table<ROW,COL,ADJ>::is_adjacent (const ROW& r, const COL&c) const
  {
    typename row_tab_t::const_iterator pos (row_tab.find (r));

    return (pos != row_tab.end())
      ? (pos->second.find (c) != pos->second.end())
      : false
      ;
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::clear_adjacent (const ROW& r, const COL& c)
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
  void table<ROW,COL,ADJ>::set_adjacent ( const ROW& r
                                        , const COL& c
                                        , const ADJ& v
                                        )
  {
    row_tab[r][c] = v;
    col_tab[c][r] = v;
  }

  template<typename ROW, typename COL, typename ADJ>
  void table<ROW,COL,ADJ>::set_adjacent ( const ROW& r
                                        , const COL& c
                                        , const ADJ& v
                                        , const std::string& msg
                                        )
  {
    if (is_adjacent (r, c))
      {
        throw we::container::exception::already_there ("set_adjacent: " + msg);
      }

    set_adjacent (r, c, v);
  }

  //! \todo Implement more efficient if necessary
  template<typename ROW, typename COL, typename ADJ>
  boost::unordered_set<ADJ> table<ROW,COL,ADJ>::adjacencies() const
  {
    boost::unordered_set<ADJ> s;

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
