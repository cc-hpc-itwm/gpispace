#include <boost/test/unit_test.hpp>

#include <gspc/util/callable_signature.hpp>
#include <gspc/testing/require_compiletime.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>

namespace
{
  struct that
  {
    int fun (int, float)
    {
      ++_value;
      return _value;
    }

    int cfun() const
    {
      return _value;
    }

    int _value;
  };
}

BOOST_AUTO_TEST_CASE (returns_functor_callable_with_just_function_arguments)
{
  GSPC_TESTING_COMPILETIME_REQUIRE
    ( gspc::util::is_callable
        < decltype ( gspc::util::bind_this
                       (std::declval<that*>(), &that::fun)
                   )
        , int (int, float)
        >{}
    );
}

BOOST_AUTO_TEST_CASE (binds_this_as_a_reference)
{
  that t {0};
  auto fun (gspc::util::bind_this (&t, &that::fun));
  BOOST_REQUIRE_EQUAL (fun (int(), float()), 1);

  t._value = 120;
  BOOST_REQUIRE_EQUAL (fun (int(), float()), 121);
}

BOOST_AUTO_TEST_CASE (const_this)
{
  that const t {120};
  auto cfun (gspc::util::bind_this (&t, &that::cfun));
  BOOST_REQUIRE_EQUAL (cfun(), 120);
}

BOOST_AUTO_TEST_CASE (const_memfn)
{
  {
    that const t {120};
    auto cfun (gspc::util::bind_this (&t, &that::cfun));
    BOOST_REQUIRE_EQUAL (cfun(), 120);
  }
  {
    that t {120};
    auto cfun (gspc::util::bind_this (&t, &that::cfun));
    BOOST_REQUIRE_EQUAL (cfun(), 120);
    t._value = 1;
    BOOST_REQUIRE_EQUAL (cfun(), 1);
  }
}
