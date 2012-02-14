// multi relation, objects as well as individual relations are allowed
// to occur more than once, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_MULTIREL_HPP
#define _CONTAINER_MULTIREL_HPP

#include <we/util/it.hpp>

#include <stdexcept>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>

#include <boost/serialization/nvp.hpp>

namespace multirel
{
  namespace exception
  {
    class delete_non_existing_object : public std::runtime_error
    {
    public:
      explicit delete_non_existing_object (const std::string & msg = "")
        : std::runtime_error ("multirel: delete non-existing object " + msg) {}
    };
  }

  template<typename L, typename R>
  class traits
  {
  public:
    typedef typename boost::bimap
    < boost::bimaps::unordered_multiset_of<L>
    , boost::bimaps::unordered_multiset_of<R>
    , boost::bimaps::unordered_multiset_of_relation<>
    > container_t;

    typedef typename container_t::value_type val_t;
    typedef typename container_t::iterator it_t;
    typedef std::pair<it_t, it_t> range_it;
    typedef typename container_t::const_iterator const_it;

    typedef typename container_t::right_map::const_iterator right_const_it;
    typedef typename container_t::left_map::const_iterator left_const_it;

    typedef typename container_t::right_map::iterator right_it;
    typedef typename container_t::left_map::iterator left_it;
    typedef std::pair<right_it,right_it> right_range_it;
    typedef std::pair<left_it,left_it> left_range_it;
  };

  template<typename IT, typename T>
  struct lr_const_it : public it::it<IT>
  {
  public:
    explicit lr_const_it (const std::pair<IT, IT> & its)
      : it::it<IT> (its.first, its.second)
    {}

    const T & operator * (void) const { return it::it<IT>::pos->second; }
  };

  template<typename L, typename R>
  struct right_const_it
    : public lr_const_it<typename traits<L,R>::right_const_it, L>
  {
  private:
    typedef typename traits<L,R>::right_const_it it_t;

  public:
    explicit right_const_it (const std::pair<it_t, it_t> & its)
      : lr_const_it<it_t, L> (its)
    {}
  };

  template<typename L, typename R>
  struct left_const_it
    : public lr_const_it<typename traits<L,R>::left_const_it, R>
  {
  private:
    typedef typename traits<L,R>::left_const_it it_t;

  public:
    explicit left_const_it (const std::pair<it_t, it_t> & its)
      : lr_const_it<it_t, R> (its)
    {}
  };

  template<typename L, typename R>
  class multirel
  {
  private:
    typename traits<L,R>::container_t container;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(container);
    }

  public:
    bool add (const L & l, const R & r)
    {
      return container.insert(typename traits<L,R>::val_t (l, r)).second;
    }

    std::size_t delete_one (const L & l, const R & r)
    {
      typename traits<L,R>::range_it
        range_it (container.equal_range (typename traits<L,R>::val_t (l, r)));

      const std::size_t dist (std::distance (range_it.first, range_it.second));

      if (dist > 0)
        container.erase (range_it.first);
      else
        throw exception::delete_non_existing_object ("in delete_one");

      return dist;
    }

    std::size_t delete_all (const L & l, const R & r)
    {
      typename traits<L,R>::range_it
        range_it (container.equal_range (typename traits<L,R>::val_t (l, r)));

      const std::size_t dist (std::distance (range_it.first, range_it.second));

      if (dist == 0)
        throw exception::delete_non_existing_object ("in delete_all");

      container.erase (range_it.first, range_it.second);

      return dist;
    }

    std::size_t delete_right (const R & r)
    {
      typename traits<L,R>::right_range_it it (container.right.equal_range (r));

      const std::size_t dist (std::distance (it.first, it.second));

      container.right.erase (it.first, it.second);

      return dist;
    }

    std::size_t delete_left (const L & l)
    {
      typename traits<L,R>::left_range_it it (container.left.equal_range (l));

      const std::size_t dist (std::distance (it.first, it.second));

      container.left.erase (it.first, it.second);

      return dist;
    }

    right_const_it<L,R> left_of (const R & r) const
    {
      return right_const_it<L,R> (container.right.equal_range (r));
    }

    left_const_it<L,R> right_of (const L & l) const
    {
      return left_const_it<L,R> (container.left.equal_range (l));
    }

    typename traits<L,R>::const_it begin (void) const
    {
      return container.begin();
    }

    typename traits<L,R>::const_it end (void) const
    {
      return container.end();
    }

    bool contains_left (const L & l) const
    {
      return (container.left.find (l) != container.left.end());
    }

    bool contains_right (const R & r) const
    {
      return (container.right.find (r) != container.right.end());
    }
  };
} // namespace multirel

#endif
