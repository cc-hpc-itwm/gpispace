// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/printer/generic.hpp>

#include <QtCore/QString>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (QString, os, val)
{
  os << val.toStdString();
}
