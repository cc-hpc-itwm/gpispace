// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE util_qt_boost_connect

#include <test/boost_connect.hpp>

#include <util/qt/boost_connect.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/test/unit_test.hpp>

using namespace boost::lambda;

BOOST_FIXTURE_TEST_CASE (unbind, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (void)>
                  ( this
                  , SIGNAL (signal1())
                  , this
                  , bind (&boost_connect_fixture::called, this)
                  )
                );

  BOOST_REQUIRE
    (fhg::util::qt::boost_disconnect (this, SIGNAL (signal1()), this));

  emit signal1();

  BOOST_REQUIRE_EQUAL (_called, false);
}

BOOST_FIXTURE_TEST_CASE (unbind_all, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (void)>
                  ( this
                  , SIGNAL (signal1())
                  , this
                  , bind (&boost_connect_fixture::called, this)
                  )
                );

  BOOST_REQUIRE (fhg::util::qt::boost_disconnect (this, NULL, this));

  emit signal1();

  BOOST_REQUIRE_EQUAL (_called, false);
}

BOOST_FIXTURE_TEST_CASE (auto_unbind, boost_connect_fixture)
{
  QObject* testObj (new QObject);

  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (void)>
                  (this, SIGNAL (signal1()), testObj, var(_called) = true)
                );

  delete testObj;

  emit signal1();

  BOOST_REQUIRE_EQUAL (_called, false);
}

BOOST_FIXTURE_TEST_CASE (nullary, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (void)>
                  ( this
                  , SIGNAL (signal1())
                  , this
                  , bind (&boost_connect_fixture::called, this)
                  )
                );

  emit signal1();

  BOOST_REQUIRE_EQUAL (_called, true);
}

BOOST_FIXTURE_TEST_CASE (nullary_lambda, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (void)>
                  (this, SIGNAL (signal1()), this, var (_called) = true)
                );

  emit signal1();

  BOOST_REQUIRE_EQUAL (_called, true);
}


BOOST_FIXTURE_TEST_CASE (unary, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (int)>
                  (this, SIGNAL (signal2 (int)), this, var (_ival) = _1)
                );

  emit signal2 (2);

  BOOST_REQUIRE_EQUAL (_ival, 2);
}

BOOST_FIXTURE_TEST_CASE (unary_discarding_value, boost_connect_fixture)
{
  BOOST_REQUIRE ( fhg::util::qt::boost_connect<void (int)>
                  (this, SIGNAL (signal2 (int)), this, var (_ival) = 1)
                );

  emit signal2 (2);

  BOOST_REQUIRE_EQUAL (_ival, 1);
}

void boost_connect_fixture::function (int val)
{
  _ival = val;
}

BOOST_FIXTURE_TEST_CASE (member_function, boost_connect_fixture)
{
  BOOST_REQUIRE
    ( fhg::util::qt::boost_connect<void (QByteArray)>
      ( this
      , SIGNAL (signal3 (QByteArray))
      , this
      , bind ( &boost_connect_fixture::function
             , this
             , bind (&QByteArray::toInt, _1, (bool*)0, 10)
             )
      )
    );

  emit signal3 ("2");

  BOOST_REQUIRE_EQUAL (_ival, 2);
}

BOOST_FIXTURE_TEST_CASE (binary, boost_connect_fixture)
{
  bool is_equal (false);

  BOOST_REQUIRE
    ( fhg::util::qt::boost_connect<void (unsigned int, unsigned int)>
      ( this
      , SIGNAL (signal4 (unsigned int, unsigned int))
      , this
      , var (is_equal) = _1 == _2
      )
    );

  emit signal4 (1, 2);

  BOOST_REQUIRE_EQUAL (is_equal, false);

  emit signal4 (1, 1);

  BOOST_REQUIRE_EQUAL (is_equal, true);
}
