// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE locked_parent_child_relation
#include <boost/test/unit_test.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include <fhg/util/boost/test/printer/set.hpp>

#include <set>

//! \todo add test using more than one thread

namespace
{
  namespace bbm = boost::bimaps;

  typedef int id_type;

  struct locked_parent_child_relation_type
  {
    void started (id_type parent, id_type child);
    bool terminated (id_type parent, id_type child);

    boost::optional<id_type> parent (id_type child) const;
    bool contains (id_type parent) const;

    void apply (id_type parent, boost::function<void (id_type)>) const;

  private:
    mutable boost::mutex _relation_mutex;
    typedef bbm::bimap
      < bbm::unordered_multiset_of<id_type> // parent can have many childs
      , bbm::unordered_set_of<id_type>      // child can have at most one parent
      , bbm::set_of_relation<>
      > relation_type;
    relation_type _relation;
  };

  void locked_parent_child_relation_type::started
    (id_type parent, id_type child)
  {
    boost::mutex::scoped_lock const _ (_relation_mutex);

    // no child id will occur twice
    assert (_relation.right.find (child) == _relation.right.end());

    _relation.insert (relation_type::value_type (parent, child));
  }

  bool locked_parent_child_relation_type::terminated
    (id_type parent, id_type child)
  {
    boost::mutex::scoped_lock const _ (_relation_mutex);

    _relation.erase (relation_type::value_type (parent, child));

    return !contains (parent);
  }

  boost::optional<id_type> locked_parent_child_relation_type::parent
    (id_type child) const
  {
    boost::mutex::scoped_lock const _ (_relation_mutex);

    relation_type::right_map::const_iterator const pos
      (_relation.right.find (child));

    if (pos != _relation.right.end())
    {
      return pos->second;
    }

    return boost::none;
  }

  bool locked_parent_child_relation_type::contains (id_type parent) const
  {
    return _relation.left.find (parent) != _relation.left.end();
  }

  void locked_parent_child_relation_type::apply
    (id_type parent, boost::function<void (id_type)> fun) const
  {
    boost::mutex::scoped_lock const _ (_relation_mutex);

    relation_type::left_map::const_iterator pos (_relation.left.find (parent));

    while (pos != _relation.left.end() && pos->first == parent)
    {
      fun (pos->second);
      ++pos;
    }
  }
}

BOOST_AUTO_TEST_CASE (contains)
{
  locked_parent_child_relation_type relation;

  BOOST_REQUIRE (!relation.contains (0));
  BOOST_REQUIRE (!relation.contains (1));

  relation.started (0, 0);

  BOOST_REQUIRE (relation.contains (0));
  BOOST_REQUIRE (!relation.contains (1));

  relation.started (2, 2);

  BOOST_REQUIRE (relation.contains (0));
  BOOST_REQUIRE (!relation.contains (1));

  relation.started (1, 1);

  BOOST_REQUIRE (relation.contains (0));
  BOOST_REQUIRE (relation.contains (1));

  relation.terminated (1, 1);

  BOOST_REQUIRE (relation.contains (0));
  BOOST_REQUIRE (!relation.contains (1));

  relation.terminated (0, 0);

  BOOST_REQUIRE (!relation.contains (0));
  BOOST_REQUIRE (!relation.contains (1));
  BOOST_REQUIRE (relation.contains (2));
}

BOOST_AUTO_TEST_CASE (started_terminated)
{
  locked_parent_child_relation_type relation;

  relation.started (0, 0);
  BOOST_REQUIRE_EQUAL (relation.terminated (0, 0), true);

  relation.started (0, 0);
  relation.started (0, 1);

  BOOST_REQUIRE_EQUAL (relation.terminated (0, 0), false);
  BOOST_REQUIRE_EQUAL (relation.terminated (0, 1), true);

  relation.started (0, 0);
  relation.started (0, 1);
  relation.started (1, 2);
  relation.started (1, 3);

  BOOST_REQUIRE_EQUAL (relation.terminated (0, 1), false);
  BOOST_REQUIRE_EQUAL (relation.terminated (1, 3), false);
  BOOST_REQUIRE_EQUAL (relation.terminated (0, 0), true);
  BOOST_REQUIRE_EQUAL (relation.terminated (1, 2), true);
}

BOOST_AUTO_TEST_CASE (parent)
{
  locked_parent_child_relation_type relation;

  BOOST_REQUIRE (!relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (!relation.parent (2));

  relation.started (0, 0);

  BOOST_REQUIRE (relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (!relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (0), 0);

  relation.started (0, 1);

  BOOST_REQUIRE (relation.parent (0));
  BOOST_REQUIRE (relation.parent (1));
  BOOST_REQUIRE (!relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (0), 0);
  BOOST_REQUIRE_EQUAL (*relation.parent (1), 0);

  relation.started (1, 2);

  BOOST_REQUIRE (relation.parent (0));
  BOOST_REQUIRE (relation.parent (1));
  BOOST_REQUIRE (relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (0), 0);
  BOOST_REQUIRE_EQUAL (*relation.parent (1), 0);
  BOOST_REQUIRE_EQUAL (*relation.parent (2), 1);

  relation.terminated (0, 1);

  BOOST_REQUIRE (relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (0), 0);
  BOOST_REQUIRE_EQUAL (*relation.parent (2), 1);

  relation.terminated (99, 99);

  BOOST_REQUIRE (relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (0), 0);
  BOOST_REQUIRE_EQUAL (*relation.parent (2), 1);

  relation.terminated (0, 0);

  BOOST_REQUIRE (!relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (relation.parent (2));
  BOOST_REQUIRE_EQUAL (*relation.parent (2), 1);

  relation.terminated (1, 2);

  BOOST_REQUIRE (!relation.parent (0));
  BOOST_REQUIRE (!relation.parent (1));
  BOOST_REQUIRE (!relation.parent (2));
}

namespace
{
  std::set<id_type> childs;

  void fun (id_type child)
  {
    childs.insert (child);
  }
}

BOOST_AUTO_TEST_CASE (apply)
{
#define APPLY(key)                              \
  childs.clear();                               \
  expected.clear();                             \
  relation.apply (key, fun)

  locked_parent_child_relation_type relation;

  std::set<id_type> expected;

  APPLY (0);
  BOOST_REQUIRE_EQUAL (childs, expected);

  relation.started (0, 0);

  APPLY (0);
  expected.insert (0);
  BOOST_REQUIRE_EQUAL (childs, expected);

  relation.started (1, 1);

  APPLY (0);
  expected.insert (0);
  BOOST_REQUIRE_EQUAL (childs, expected);

  APPLY (1);
  expected.insert (1);
  BOOST_REQUIRE_EQUAL (childs, expected);

  relation.started (0, 2);

  APPLY (0);
  expected.insert (0);
  expected.insert (2);
  BOOST_REQUIRE_EQUAL (childs, expected);

  relation.terminated (0, 0);

  APPLY (0);
  expected.insert (2);
  BOOST_REQUIRE_EQUAL (childs, expected);

#undef APPLY
}
