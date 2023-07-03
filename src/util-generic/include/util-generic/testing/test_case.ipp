// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test_suite.hpp>

#define FHG_UTIL_TESTING_TEMPLATED_CASE_IMPL(name_, template_type_, types_...) \
  using name_ ## _types = ::boost::mpl::list<types_>;                          \
  BOOST_AUTO_TEST_CASE_TEMPLATE (name_, template_type_, name_ ## _types)
