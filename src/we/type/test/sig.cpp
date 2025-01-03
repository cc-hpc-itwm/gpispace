// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <sig_op.hpp>
#include <sig_struct.hpp>

#include <we/exception.hpp>
#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (sig_value)
{
  namespace value = pnet::type::value;

  namespace line2D = pnetc::type::line2D;
  namespace point2D = pnetc::type::point2D;

  value::value_type vq;
  value::poke ("x", vq, 1.0f);

  BOOST_CHECK_THROW ( line2D::q::from_value (vq)
                    , pnet::exception::missing_field
                    );

  value::poke ("y", vq, 2.0);

  BOOST_CHECK_THROW ( line2D::q::from_value (vq)
                    , pnet::exception::type_mismatch
                    );

  value::poke ("y", vq, 2.0f);

  value::value_type vp;
  value::poke ("x", vp, 3.0f);
  value::poke ("y", vp, 4.0f);
  value::value_type vl;
  value::poke ("p", vl, vp);
  value::poke ("q", vl, vq);

  BOOST_CHECK_EQUAL (vq, line2D::q::to_value (line2D::q::from_value (vq)));
  BOOST_CHECK_EQUAL (vp, point2D::to_value (point2D::from_value (vp)));
  BOOST_CHECK_EQUAL (vl, line2D::to_value (line2D::from_value (vl)));
}
